/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cf Constant Folding
 *
 *   Module constant_folding.c implements constant folding.
 *
 *   IMPORTANT: this optimization ensures LaC-function traversal to happen
 *              "inline", i.e., LaC funs will be traversed when coming across
 *              their application and NOT when traversing the fundef-spine!!!
 *
 *   This module does serve two purposes:
 *   1) it performs some standard constant folding
 *   2) it serves as a driver module for running several advanced
 *      constant folding techniques on N_prf nodes.
 *
 *   ad 1) Bog-standard Constant Folding
 *   -----------------------------------
 *   This module performs the following 4 transformations:
 *
 *   a. "classical" constant folding, e.g. replacement of
 *      "x = 2 + 3" by " x = 5"
 *       here, we make use of the typechecker, which has already done the
 *       actual addition by inferring AKV types.
 *       IMPORTANT: Making CF dependent on AKV types rather than constant
 *       rguments is a VERY important design decision. Only this way we prevent
 *       CF to be called with arguments that may violate the prf's domain
 *       restrictions (cf. bug 61).
 *   b. array "scalarization"i, e.g.:
 *      t1 = [a,b];
 *      t2 = [t1, t1];
 *
 *      is replaced by
 *
 *      t1 = [a,b];
 *      t2 = [[a,b],[a,b]];       * the nesting is encoded as FRAMESHAPE! *
 *
 *   c. Conditional-function constant-predicate constant folding, e.g:
 *      replacement of a conditional function body, when its predicate
 *      is known to be true/false, by the condfn's appropriate clause, e.g.:
 *
 *       if ( pred ) {
 *           z = exprt;
 *       } else {
 *           z = exprf;
 *       }
 *
 *          will, if "false == pred", be transformed to:
 *
 *       z = exprf;
 *   d. with-loop improvements for generators of size 1, e.g.:
 *
 *      with {
 *        ( [0] <= [i] < [1])  : i;
 *      } : genarray( shp, def)
 *
 *         is transformed into:
 *      with {
 *        ( [0] <= [i] < [1])  : 0;
 *      } : genarray( shp, def)
 *
 *      This is triggered by GENERATOR_GENWIDTH being 1 and CODE_USE being 1.
 *      The actual transformation is enabled by temporarily(!) upgrading
 *      the type of "i" to int{1} !!
 *
 *
 *   as 2) advanced N_prf based optimizations:
 *   -----------------------------------------
 *   This driver module performs AST traversal on N_prf nodes, invoking four
 *   distinct sets of constant-folding functions, via N_prf based function
 *   ables:
 *
 *    1. SCS: Symbolic Constant Simplification, e.g. replacement of:
 *          z = M + 0;
 *      by:
 *          z = M;
 *
 *      SCS uses function table prf_scs, defined
 *      in symbolic_constant_simplification.c
 *
 *    2. SCCF: Structural Constant constant folding, e.g.:
 *      e.g., reshape(const, structuralconst)
 *      e.g., one = 1; two = 2; three = 3;
 *            reshape([3],[one, two, three])
 *
 *      SCCF uses function table prf_sccf, defined
 *      in structural_constant_constant_folding.c
 *
 *    3. SAACF: SAA Constant Folding,
 *
 *      SAACF uses function table prf_saacf, defined
 *      in saa_constant_folding.c
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file constant_folding.c
 *
 * Prefix: CF
 *
 *****************************************************************************/

