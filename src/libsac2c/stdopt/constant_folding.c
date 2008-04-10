/*
 * $Id constant_folding.c 15337 2007-06-11 19:49:07Z cg $
 */

/** <!--********************************************************************-->
 *
 * @defgroup cf Constant Folding
 *
 *   Module constant_folding.c implements constant folding.
 *
 *   This driver module performs AST traversal, invoking four
 *   distinct sets of constant-folding functions, via function tables:
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
 *    3. CF: Bog-standard Constant Folding, e.g.:
 *       a. Conditional-function constant-predicate constant folding, e.g:
 *          replacement of a conditional function body, when its predicate
 *          is known to be true/false, by the condfn's appropriate clause, e.g.:
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
 *
 *       b. Replacement of "x = 2 + 3" by " x = 5", where the value "5"
 *          is known to the typechecker, which has already done the actual
 *          addition.
 *
 *    4. SAACF: SAA Constant Folding,
 *
 *      SAACF uses function table prf_saacf, defined
 *      in saa_constant_folding.c
 *
 *   For constant expressions,  we compute primitive functions at compile time
 *   so we need not to compute them at runtime. This simplifies code
 *   and allows further optimizations.
 *
 *
 *   IMPORTANT: Making CF dependent on AKV types rather than constant arguments
 *   is a VERY important design decision. Only this way we prevent CF
 *   to be called with arguments that may violate the prf's domain restrictions
 *   (cf. bug 61).
 *
 *   Each computed constant expression is stored in the AVIS_SSACONST(avis)
 *   attribute of the assigned identifier for later access.
 *
 *   When traversing into a special fundef, we propagate constant information
 *   for all args (in loops only the loop-invariant ones) by storing the
 *   AVIS_SSACONST() in the corresponding args. constant results are propagted
 *   back in the calling context by inserting a assignment to the constant
 *   value. The removal of unused function args and result values is done
 *   later by the dead code removal.
 *
 *   At this time, the following primitive operations are implemented:

 *     for full constants (scalar value, arrays with known values):
 *       tob, toc, toi, tof, tod, abs, not, dim, shape, min, max, add, sub, mul, div,
 *       mod, and, le, lt, eq, ge, neq, reshape, sel, take_SxV, drop_SxV,
 *       cat_VxV, modarray
 *       Any folding on full constants (i.e., ALL function arguments constants)
 *       is performed by the type-checker.
 *
 *     structural constant, with full constant iv (array with ids as values):
 *       reshape, sel, take, drop, modarray
 *
 *     shape constant (array with known shape, scalar id):
 *       shape, sub
 *
 *     dim constants (expression with known dimension):
 *       dim, eq
 *
 *  arithmetic optimizations:
 *    add (x+0->x, 0+x->x),
 *    sub (x-0->x)
 *    mul (x*0->0, 0*x->0, x*1->x, 1*x->x),
 *    div (x/0->error, 0/x->0, x/1->x),
 *    and (x&&1->x, 1&&x->x, x&&0->0, 0&&x->0), x&&x -> x
 *    or  (x||1->1, 1||x->1, x||0->x, 0||x->x,  x||x -> x
 *    mod (x,0) -> error
 *    min (x,x) -> x
 *    max (x,x) -> x
 *
 *  relationals:
 *    x==x, x<=x, x>=x  -> genarray(shape(x),true),  if x is AKS or better
 *    x!=x, x<x,  x>x   -> genarray(shape(x),false), if x is AKS or better
 *
 *
 *
 *  special sel-modarray optimization:
 *    looking up in a modarray chain for setting the sel referenced value
 *
 *  not yet implemented: rotate
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
#include "shape.h"
#include "ctinfo.h"
#include "compare_tree.h"
#include "namespaces.h"
#include "remove_vardecs.h"
#include "saa_constant_folding.h"
#include "symbolic_constant_simplification.h"
#include "structural_constant_constant_folding.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool remassign;
    bool onefundef;
    node *fundef;
    node *preassign;
    node *postassign;
};

#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_POSTASSIGN(n) (n->postassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_REMASSIGN (result) = FALSE;
    INFO_ONEFUNDEF (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

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
 * @fn node* CFdoConstantFolding(node* fundef)
 *
 *****************************************************************************/

