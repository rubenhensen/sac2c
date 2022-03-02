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

#define DBUG_PREFIX "CF"
#include "debug.h"

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
#include "constant_folding.h"
#include "saa_constant_folding.h"
#include "symbolic_constant_simplification.h"
#include "structural_constant_constant_folding.h"
#include "constant_folding_info.h"
#include "pattern_match.h"
#include "flattengenerators.h"
#include "check.h"
#include "ivexpropagation.h"
#include "polyhedral_guard_optimization.h"
#include "polyhedral_utilities.h"
#include "polyhedral_setup.h"

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NUM_IDS_SOFAR (result) = 0;

    INFO_LHSTYPE (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_TOPBLOCK (result) = NULL;
    INFO_AVISMIN (result) = NULL;
    INFO_AVISMAX (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_PART (result) = NULL;
    INFO_PROXYARR (result) = NULL;
    INFO_DOINGEXTREMA (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node* CFdoConstantFolding(node* arg_node)
 *
 *****************************************************************************/
node *
CFdoConstantFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cf);
    arg_node = TRAVdo (arg_node, (info *)arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    if ((global.local_funs_grouped) && (N_fundef == NODE_TYPE (arg_node))
        && (!FUNDEF_ISLACFUN (arg_node))) {
        /**
         *   In case we are dealing with an "ordinary" (ie non LACFUN) function
         *   we are facing a potential inconsistency in case of
         *   the -glf representation here!
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
 * @name Static (usually) helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool IsScalarConstantNode( node *arg_node)
 *
 * @brief in contrast to COisConstant, this function ensures a "minimal"
 *        AST representation of the argument!
 *
 *****************************************************************************/
static bool
IsScalarConstantNode (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_RETURN (PMO (PMObool (arg_node)) || PMO (PMOnumbyte (arg_node))
                 || PMO (PMOnumubyte (arg_node)) || PMO (PMOnumint (arg_node))
                 || PMO (PMOnumuint (arg_node)) || PMO (PMOnumshort (arg_node))
                 || PMO (PMOnumushort (arg_node)) || PMO (PMOnumlong (arg_node))
                 || PMO (PMOnumulong (arg_node)) || PMO (PMOnumlonglong (arg_node))
                 || PMO (PMOnumulonglong (arg_node)) || PMO (PMOchar (arg_node))
                 || PMO (PMOnum (arg_node)) || PMO (PMOfloat (arg_node))
                 || PMO (PMOdouble (arg_node)));
}

bool
CFisFullyConstantNode (node *arg_node)
{
    bool res;
    constant *frameshape = NULL;

    DBUG_ENTER ();

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

#ifdef FIXME
/** <!--********************************************************************-->
 *
 * @fn bool isNarrayHasIdNode( node *arg_node)
 *
 * @brief  Predicate returns TRUE if any ARRAY_AELEMS is an N_id node.
 *
 *****************************************************************************/
static bool
isNarrayHasIdNode (node *arg_node)
{
    bool res;

    DBUG_ENTER ();

    arg_node = ARRAY_AELEMS (arg_node);
    res = FALSE;
    while ((!res) && (arg_node != NULL)) {
        res = res || (N_id == NODE_TYPE (EXPRS_EXPR (arg_node)));
        arg_node = EXPRS_NEXT (arg_node);
    }

    DBUG_RETURN (res);
}
#endif // FIXME

/** <!--********************************************************************-->
 *
 * @fn node *CFunflattenSimpleScalars( node *arg_node)
 *
 * @brief  Replaces N_array elements that are N_id nodes
 *         pointing to simple scalar constants by their values.
 *
 *           one = 1;
 *           two = 2;
 *           arr = [ one, 3, two];
 *
 *         by:
 *
 *          arr = [ 1, 3, 2];
 *
 *        If the N_array is already flattened, we do nothing.
 *
 *****************************************************************************/
node *
CFunflattenSimpleScalars (node *arg_node)
{
    node *res;

    DBUG_ENTER ();

    res = arg_node;
#ifdef FIXME
    node *el;
    node *curel;
    node *cons;
    pattern *pat;

    pat = PMconst (1, PMAgetNode (&cons));

    if (TUisScalar (ARRAY_ELEMTYPE (arg_node)) && isNarrayHasIdNode (arg_node)
        && CFisFullyConstantNode (arg_node)) {
        res = DUPdoDupTree (arg_node);
        DBUG_PRINT ("Unflattening N_array of scalar constants");
        el = ARRAY_AELEMS (res);
        while (NULL != el) {
            curel = EXPRS_EXPR (el);
            if ((N_id == NODE_TYPE (curel)) && (PMmatchFlat (pat, curel))) {
                FREEdoFreeNode (EXPRS_EXPR (el));
                EXPRS_EXPR (el) = DUPdoDupNode (cons);
            }
            el = EXPRS_NEXT (el);
        }
        arg_node = FREEdoFreeTree (arg_node);
    }
    pat = PMfree (pat);
#endif // FIXME

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* CFcreateConstExprsFromType( ntype *type)
 *
 * @brief Create AST exprs node for "type", which is known to be AKV.
 *
 *****************************************************************************/
node *
CFcreateConstExprsFromType (ntype *type)
{
    node *res = NULL;
    size_t i;
    DBUG_ENTER ();

    if (TYisProd (type)) {
        /* 
         * decrement after check for > 0, safe method for reverse loop ending on 0
         * i : (ProductSize - 1) to 0
         */
        for (i = TYgetProductSize (type); i-- > 0; ) {
            res = TBmakeExprs (CFcreateConstExprsFromType (TYgetProductMember (type, i)),
                               res);
        }
    } else {
        res = COconstant2AST (TYgetValue (type));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @brief This function wraps the identifier given as first argument into
 *        a type_conv, iff the type of the identifier is less precise than
 *        the oldtype argument.
 *
 *        If the argument is no expression, nothing is done.
 *
 * @param id       expression node (reused)
 * @param oldtype  type to enforce
 *
 * @return new expression
 ******************************************************************************/
node *
PreventTypePrecisionLoss (node *id, ntype *oldtype)
{
    node *res;

    DBUG_ENTER ();

    if ((id != NULL) && (NODE_TYPE (id) == N_id)) {
        if (!TYleTypes (ID_NTYPE (id), oldtype)) {
            DBUG_PRINT ("Generating type_conv( oldtype, %s", AVIS_NAME (ID_AVIS (id)));
            res = TCmakePrf2 (F_type_conv, TBmakeType (TYcopyType (oldtype)), id);
        } else {
            res = id;
        }
    } else {
        res = id;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* node* CreateAssignsFromIdsExprs( node *ids, node *exprs)
 *
 * @brief recycles all ids and all expressions while creating an
 *        assignment chain left-to-right, ie.
 *        A call with ids: A,B,C and exprs: a,b,c
 *        is transformed into
 *        A = a;
 *        B = b;
 *        C = b;
 *        The N_exprs nodes are freed, SSAASSIGN links are updated.
 *
 *****************************************************************************/
static node *
CreateAssignsFromIdsExprs (node *ids, node *exprs, ntype *restypes)
{
    node *res = NULL;
    node *cexpr = NULL;
    node *tmp;
    node *expr;
    size_t pos = 0;

    DBUG_ENTER ();

    while (ids != NULL) {
        DBUG_ASSERT (exprs != NULL,
                     "ids chain longer than exprs chain in CreateAssignsFromIdsExprs");

        /* grab expression and ensure type */
        expr = EXPRS_EXPR (exprs);
        EXPRS_EXPR (exprs) = NULL;
        expr = PreventTypePrecisionLoss (expr, TYgetProductMember (restypes, pos));

        tmp = TBmakeAssign (TBmakeLet (ids, expr), NULL);
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = tmp;

        if (cexpr == NULL) {
            cexpr = tmp;
            res = tmp;
        } else {
            ASSIGN_NEXT (cexpr) = tmp;
            cexpr = tmp;
        }
        tmp = ids;
        ids = IDS_NEXT (tmp);
        IDS_NEXT (tmp) = NULL;

        exprs = FREEdoFreeNode (exprs);
        pos++;
    }

    DBUG_ASSERT (exprs == NULL,
                 "exprs chain longer than ids chain in CreateAssignsFromIdsExprs");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *InvokeCFprfAndFlattenExtrema( node *arg_node,
 *                                                info *arg_info,
 *                                                 travfun_p fn,
 *                                                 node *res)
 *
 * @brief Invoke specific CFprf function.
 *        Then, flatten the extrema, if they were created.
 *
 *****************************************************************************/

static node *
InvokeCFprfAndFlattenExtrema (node *arg_node, info *arg_info, travfun_p fn, node *res)
{
    node *ex;

    DBUG_ENTER ();

    if ((NULL == res) && (NULL != fn)) {
        res = fn (arg_node, arg_info);
        if ((NULL != res) && (NULL != INFO_AVISMIN (arg_info))) {
            ex = FLATGexpression2Avis (INFO_AVISMIN (arg_info), &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGN (arg_info), NULL);
            INFO_AVISMIN (arg_info) = ex;
        }
        if ((NULL != res) && (NULL != INFO_AVISMAX (arg_info))) {
            ex = FLATGexpression2Avis (INFO_AVISMAX (arg_info), &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGN (arg_info), NULL);
            INFO_AVISMAX (arg_info) = ex;
        }
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
 *   CF does NOT traverse wrapper functions, not the LOCALFUNS of
 *   wrapper functions.
 *
 *****************************************************************************/

node *
CFfundef (node *arg_node, info *arg_info)
{
    node *old_fundef, *old_topblock, *old_vardecs;
    ntype *old_lhstype;

    DBUG_ENTER ();

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISWRAPPERFUN (arg_node))) {
        old_fundef = INFO_FUNDEF (arg_info);
        old_topblock = INFO_TOPBLOCK (arg_info);
        old_vardecs = INFO_VARDECS (arg_info);
        old_lhstype = INFO_LHSTYPE (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_TOPBLOCK (arg_info) = NULL;
        INFO_VARDECS (arg_info) = NULL;
        INFO_LHSTYPE (arg_info) = NULL;

        DBUG_PRINT ("traversing body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("leaving body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));

        INFO_FUNDEF (arg_info) = old_fundef;
        INFO_TOPBLOCK (arg_info) = old_topblock;
        INFO_VARDECS (arg_info) = old_vardecs;
        INFO_LHSTYPE (arg_info) = old_lhstype;

        if (FUNDEF_ISLACINLINE (arg_node)) {
            RMVdoRemoveVardecsOneFundef (arg_node);
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
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

    DBUG_ENTER ();

    if (NULL == INFO_TOPBLOCK (arg_info)) {
        INFO_TOPBLOCK (arg_info) = arg_node;
        INFO_VARDECS (arg_info) = NULL;
    }

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /* New vardecs go only at top level block */
    if (INFO_TOPBLOCK (arg_info) == arg_node) {
        INFO_TOPBLOCK (arg_info) = NULL;
        if (NULL != INFO_VARDECS (arg_info)) {
            BLOCK_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDECS (arg_info), BLOCK_VARDECS (arg_node));
            INFO_VARDECS (arg_info) = NULL;
        }
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

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
        DBUG_PRINT ("CFassign removed dead assignment");
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
    DBUG_ENTER ();
    node *res;

    /* check for constant condition */
    res = arg_node;
    if (TYisAKV (ID_NTYPE (FUNCOND_IF (arg_node)))
        && ((!FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
            || (!COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)))) {
        if (COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)) {
            res = DUPdoDupTree (FUNCOND_THEN (arg_node));
            arg_node = FREEdoFreeTree (arg_node);
            DBUG_PRINT ("CFfuncond found TRUE condition");
        } else {
            res = DUPdoDupTree (FUNCOND_ELSE (arg_node));
            arg_node = FREEdoFreeTree (arg_node);
            DBUG_PRINT ("CFfuncond found FALSE condition");
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

    DBUG_ENTER ();

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("CFcondThen found TRUE condition");

    /* select then-part for later insertion in assignment chain */
    pre = BLOCK_ASSIGNS (COND_THEN (arg_node));
    if (pre != NULL) { /* empty code block must not be moved */
        DBUG_ASSERT (NULL == INFO_PREASSIGN (arg_info), "CFcondThen preassign confusion");
        INFO_PREASSIGN (arg_info) = pre;
        /*
         * delete pointer to codeblock to preserve assignments from
         * being freed
         */
        BLOCK_ASSIGNS (COND_THEN (arg_node)) = NULL;
    }

    DBUG_RETURN (arg_node);
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

    DBUG_ENTER ();

    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
    DBUG_PRINT ("CFcondElse found FALSE condition");

    /* select else-part for later insertion in assignment chain */
    pre = BLOCK_ASSIGNS (COND_ELSE (arg_node));
    if (pre != NULL) { /* empty code block must not be moved */
        DBUG_ASSERT (NULL == INFO_PREASSIGN (arg_info), "CFcondElse preassign confusion");
        INFO_PREASSIGN (arg_info) = pre;
        /*
         * delete pointer to codeblock to preserve assignments from
         * being freed
         */
        BLOCK_ASSIGNS (COND_ELSE (arg_node)) = NULL;
    }

    DBUG_RETURN (arg_node);
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

    DBUG_ENTER ();

    /* check for constant COND */
    condknown = TYisAKV (ID_NTYPE (COND_COND (arg_node)));
    if (condknown) {
        condtrue = COisTrue (TYgetValue (ID_NTYPE (COND_COND (arg_node))), TRUE);
    }

    if (condtrue && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
        /*
         * if this is a do- or while function and the condition is evaluated
         * to true we have an endless loop. We MUST avoid doing CF, as it
         * won't terminate either!
         */
        CTIwarnLoc (NODE_LOCATION (arg_node),
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

        FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)) = FALSE;
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
    DBUG_ENTER ();

    /*
     * What's intended here is that the typechecker may determine
     * that the lhs is of type AKV, so it knows the value of the lhs.
     * Hence, it can discard the RHS and replace it by the now-known lhs value.
     */
    DBUG_ASSERT (LET_IDS (arg_node) != NULL, "empty LHS of let found in CF");
    DBUG_ASSERT (LET_EXPR (arg_node) != NULL, "empty RHS of let found in CF");
    DBUG_ASSERT (NULL == INFO_AVISMIN (arg_info), "AVISMIN non-NULL");
    DBUG_ASSERT (NULL == INFO_AVISMAX (arg_info), "AVISMAX non-NULL");
    DBUG_ASSERT (FALSE == INFO_DOINGEXTREMA (arg_info), "DOINGEXTREMA TRUE");

    DBUG_PRINT ("Looking at LHS: %s", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
    /**
     *  First, we collect the INFO_LHSTYPE!
     */
    INFO_LET (arg_info) = arg_node;
    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* If first result has extrema, add them now.
     */
    if (NULL != INFO_AVISMIN (arg_info)) {
        DBUG_ASSERT (N_avis == NODE_TYPE (INFO_AVISMIN (arg_info)),
                     "AVIS_MIN not N_avis");
        IVEXPsetMinvalIfNotNull (IDS_AVIS (LET_IDS (arg_node)), INFO_AVISMIN (arg_info));

        INFO_AVISMIN (arg_info) = NULL;
    }
    if (NULL != INFO_AVISMAX (arg_info)) {
        DBUG_ASSERT (N_avis == NODE_TYPE (INFO_AVISMAX (arg_info)),
                     "AVIS_MAX not N_avis");
        IVEXPsetMaxvalIfNotNull (IDS_AVIS (LET_IDS (arg_node)), INFO_AVISMAX (arg_info));
        INFO_AVISMAX (arg_info) = NULL;
    }

    if (TYisProdOfAKV (INFO_LHSTYPE (arg_info))
        && (NODE_TYPE (LET_EXPR (arg_node)) != N_funcond)) {
        /**
         * NB: we MUST not throw away funconds UNLESS we know the predicate!
         *     otherwise, we come into an inconsistent state which is not
         *     being dealt with in SAA insertion (cf bug 446).
         *     Therefore we HAVE TO traverse the RHS if these are funconds.
         */

        if (!CFisFullyConstantNode (LET_EXPR (arg_node))) {
            DBUG_PRINT ("LHS (%s) is AKV: replacing RHS by constant",
                        AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
            LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
            if (TYgetProductSize (INFO_LHSTYPE (arg_info)) == 1) {
                LET_EXPR (arg_node) = CFcreateConstExprsFromType (
                  TYgetProductMember (INFO_LHSTYPE (arg_info), 0));
            } else {
                LET_EXPR (arg_node)
                  = CFcreateConstExprsFromType (INFO_LHSTYPE (arg_info));
            }
            global.optcounters.cf_expr += TYgetProductSize (INFO_LHSTYPE (arg_info));
        }
    }

    /**
     *  If CF has replaced the RHS by an N_exprs chain, we have to break this
     *  up now!
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_exprs) {
        DBUG_PRINT ("RHS replaced by N_exprs chain in lhs %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
        INFO_POSTASSIGN (arg_info)
          = TCappendAssign (CreateAssignsFromIdsExprs (LET_IDS (arg_node),
                                                       LET_EXPR (arg_node),
                                                       INFO_LHSTYPE (arg_info)),
                            INFO_POSTASSIGN (arg_info));
        LET_EXPR (arg_node) = NULL;
        LET_IDS (arg_node) = NULL;
        INFO_REMASSIGN (arg_info) = TRUE;
    } else {
        LET_EXPR (arg_node)
          = PreventTypePrecisionLoss (LET_EXPR (arg_node),
                                      TYgetProductMember (INFO_LHSTYPE (arg_info), 0));
    }

    INFO_LHSTYPE (arg_info) = TYfreeTypeConstructor (INFO_LHSTYPE (arg_info));

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_PRINT ("CFarray looking at  %s",
                AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));
    exprs = ARRAY_AELEMS (arg_node);
    pat = PMarray (0, 2, PMarray (1, PMAgetFS (&fs), 1, PMskip (0)), PMskip (0));

    if (PMmatchFlat (pat, arg_node)) {

        pat2 = PMarray (2, PMAhasFS (&fs), PMAgetNode (&array), 1, PMskip (0));

        while ((exprs != NULL) && PMmatchFlat (pat2, EXPRS_EXPR (exprs))) {
            lexprs = TCappendExprs (lexprs, DUPdoDupTree (ARRAY_AELEMS (array)));
            exprs = EXPRS_NEXT (exprs);
        }

        if (exprs == NULL) {
            fshp = COconstant2Shape (fs);
            res
              = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (array)),
                             SHappendShapes (ARRAY_FRAMESHAPE (arg_node), fshp), lexprs);
            fshp = SHfreeShape (fshp);
            DBUG_PRINT ("N_array %s being expanded",
                        AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));
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

    res = CFunflattenSimpleScalars (res);

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

    DBUG_ENTER ();
    DBUG_PRINT ("evaluating prf %s", global.prf_name[PRF_PRF (arg_node)]);

    /* Bog-standard constant-folding is all handled by typechecker now */

    /* Try symbolic constant simplification */
    if (global.optimize.doscs) {
        DBUG_PRINT ("trying SCS...");
        res = InvokeCFprfAndFlattenExtrema (arg_node, arg_info,
                                            prf_cfscs_funtab[PRF_PRF (arg_node)], res);
        DBUG_PRINT ("   %s", (res == NULL) ? "not applicable" : "optimized!");
    }

    /* If that doesn't help, try structural constant constant folding */
    if (global.optimize.dosccf) {
        DBUG_PRINT ("%s SCCF...", (res == NULL) ? "trying" : "skipping");
        res = InvokeCFprfAndFlattenExtrema (arg_node, arg_info,
                                            prf_cfsccf_funtab[PRF_PRF (arg_node)], res);
        DBUG_PRINT ("   %s", (res == NULL) ? "not applicable" : "optimized!");
    }

    /* If that doesn't help, try SAA constant folding */
    if (global.optimize.dosaacf) {
        DBUG_PRINT ("%s SAACF...", (res == NULL) ? "trying" : "skipping");
        res = InvokeCFprfAndFlattenExtrema (arg_node, arg_info,
                                            prf_cfsaa_funtab[PRF_PRF (arg_node)], res);
        DBUG_PRINT ("   %s", (res == NULL) ? "not applicable" : "optimized!");
    }

    if (res != NULL) {
        /* free this primitive function and replace it by new node */
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = res;
        global.optcounters.cf_expr++; // increment constant folding counter
        DBUG_PRINT ("Optimized %s", AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));
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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    node *old_part;

    DBUG_ENTER ();

    old_part = INFO_PART (arg_info);
    INFO_PART (arg_info) = arg_node;

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
     * Traverse this partition's code if it has not yet been traversed.
     * Mark the code as completely traversed afterwards by inverting CODE_USED
     */
    arg_node = POLYSsetClearAvisPart (arg_node, arg_node);
    if (CODE_USED (PART_CODE (arg_node)) > 0) {
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
        CODE_USED (PART_CODE (arg_node)) *= -1;
    }
    arg_node = POLYSsetClearAvisPart (arg_node, NULL);

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

    INFO_PART (arg_info) = old_part;

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

#undef DBUG_PREFIX