#include "constant_folding.h"

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "constants.h"
#include "lacinlining.h"
#include "shape.h"
#include "ctinfo.h"
#include "compare_tree.h"
#include "namespaces.h"
#include "remove_vardecs.h"
#include "saa_constant_folding.h"
#include "symbolic_constant_simplification.h"
#include "structural_constant_constant_folding.h"
#include "constant_folding_info.h"
#include "pattern_match.h"

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LACFUNOK (result) = TRUE;
    INFO_TRAVINLAC (result) = FALSE;

    INFO_NUM_IDS_SOFAR (result) = 0;

    INFO_LHSTYPE (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_TOPBLOCK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @name Static global function tables
 * @{
 *
 *****************************************************************************/

static const travfun_p prf_cfscs_funtab[] = {
#define PRFcf_scs_fun(cf_scs_fun) cf_scs_fun
#include "prf_info.mac"
};

static const travfun_p prf_cfsccf_funtab[] = {
#define PRFcf_sccf_fun(cf_sccf_fun) cf_sccf_fun
#include "prf_info.mac"
};

static const travfun_p prf_cfsaa_funtab[] = {
#define PRFcf_saa_fun(cf_saa_fun) cf_saa_fun
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 * @}  <!-- Static global function tables -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name AST traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node* CFdoConstantFoldingOneFundef(node* arg_node)
 *
 * NB: this enforces "following up LACFUNs"
 *
 *****************************************************************************/

node *
CFdoConstantFoldingOneFundef (node *arg_node)
{

    info *arg_info;

    DBUG_ENTER ("CFdoConstantFoldingOneFundef");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "CFdoConstantFoldingOneFundef called for non-fundef node");

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = TRUE;
    INFO_LACFUNOK (arg_info) = FALSE;

    SCSinitSymbolicConstantSimplification ();
    TRAVpush (TR_cf);
    arg_node = TRAVdo (arg_node, (info *)arg_info);
    TRAVpop ();
    SCSfinalizeSymbolicConstantSimplification ();

    arg_info = FreeInfo (arg_info);

    if (global.local_funs_grouped && !FUNDEF_ISLACFUN (arg_node)) {
        /**
         *   In case we are dealing with an "ordinary" (ie non LACFUN) function
         *   we are facing a potential inconsistency in case of the -glf representation
         *   here!
         *   One of the local functions may have been tagged as LACINLINE which
         *   implicitly transforms it into an ordinary function and, thus, renders
         *   the localfun list wrong.
         *   Rather than fixing that, we apply lacinline instead.
         */
        arg_node = LINLdoLACInliningOneFundef (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* CFdoConstantFolding(node* arg_node)
 *
 *****************************************************************************/

node *
CFdoConstantFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFolding");

    arg_info = MakeInfo ();

    SCSinitSymbolicConstantSimplification ();
    TRAVpush (TR_cf);
    arg_node = TRAVdo (arg_node, (info *)arg_info);
    TRAVpop ();
    SCSfinalizeSymbolicConstantSimplification ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Static (usually) helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool IsFullyConstantNode( node *arg_node)
 *
 * @brief in contrast to COisConstant, this function ensures a "minimal"
 *        AST representation of the argument!
 *
 *****************************************************************************/
static bool
IsScalarConstantNode (node *arg_node)
{
    DBUG_ENTER ("IsScalarConstantNode");

    DBUG_RETURN (PMO (PMObool (arg_node)) || PMO (PMOchar (arg_node))
                 || PMO (PMOnum (arg_node)) || PMO (PMOfloat (arg_node))
                 || PMO (PMOdouble (arg_node)));
}

static bool
IsFullyConstantNode (node *arg_node)
{
    bool res;
    constant *frameshape = NULL;

    DBUG_ENTER ("IsFullyConstantNode");

    if (IsScalarConstantNode (arg_node)) {
        res = TRUE;
    } else if (PMO (PMOarrayConstructorGuards (&frameshape, &arg_node, arg_node))) {
        arg_node = ARRAY_AELEMS (arg_node);
        res = TRUE;
        while (res && (arg_node != NULL)) {
            res = res && IsScalarConstantNode (EXPRS_EXPR (arg_node));
            arg_node = EXPRS_NEXT (arg_node);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* CreateConstExprsFromType( ntype *type)
 *
 * @brief Create AST exprs node for "type", which is known to be AKV.
 *
 *****************************************************************************/
static node *
CreateConstExprsFromType (ntype *type)
{
    node *res = NULL;
    int i;
    DBUG_ENTER ("CreateConstExprsFromType");

    if (TYisProd (type)) {
        for (i = TYgetProductSize (type) - 1; i >= 0; i--) {
            res = TBmakeExprs (CreateConstExprsFromType (TYgetProductMember (type, i)),
                               res);
        }
    } else {
        res = COconstant2AST (TYgetValue (type));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* node* CreateAssignsFromIdsExprs( node *ids, node *exprs)
 *
 * @brief recylcles all ids and all expressions while creating an
 *        assignment chain left-to-right, ie.
 *        A call with ids: A,B,C and exprs: a,b,c
 *        is transformed into
 *        A = a;
 *        B = b;
 *        C = b;
 *        The N_exprs nodes are being freed!
 *
 *****************************************************************************/
static node *
CreateAssignsFromIdsExprs (node *ids, node *exprs)
{
    node *res = NULL;
    node *expr;

    DBUG_ENTER ("CreateAssignsFromIdsExprs");

    if (ids != NULL) {
        DBUG_ASSERT ((exprs != NULL),
                     "ids chain longer than exprs chain in CreateAssignsFromIdsExprs");
        expr = EXPRS_EXPR (exprs);
        EXPRS_EXPR (exprs) = NULL;
        res = TBmakeAssign (TBmakeLet (ids, expr),
                            CreateAssignsFromIdsExprs (IDS_NEXT (ids),
                                                       FREEdoFreeNode (exprs)));
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = res;
        IDS_NEXT (ids) = NULL;
    } else {
        DBUG_ASSERT ((exprs == NULL),
                     "exprs chain longer than ids chain in CreateAssignsFromIdsExprs");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node* CFfundef(node *arg_node, fundefinfo *arg_info)
 *
 * description:
 *   traverses args and block in this order.
 *   the args are only traversed in loop special functions to remove
 *   propagated constants from loop-dependent arguments.
 *
 *****************************************************************************/

node *
CFfundef (node *arg_node, info *arg_info)
{
    node *old_fundef, *old_topblock, *old_vardecs;
    ntype *old_lhstype;

    DBUG_ENTER ("CFfundef");

    if ((FUNDEF_BODY (arg_node) != NULL)
        && (!FUNDEF_ISLACFUN (arg_node) || INFO_LACFUNOK (arg_info)
            || INFO_TRAVINLAC (arg_info))) {

        old_fundef = INFO_FUNDEF (arg_info);
        old_topblock = INFO_TOPBLOCK (arg_info);
        old_vardecs = INFO_VARDECS (arg_info);
        old_lhstype = INFO_LHSTYPE (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_TOPBLOCK (arg_info) = NULL;
        INFO_VARDECS (arg_info) = NULL;
        INFO_LHSTYPE (arg_info) = NULL;

        DBUG_PRINT ("CF", ("traversing body of %s", FUNDEF_NAME (arg_node)));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("CF", ("leaving body of %s", FUNDEF_NAME (arg_node)));

        INFO_FUNDEF (arg_info) = old_fundef;
        INFO_TOPBLOCK (arg_info) = old_topblock;
        INFO_VARDECS (arg_info) = old_vardecs;
        INFO_LHSTYPE (arg_info) = old_lhstype;

        if (FUNDEF_ISLACINLINE (arg_node)) {
            RMVdoRemoveVardecsOneFundef (arg_node);
        }
    }
    if (!INFO_ONEFUNDEF (arg_info) && !INFO_TRAVINLAC (arg_info)
        && FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses instructions only
 *
 *****************************************************************************/

node *
CFblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CFblock");

    if (NULL == INFO_TOPBLOCK (arg_info)) {
        INFO_TOPBLOCK (arg_info) = arg_node;
        INFO_VARDECS (arg_info) = BLOCK_VARDEC (arg_node);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert at least the N_empty node in an empty block */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    /* New vardecs go only at top level block */
    if (INFO_TOPBLOCK (arg_info) == arg_node) {
        INFO_TOPBLOCK (arg_info) = NULL;
        BLOCK_VARDEC (arg_node) = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFassign(node *arg_node, info *arg_info)
 *
 * description:
 *   top-down traversal of assignments. in bottom-up return traversal remove
 *   marked assignment-nodes from chain and insert moved assignments (e.g.
 *   from constant, inlined conditionals)
 *
 *   Also, insert assignments generated by constant-folded multi-result
 *   primitives. E.g.:
 *       A',B',pred = _same_shape_AxA_(A,B)
 *   where A and B can be determined to have same shapes, becomes:
 *       A',B',pred = A,B,TRUE;
 *
 *   This syntax not supported by sac2c, so here we basically turn it into:
 *       A' = A;
 *       B' = B;
 *       pred = TRUE;
 *
 *
 *****************************************************************************/

node *
CFassign (node *arg_node, info *arg_info)
{
    bool remassign;
    node *preassign;
    node *postassign;
    node *succ;

    DBUG_ENTER ("CFassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* save removal flag for modifications in bottom-up traversal */
    remassign = INFO_REMASSIGN (arg_info);
    preassign = INFO_PREASSIGN (arg_info);
    postassign = INFO_POSTASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;
    INFO_PREASSIGN (arg_info) = NULL;
    INFO_POSTASSIGN (arg_info) = NULL;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /**
     * First, we separate the current assignments from its successors:
     */
    succ = ASSIGN_NEXT (arg_node);
    ASSIGN_NEXT (arg_node) = NULL;

    /**
     * Delete the current one if needed.
     */
    if (remassign) {
        DBUG_PRINT ("CF", ("CFassign removed dead assignment"));
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMASSIGN (arg_info) = FALSE;
    }

    /**
     *  Then construct the final chain, back to front:
     */
    postassign = TCappendAssign (postassign, succ);
    arg_node = TCappendAssign (arg_node, postassign);
    arg_node = TCappendAssign (preassign, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *  function:
 *    node *CFfuncond(node *arg_node, node* arg_info)
 *
 *  description:
 *    Handles:     z = condition ? trueexpr : falseexpr;
 *    Check if the conditional predicate is constant.
 *    If it is constant, than resolve all funcond nodes according
 *    to the predicate and set the inline flag.
 *
 *****************************************************************************/
node *
CFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFfuncond");
    node *res;

    /* check for constant condition */
    res = arg_node;
    if (TYisAKV (ID_NTYPE (FUNCOND_IF (arg_node)))
        && ((!FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
            || (!COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)))) {
        if (COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)) {
            res = DUPdoDupTree (FUNCOND_THEN (arg_node));
            arg_node = FREEdoFreeTree (arg_node);
            DBUG_PRINT ("CF", ("CFfuncond found TRUE condition"));
        } else {
            res = DUPdoDupTree (FUNCOND_ELSE (arg_node));
            arg_node = FREEdoFreeTree (arg_node);
            DBUG_PRINT ("CF", ("CFfuncond found FALSE condition"));
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static node *CFcondThen(node *arg_node, info *arg_info)
 *
 * description:
 *  Evaluate THEN leg of cond
 *  The THEN leg is known to be true.
 *
 *****************************************************************************/
static node *
CFcondThen (node *arg_node, info *arg_info)
{
    node *pre;
    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("CF", ("CFcondThen found TRUE condition"));

    /* select then-part for later insertion in assignment chain */
    pre = BLOCK_INSTR (COND_THEN (arg_node));
    if (NODE_TYPE (pre) != N_empty) { /* empty code block must not be moved */
        DBUG_ASSERT ((NULL == INFO_PREASSIGN (arg_info)),
                     ("CFcondThen preassign confusion"));
        INFO_PREASSIGN (arg_info) = pre;
        /*
         * delete pointer to codeblock to preserve assignments from
         * being freed
         */
        BLOCK_INSTR (COND_THEN (arg_node)) = NULL;
    }
    return (arg_node);
}

/******************************************************************************
 *
 * function:
 *  static node* CFcondElse(node *arg_node, info *arg_info)
 *
 * description:
 *  Evaluate ELSE leg of cond
 *  The ELSE leg is known to be true.
 *
 *
 *****************************************************************************/
static node *
CFcondElse (node *arg_node, info *arg_info)
{
    node *pre;

    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
    DBUG_PRINT ("CF", ("CFcondElse found FALSE condition"));

    /* select else-part for later insertion in assignment chain */
    pre = BLOCK_INSTR (COND_ELSE (arg_node));
    if (NODE_TYPE (pre) != N_empty) { /* empty code block must not be moved */
        DBUG_ASSERT ((NULL == INFO_PREASSIGN (arg_info)),
                     ("CFcondElse preassign confusion"));
        INFO_PREASSIGN (arg_info) = pre;
        /*
         * delete pointer to codeblock to preserve assignments from
         * being freed
         */
        BLOCK_INSTR (COND_ELSE (arg_node)) = NULL;
    }
    return (arg_node);
}
/******************************************************************************
 *
 * function:
 *   node* CFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   Handle folding of: IF (cond) Then {thenblock} Else {elseblock}.
 *   If cond is statically known, we extract the appropriate
 *   (then/else) block, and place it in the PREASSIGN list, where
 *   CFassign will handle placing it before this line. We then
 *   mark arg_node for deletion, again by CFassign.
 *
 *   traverses conditional and optional then-part, else-part
 *
 *****************************************************************************/
node *
CFcond (node *arg_node, info *arg_info)
{
    bool condknown = FALSE;
    bool condtrue = FALSE;

    DBUG_ENTER ("CFcond");

    /* check for constant COND */
    condknown = TYisAKV (ID_NTYPE (COND_COND (arg_node)));
    if (condknown) {
        condtrue = COisTrue (TYgetValue (ID_NTYPE (COND_COND (arg_node))), TRUE);
    }

    if (condtrue && FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info))) {
        /*
         * if this is a do- or while function and the condition is evaluated
         * to true we have an endless loop. We MUST avoid doing CF, as it
         * won't terminate either!
         */
        CTIwarnLine (NODE_LINE (arg_node),
                     "Infinite loop detected, program may not terminate");
        condknown = FALSE;
    }

    if (condknown) {
        arg_node
          = condtrue ? CFcondThen (arg_node, arg_info) : CFcondElse (arg_node, arg_info);
        /*
         * mark the conditional for removal. The selected block
         * has been placed in preassign; CFassign will
         * insert the block
         * behind this conditional assignment and traverse it
         * for constant folding.
         */
        INFO_REMASSIGN (arg_info) = TRUE;

        FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)) = FALSE;
        FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)) = FALSE;
        FUNDEF_ISLACINLINE (INFO_FUNDEF (arg_info)) = TRUE;

    } else {
        /*
         * no constant condition:
         * do constant folding in conditional
         */
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFlet(node *arg_node, info *arg_info)
 *
 * description:
 *  See notes below.
 *
 *  Also, if the rhs is now an N_exprs node, CF has replaced
 *  a prf of the form:
 *      B',C',pred = prf_same_shape_AxA_(B,C)
 *  with:
 *      B',C', pred = B,C, TRUE
 *  except that SAC doesn't like that, so we fix it on the
 *  way back up the traversal tree, eventually replacing it by:
 *      B' = B;
 *      C' = C;
 *      pred = TRUE;
 *
 *****************************************************************************/

node *
CFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFlet");

    /*
     * What's intended here is that the typechecker may determine
     * that the lhs is of of type AKV, so it knows the value of the lhs.
     * Hence, it can discard the RHS and replace it by the now-known lhs value.
     */
    DBUG_ASSERT ((LET_IDS (arg_node) != NULL), "empty LHS of let found in CF");
    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "empty RHS of let found in CF");

    /**
     *  First, we collect the INFO_LHSTYPE!
     */
    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (TYisProdOfAKV (INFO_LHSTYPE (arg_info))
        && (NODE_TYPE (LET_EXPR (arg_node)) != N_funcond)) {
        /**
         * NB: we MUST not throw away funconds UNLESS we know the predicate!
         *     otherwise, we come into an inconsistent state which is not
         *     being dealt with in SAA insertion (cf bug 446).
         *     Therefore we HAVE TO traverse the RHS if these are funconds.
         */

        if (!IsFullyConstantNode (LET_EXPR (arg_node))) {
            LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
            if (TYgetProductSize (INFO_LHSTYPE (arg_info)) == 1) {
                LET_EXPR (arg_node) = CreateConstExprsFromType (
                  TYgetProductMember (INFO_LHSTYPE (arg_info), 0));
            } else {
                LET_EXPR (arg_node) = CreateConstExprsFromType (INFO_LHSTYPE (arg_info));
            }
            global.optcounters.cf_expr += TYgetProductSize (INFO_LHSTYPE (arg_info));
        }
    }

    INFO_LHSTYPE (arg_info) = TYfreeTypeConstructor (INFO_LHSTYPE (arg_info));

    /**
     *  If CF has replaced the RHS by an N_exprs chain, we have to break this
     *  up now!
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_exprs) {
        INFO_POSTASSIGN (arg_info)
          = TCappendAssign (CreateAssignsFromIdsExprs (LET_IDS (arg_node),
                                                       LET_EXPR (arg_node)),
                            INFO_POSTASSIGN (arg_info));
        LET_EXPR (arg_node) = NULL;
        LET_IDS (arg_node) = NULL;
        INFO_REMASSIGN (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFids( node *arg_node, info *arg_info)
 *
 * @brief: If the type checker has determined that the type of
 *         the LHS is AKV, replace the RHS by that value.
 *
 *         If the type checker is unable to do that, the
 *         SAA code is given a chance to make a similar determination.
 *
 *****************************************************************************/
node *
CFids (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CFids");

    INFO_NUM_IDS_SOFAR (arg_info)++;

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    } else {
        INFO_LHSTYPE (arg_info) = TYmakeEmptyProductType (INFO_NUM_IDS_SOFAR (arg_info));
    }
    INFO_NUM_IDS_SOFAR (arg_info)--;

    INFO_LHSTYPE (arg_info)
      = TYsetProductMember (INFO_LHSTYPE (arg_info), INFO_NUM_IDS_SOFAR (arg_info),
                            IDS_NTYPE (arg_node));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFarray(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses array elements to propagate constant identifiers
 *   E.g., if we have:
 *     b = [one, two];
 *     c = [b,b,b];
 *   CFarray will expand that into:
 *     c = [[one,two], [one,two], [one,two]];
 *
 ******************************************************************************/
node *
CFarray (node *arg_node, info *arg_info)
{
    constant *fs = NULL;
    shape *fshp;
    node *exprs, *res, *array;
    node *lexprs = NULL;
    pattern *pat, *pat2;

    DBUG_ENTER ("CFarray");

    exprs = ARRAY_AELEMS (arg_node);
    pat = PMarray (0, 2, PMarray (1, PMAgetFS (&fs), 1, PMskip (0)), PMskip (0));

    if (PMmatchFlat (pat, arg_node)) {

        pat2 = PMarray (2, PMAhasFS (&fs), PMAgetNode (&array), 1, PMskip (0));

        while ((exprs != NULL) && PMmatchFlat (pat2, EXPRS_EXPR (exprs))) {
            lexprs = TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (array)), lexprs);
            exprs = EXPRS_NEXT (exprs);
        }

        if (exprs == NULL) {
            fshp = COconstant2Shape (fs);
            res
              = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (array)),
                             SHappendShapes (ARRAY_FRAMESHAPE (arg_node), fshp), lexprs);
            fshp = SHfreeShape (fshp);
            arg_node = FREEdoFreeNode (arg_node);
        } else {
            res = arg_node;
        }
        pat2 = PMfree (pat2);
    } else {
        res = arg_node;
    }
    fs = (NULL != fs) ? COfreeConstant (fs) : fs;
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node* CFprf(node *arg_node, info *arg_info)
 *
 * description:
 *   evaluates primitive function with constant paramters and substitutes
 *   the function application by its value.
 *
 *****************************************************************************/

node *
CFprf (node *arg_node, info *arg_info)
{
    node *res = NULL;
    travfun_p fn;

    DBUG_ENTER ("CFprf");
    DBUG_PRINT ("CF", ("evaluating prf %s", global.prf_name[PRF_PRF (arg_node)]));
    /* Bog-standard constant-folding is all handled by typechecker now */
    /* Try symbolic constant simplification */
    fn = prf_cfscs_funtab[PRF_PRF (arg_node)];
    if ((NULL == res) && (NULL != fn)) {
        res = fn (arg_node, arg_info);
    }

    /* If that doesn't help, try structural constant constant folding */
    fn = prf_cfsccf_funtab[PRF_PRF (arg_node)];
    if ((NULL == res) && (NULL != fn)) {
        res = fn (arg_node, arg_info);
    }

    /* If that doesn't help, try SAA constant folding */
    fn = prf_cfsaa_funtab[PRF_PRF (arg_node)];
    if ((NULL == res) && (NULL != fn)) {
        res = fn (arg_node, arg_info);
    }

    if (res != NULL) {
        /* free this primitive function and replace it by new node */
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = res;

        /* increment constant folding counter */
        global.optcounters.cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFap (node *arg_node, info *arg_info)
{
    bool old_til;

    DBUG_ENTER ("CFap");

    if (!INFO_LACFUNOK (arg_info)
        && (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))
            || (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))
                && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))))) {
        old_til = INFO_TRAVINLAC (arg_info);
        INFO_TRAVINLAC (arg_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_TRAVINLAC (arg_info) = old_til;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFwith (node *arg_node, info *arg_info)
{
    node *vecassign = NULL;
    ntype *old_lhstype;

    DBUG_ENTER ("CFwith");

    old_lhstype = INFO_LHSTYPE (arg_info);
    INFO_LHSTYPE (arg_info) = NULL;

    /*
     * Create a fake assignment for the index vector in case the variables
     * need to be looked up.
     */
    if (WITH_IDS (arg_node) != NULL) {
        vecassign
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (WITH_VEC (arg_node)),
                                     TCmakeIntVector (TCids2Exprs (WITH_IDS (arg_node)))),
                          NULL);
        AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (arg_node))) = vecassign;
    }

    /*
     * Codes are traversed via the Parts to allow for exploiting generators
     * of width one.
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Remove the fake assignment after the traversal
     */
    if (vecassign != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (arg_node))) = NULL;
        vecassign = FREEdoFreeTree (vecassign);
    }

    INFO_LHSTYPE (arg_info) = old_lhstype;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFcode");

    /*
     * Do not traverse CODE_NEXT since codes are traversed through the Parts
     */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFpart (node *arg_node, info *arg_info)
{
    ntype *temp;
    node *n;

    DBUG_ENTER ("CFpart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    /*
     * Try to temporarily upgrade the types of the index variables to AKV
     * in case some width of the generator is known to be one
     */
    if ((CODE_USED (PART_CODE (arg_node)) == 1)
        && (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator)
        && (GENERATOR_GENWIDTH (PART_GENERATOR (arg_node)) != NULL)) {
        node *gen = PART_GENERATOR (arg_node);
        ntype *gwtype;
        ntype *lbtype;

        /*
         * Try to upgrade the type of the index vector
         */
        gwtype = NTCnewTypeCheck_Expr (GENERATOR_GENWIDTH (gen));
        lbtype = NTCnewTypeCheck_Expr (GENERATOR_BOUND1 (gen));

        if ((TYisAKV (gwtype)) && (COisOne (TYgetValue (gwtype), TRUE))
            && (TYisAKV (lbtype))) {
            IDS_NTYPE (PART_VEC (arg_node))
              = TYfreeType (IDS_NTYPE (PART_VEC (arg_node)));
            IDS_NTYPE (PART_VEC (arg_node)) = TYcopyType (lbtype);
        }

        gwtype = TYfreeType (gwtype);
        lbtype = TYfreeType (lbtype);

        /*
         * Try to upgrade the types of the index scalars
         */
        if ((NODE_TYPE (GENERATOR_GENWIDTH (gen)) == N_array)
            && (NODE_TYPE (GENERATOR_BOUND1 (gen)) == N_array)) {
            node *lbe = ARRAY_AELEMS (GENERATOR_BOUND1 (gen));
            node *gwe = ARRAY_AELEMS (GENERATOR_GENWIDTH (gen));
            n = PART_IDS (arg_node);

            while (n != NULL) {
                gwtype = NTCnewTypeCheck_Expr (EXPRS_EXPR (gwe));
                lbtype = NTCnewTypeCheck_Expr (EXPRS_EXPR (lbe));

                if ((TYisAKV (gwtype)) && (COisOne (TYgetValue (gwtype), TRUE))
                    && (TYisAKV (lbtype))) {
                    IDS_NTYPE (n) = TYfreeType (IDS_NTYPE (n));
                    IDS_NTYPE (n) = TYcopyType (lbtype);
                }

                gwtype = TYfreeType (gwtype);
                lbtype = TYfreeType (lbtype);

                n = IDS_NEXT (n);
                lbe = EXPRS_NEXT (lbe);
                gwe = EXPRS_NEXT (gwe);
            }
        }
    }

    /*
     * Traverse this parts code if it has not yet been traversed.
     * Mark the code as completely traversed afterwards by inverting CODE_USED
     */
    if (CODE_USED (PART_CODE (arg_node)) > 0) {
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
        CODE_USED (PART_CODE (arg_node)) *= -1;
    }

    /*
     * Revert types of index variables to AKS
     */
    temp = IDS_NTYPE (PART_VEC (arg_node));
    IDS_NTYPE (PART_VEC (arg_node)) = TYeliminateAKV (temp);
    temp = TYfreeType (temp);

    n = PART_IDS (arg_node);
    while (n != NULL) {
        temp = IDS_NTYPE (n);
        IDS_NTYPE (n) = TYeliminateAKV (temp);
        temp = TYfreeType (temp);

        n = IDS_NEXT (n);
    }

    /*
     * Traverse PART_NEXT
     */
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    /*
     * Revert CODE_USED to correct state
     */
    CODE_USED (PART_CODE (arg_node)) = abs (CODE_USED (PART_CODE (arg_node)));

    DBUG_RETURN (arg_node);
}