node *
CFdoConstantFolding (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CFdoConstantFolding called for non-fundef node");

    /* do not start traversal in special functions */
    arg_info = MakeInfo ();

    TRAVpush (TR_cf);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node* CFdoConstantFoldingModule(node* syntax_tree)
 *
 *****************************************************************************/

node *
CFdoConstantFoldingModule (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFoldingModule");

    /* do not start traversal in special functions */
    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_cf);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool IsFullyConstantNode( node *arg_node)
 *
 *****************************************************************************/
static bool
IsFullyConstantNode (node *arg_node)
{
    bool res;

    DBUG_ENTER ("IsFullyConstantNode");

    switch (NODE_TYPE (arg_node)) {
    case N_bool:
    case N_char:
    case N_num:
    case N_float:
    case N_double:
        res = TRUE;
        break;

    case N_array: {
        node *elems = ARRAY_AELEMS (arg_node);
        res = TRUE;
        while (res && (elems != NULL)) {
            res = res && IsFullyConstantNode (EXPRS_EXPR (elems));
            elems = EXPRS_NEXT (elems);
        }
    } break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * function:
 * @fn node* SplitMultipleAssigns( node *arg_node, info *arg_info)
 * description: Split an N_let node of the form:
 *   A,B,C = a,b,c;
 *  into
 *   A = a;
 *   B = b;
 *   C = b;
 *  This part of the code replaces the N_let node by:
 *   A = a;
 *  and leaves the other assigns in the arg_info INFO_POSTASSIGN node
 *  for later processing by CFassign.
 *
 *****************************************************************************/
static node *
SplitMultipleAssigns (node *arg_node, info *arg_info)
{
    node *id;
    node *expr;
    node *curlhs;
    node *currhs;
    node *postass;

    DBUG_ENTER ("SplitMultipleAssigns");
    DBUG_ASSERT (N_let == NODE_TYPE (arg_node),
                 "SplitMultipleAssigns expected N_let node");
    if (N_exprs == NODE_TYPE (LET_EXPR (arg_node))) {
        DBUG_PRINT ("CF", ("SplitMultipleAssigns found EXPRS. Goody.")); /* TEMP */

        /* Build new N_assign nodes for all but first lhs, rhs */
        curlhs = IDS_NEXT (LET_IDS (arg_node));
        currhs = EXPRS_NEXT (LET_EXPR (arg_node));
        while (NULL != curlhs) {
            DBUG_ASSERT ((NULL != currhs), "lhs<rhs count mismatch");
            postass = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (curlhs), NULL),
                                               EXPRS_EXPR (currhs)),
                                    NULL);
            AVIS_SSAASSIGN (IDS_AVIS (curlhs)) = postass;
            INFO_POSTASSIGN (arg_info)
              = TCappendAssign (INFO_POSTASSIGN (arg_info), postass);
            IDS_AVIS (curlhs) = NULL;
            EXPRS_EXPR (currhs) = NULL;
            curlhs = IDS_NEXT (curlhs);
            currhs = EXPRS_NEXT (currhs);
        }
        DBUG_ASSERT ((NULL == currhs), "lhs>rhs count mismatch");

        /* Now replace current assign with the first lhs, rhs */
        /* do rhs first */
        expr = LET_EXPR (arg_node);
        id = EXPRS_EXPR (expr);
        EXPRS_EXPR (expr) = NULL;
        DBUG_ASSERT (N_id == NODE_TYPE (id), "SplitMultipleAssigns Expected N_id");
        LET_EXPR (arg_node) = id;
        FREEdoFreeTree (expr);

        /* now do lhs */
        id = LET_IDS (arg_node);
        expr = IDS_NEXT (id);
        IDS_NEXT (id) = NULL;
        FREEdoFreeTree (expr);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node* CFfundef(node *arg_node, info *arg_info)
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
    DBUG_ENTER ("CFfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (FUNDEF_ISLACINLINE (arg_node)) {
            RMVdoRemoveVardecsOneFundef (arg_node);
        }
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert at least the N_empty node in an empty block */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
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

    DBUG_ENTER ("CFassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* save removal flag for modifications in bottom-up traversal */
    remassign = INFO_REMASSIGN (arg_info);
    preassign = INFO_PREASSIGN (arg_info);
    postassign = INFO_POSTASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;
    INFO_PREASSIGN (arg_info) = NULL;
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remassign) {
        /* skip this assignment and free it */
        DBUG_PRINT ("CF", ("remove dead assignment"));
        arg_node = FREEdoFreeNode (arg_node);
    }

    if (preassign != NULL) {
        arg_node = TCappendAssign (preassign, arg_node);
    }

    if (postassign != NULL) {
        arg_node = TCappendAssign (postassign, arg_node);
    }

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

    /* check for constant condition */
    if (TYisAKV (ID_NTYPE (FUNCOND_IF (arg_node)))
        && ((!FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
            || (!COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)))) {
        node *tmp;
        if (COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)) {
            tmp = FUNCOND_THEN (arg_node);
            FUNCOND_THEN (arg_node) = NULL;
        } else {
            tmp = FUNCOND_ELSE (arg_node);
            FUNCOND_ELSE (arg_node) = NULL;
        }
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *CFcondThen(node *arg_node, info *arg_info)
 *
 * description:
 *  Evaluate THEN leg of cond
 *
 *****************************************************************************/
static node *
CFcondThen (node *arg_node, info *arg_info)
{
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    /* select then-part for later insertion in assignment chain */
    INFO_PREASSIGN (arg_info) = BLOCK_INSTR (COND_THEN (arg_node));

    if (NODE_TYPE (INFO_PREASSIGN (arg_info)) == N_empty) {
        /* empty code block must not be moved */
        INFO_PREASSIGN (arg_info) = NULL;
    } else {
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
 *
 *
 *****************************************************************************/
static node *
CFcondElse (node *arg_node, info *arg_info)
{

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    /* select else-part for later insertion in assignment chain */
    INFO_PREASSIGN (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));

    if (NODE_TYPE (INFO_PREASSIGN (arg_info)) == N_empty) {
        /* empty code block must not be moved */
        INFO_PREASSIGN (arg_info) = NULL;
    } else {
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
 *   checks for constant conditional - removes corresponding counterpart
 *   of the conditional.
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
         * mark this assignment for removal, the selected code part will
         * be inserted behind this conditional assignment and traversed
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
     * Try to replace the rhs with a constant (given by the lhs type) if
     * - RHS node IS NOT an N_funcond node (this would violate fun-form
     * - RHS is not yet constant
     *
     * What's intended here is that the typechecker may determine
     * that the lhs is of of type AKV, so it knows the value of the lhs.
     * Hence, it can discard the RHS and replace it by the now-known lhs value.
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) != N_funcond)
        && (!IsFullyConstantNode (LET_EXPR (arg_node)))) {

        /*
         * Traverse into LHS
         * This yields an assignment for each ids node with constant type
         */
        if (LET_IDS (arg_node) != NULL) {
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        }

        /*
         * If ALL ids nodes are constant, the current assignment can be eliminated
         */
        if (TCcountIds (LET_IDS (arg_node))
            == TCcountAssigns (INFO_PREASSIGN (arg_info))) {

            /*
             * Set all AVIS_SSAASSIGN links
             */
            node *preass = INFO_PREASSIGN (arg_info);
            while (preass != NULL) {
                AVIS_SSAASSIGN (IDS_AVIS (ASSIGN_LHS (preass))) = preass;
                preass = ASSIGN_NEXT (preass);
            }

            global.optcounters.cf_expr += TCcountIds (LET_IDS (arg_node));
            INFO_REMASSIGN (arg_info) = TRUE;
        } else {
            if (INFO_PREASSIGN (arg_info) != NULL) {
                INFO_PREASSIGN (arg_info) = FREEdoFreeTree (INFO_PREASSIGN (arg_info));
            }
        }
    }

    /*
     * Traverse rhs only if it has not been replaced by constants
     */
    if (INFO_PREASSIGN (arg_info) == NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /* If CF introduced multiple results for an N_prf, break them
     * up now */
    arg_node = SplitMultipleAssigns (arg_node, arg_info);

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

    if (TYisAKV (IDS_NTYPE (arg_node))) {
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (arg_node),
                                     COconstant2AST (TYgetValue (IDS_NTYPE (arg_node)))),
                          INFO_PREASSIGN (arg_info));
        /*
         * Do not yet set AVIS_SSAASSIGN to the new assignment
         * this is done in CFlet iff it turns out the assignment chain is
         * in fact required
         */

    } else { /* Typechecker was no help; see if SAA can do any better */
        SAACF_ids (arg_node, arg_info);
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFarray(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses array elements to propagate constant identifiers
 *
 ******************************************************************************/

node *
CFarray (node *arg_node, info *arg_info)
{
    node *newelems = NULL;
    node *oldelems, *tmp, *first_inner_array;
    shape *shp = NULL, *newshp;
    ntype *basetype;
    ntype *atype;

    DBUG_ENTER ("CFarray");

    /*
     * Test whether whole array can be replaced with an array constant
     */
    atype = NTCnewTypeCheck_Expr (arg_node);

    if (TYisAKV (atype)) {
        /*
         * replace it
         */
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = COconstant2AST (TYgetValue (atype));
    } else {
        /*
         * Try to merge subarrays in
         */
        if (ARRAY_AELEMS (arg_node) != NULL) {
            /*
             * All elements need to be id nodes defined by N_array nodes.
             * Furthermore, they must all add the same dimensionality to
             * the dimension of their children
             */
            tmp = ARRAY_AELEMS (arg_node);
            while (tmp != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (tmp)) != N_id)
                    || (ID_SSAASSIGN (EXPRS_EXPR (tmp)) == NULL)
                    || (NODE_TYPE (ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (tmp))))
                        != N_array)) {
                    break;
                }
                oldelems = ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (tmp)));

                if (shp == NULL)
                    shp = ARRAY_FRAMESHAPE (oldelems);
                else if (!SHcompareShapes (shp, ARRAY_FRAMESHAPE (oldelems)))
                    break;

                tmp = EXPRS_NEXT (tmp);
            }
            if (tmp == NULL) {
                /*
                 * Merge subarrays into this arrays
                 */
                oldelems = ARRAY_AELEMS (arg_node);
                DBUG_ASSERT (oldelems != NULL,
                             "Trying to merge subarrays into an empty array!");
                first_inner_array = ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (oldelems)));
                tmp = oldelems;
                while (tmp != NULL) {
                    newelems
                      = TCappendExprs (newelems, DUPdoDupTree (ARRAY_AELEMS (ASSIGN_RHS (
                                                   ID_SSAASSIGN (EXPRS_EXPR (tmp))))));
                    tmp = EXPRS_NEXT (tmp);
                }

                basetype = TYcopyType (ARRAY_ELEMTYPE (first_inner_array));
                newshp = SHappendShapes (ARRAY_FRAMESHAPE (arg_node), shp);

                arg_node = FREEdoFreeNode (arg_node);

                arg_node = TBmakeArray (basetype, newshp, newelems);
            }
        }
    }

    atype = TYfreeType (atype);

    DBUG_RETURN (arg_node);
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
 * @fn node *CFwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFwith (node *arg_node, info *arg_info)
{
    node *vecassign = NULL;

    DBUG_ENTER ("CFwith");

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
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

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
    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    /*
     * Revert CODE_USED to correct state
     */
    CODE_USED (PART_CODE (arg_node)) = abs (CODE_USED (PART_CODE (arg_node)));

    DBUG_RETURN (arg_node);
}

#if 0 
 DEAD -handled by typechecker
/**<!--*************************************************************-->
  *
  * @fn node *CFprf_dim(node *arg_node, info *arg_info)
  *
  * @brief: performs standard constant-folding on dim primitive
  *         If argument rank is known (i.e., argument is AKS or AKD),
  *         the operation is replaced by the integer rank of the
  *         argument.  
  *
  * @param arg_node, arg_info
  *
  * @result new arg_node if dim() operation could be removed
  *         else NULL
  *
  ********************************************************************/
node *CFprf_dim(node *arg_node, info *arg_info)
{
  node *res = NULL;

  DBUG_ENTER( "CFprf_dim");
  DBUG_ASSERT(N_id == NODE_TYPE( PRF_ARG1(arg_node)),
               "CF_dim expected N_id node");

  if ( TUdimKnown( ID_NTYPE( PRF_ARG1(arg_node)))) {
    res = TBmakeNum( TYgetDim( ID_NTYPE(PRF_ARG1( arg_node))));
  }
  DBUG_RETURN( res);
}
#endif

/**<!--*************************************************************-->
 *
 * @fn node *CFprf_shape(node *arg_node, info *arg_info)
 *
 * @brief: performs standard constant-folding on shape primitive
 *
 * @param arg_node, arg_info
 *
 * @result new arg_node if shape() operation could be replaced
 *         else NULL
 *
 ********************************************************************/

node *
CFprf_shape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *avis;

    DBUG_ENTER ("CFprf_shape");
    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)), "CF_shape_ expected N_id node");
#if 0
  dead - handled by typechecker

  /* If AKS, result is the array's known shape */
  if ( TUshapeKnown( ID_NTYPE( PRF_ARG1( arg_node)))) {
    res = SHshape2Array( TYgetShape( ID_NTYPE( PRF_ARG1( arg_node))));
  } else {

#endif
    /* If AKD, replace the shape() operation by a list of idx_shape_sel() ops */
    if (TUdimKnown (ID_NTYPE (PRF_ARG1 (arg_node)))) {
        int i;
        for (i = TYgetDim (ID_NTYPE (PRF_ARG1 (arg_node))) - 1; i >= 0; i--) {
            avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (PRF_ARG1 (arg_node))),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                         TCmakePrf2 (F_idx_shape_sel, TBmakeNum (i),
                                                     DUPdoDupNode (PRF_ARG1 (arg_node)))),
                              INFO_PREASSIGN (arg_info));
            AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

            res = TBmakeExprs (TBmakeId (avis), res);
        }
        res = TCmakeIntVector (res);
    }
#if 0
    }
#endif

    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
 *
 * @fn node *CFprf_reshape(node *arg_node, info *arg_info)
 *
 * @brief: performs standard constant-folding on reshape primitive
 *
 * @param arg_node, arg_info
 *
 * @result if operation is an identity of the form:
 *             z = reshape( shp, arr)
 *
 *      and   (shp == shape(arr)), then the reshape is transformed to:
 *             z = arr
 *      else NULL
 *
 ********************************************************************/

node *
CFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;

    DBUG_ENTER ("CFprf_reshape");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));

    if ((NULL != arg1) && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)
        && (TUshapeKnown (ID_NTYPE (PRF_ARG2 (arg_node))))
        && (SHcompareWithCArray (TYgetShape (ID_NTYPE (PRF_ARG2 (arg_node))),
                                 COgetDataVec (arg1),
                                 SHgetExtent (COgetShape (arg1), 0)))) {
        DBUG_ASSERT (FALSE, "Night of the living dead code in CFprf_reshape");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}
