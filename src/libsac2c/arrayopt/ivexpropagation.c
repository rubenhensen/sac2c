/** <!--********************************************************************-->
 *
 * @defgroup ivexp Index Vector Extrema Propagation Traversal
 *
 * @brief:
 *
 *
 *  This code performs several related functions:
 *
 *    1. It propagates extrema (AVIS_MIN and AVIS_MAX) through
 *       assigns and, where we are able to do so, primitive functions.
 *
 *    2. It generates AST expressions to compute extrema for
 *       the results of N_array nodes and N_prf nodes, where possible.
 *
 *    3. It propagates WITHID_IDS indices.
 *
 *  Details:
 *    Assigns: RHS extrema, if present, are copied to LHS,
 *    after deleting LHS extrema, if RHS extrema differ from LHS extrema.
 *
 *    F_noteminval, F_notemaxval: LHS extrema, if present,
 *    are deleted if they do not match PRF_ARG2, and replaced
 *    by PRF_ARG2.
 *
 *    Primitives: See detailed discussion at: GenerateExtremaComputationsPrf
 *
 *    If extrema already exist on the LHS, they should be the same
 *    as those computed on the RHS, except in the case
 *    of guards. In the latter case, when CF removes a guard, it
 *    must also clear the extrema in the LHS, then this code
 *    will update it. We also let CF update the LHS, as in:
 *
 *        x', p = F_non_neg_val(x), where
 *
 *    In this case, we start out with AVIS_MIN(x') = x * 0;
 *    If we later learn more about x, we may delete the minimum
 *    and replace it with a better estimate.
 *
 *****************************************************************************
 *
 * @ingroup ivexp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivexpropagation.c
 *
 * Prefix: IVEXP
 *
 *****************************************************************************/
#include "ivexpropagation.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "IVEXP"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "tree_utils.h"
#include "type_utils.h"
#include "constants.h"
#include "tree_compound.h"
#include "pattern_match.h"
#include "ivextrema.h"
#include "flattengenerators.h"
#include "DupTree.h"
#include "check.h"
#include "globals.h"
#include "phase.h"
#include "shape.h"
#include "symbolic_constant_simplification.h"
#include "constant_folding.h"
#include "algebraic_wlfi.h"
#include "set_withloop_depth.h"
#include "new_typecheck.h"

#include "string.h" /* BobboBreaker */

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *minval;
    node *maxval;
    node *preassigns;
    node *postassigns;
    node *vardecs;
    node *curwith;
    node *let;
    node *withidids;
    int defdepth;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_MINVAL(n) ((n)->minval)
#define INFO_MAXVAL(n) ((n)->maxval)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_CURWITH(n) ((n)->curwith)
#define INFO_LET(n) ((n)->let)
#define INFO_WITHIDIDS(n) ((n)->withidids)
#define INFO_DEFDEPTH(n) ((n)->defdepth)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_MINVAL (result) = NULL;
    INFO_MAXVAL (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_CURWITH (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_WITHIDIDS (result) = NULL;
    INFO_DEFDEPTH (result) = 0;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEXPdoIndexVectorExtremaProp( node *arg_node)
 *
 * @brief: Perform index vector extrema propagation on a function.
 *
 *****************************************************************************/
node *
IVEXPdoIndexVectorExtremaProp (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "IVEXPdoIndexVectorExtremaPropexpected N_fundef");

    DBUG_PRINT ("Starting index vector extrema propagation traversal");
    arg_info = MakeInfo ();

    TRAVpush (TR_ivexp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("Index vector extrema propagation complete.");

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

#ifdef FIXME
/** <!--********************************************************************-->
 *
 * @fn static node *BuildExtremaNonnegval( node *arg_node, info *arg_info)
 *
 * @brief Construct extrema computation for F_non_neg_val_V, unless
 *        it is already there.
 *
 *        We start with:
 *
 *          v', p' =  F_non_neg_val_V( v);
 *
 *        And generate:
 *
 *          zr = 0;
 *          p = _ge_VxS_( v, zr);
 *          v', p' = F_non_neg_val_V( v, p);
 *          wrong! We should generate:
 *
 *          zr = 0;
 *          minv = _max_VxS_( AVIS_MIN( v), zr);
 *          maxv = _max_VxS_( AVIS_MAX( v), zr);
 *          v'' - F_non_neg_val_V( v);
 *          v' = F_attachextrema(v'', minv, maxv);
 *
 * @param: arg_node: N_prf of F_non_neg_val_V.
 *         arg_info: your basic arg_info.
 *
 * @result: Possibly updated N_prf node.
 *
 *****************************************************************************/
static node *
BuildExtremaNonnegval (node *arg_node, info *arg_info)
{
    node *zr;
    node *zavis;
    node *zass;
    node *zids;
    int shp;

    DBUG_ENTER ();
    DBUG_PRINT_TAG ("IVEXI", "completely untested yet");

    under construction

      if (NULL == PRF_ARG2 (arg_node))
    { /* Do we already have extrema? */
        /* Construct zr */
        zr = makeIntScalar (0, &INFO_VARDECS (arg_info), &INFO_PREASSIGNSWITH (arg_info));

        /* Construct Boolean vector, p */
        shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (PRF_ARG1 (arg_node))));
        zavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                      SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (zavis, INFO_VARDECS (arg_info));
        zids = TBmakeIds (zavis, NULL);
        zass
          = TBmakeAssign (TBmakeLet (zids,
                                     TCmakePrf2 (F_ge_VxS,
                                                 DUPdoDupNode (PRF_ARG1 (arg_node)), zr)),
                          NULL);
        AVIS_SSAASSIGN (zavis) = zass;

        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (zass))) = TRUE;

        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), zass);

        /* Attach new extrema to N_prf */
        PRF_ARG2 (arg_node) = TBmakeExprs (TBmakeId (zavis), NULL);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildExtremaShape( node *arg_node, info *arg_info)
 *
 * @brief Construct Extrema computation for F_shape, unless
 *        it is already there.
 *
 *          v' =  F_shape_A( arr);
 *
 *        And generate:
 *
 *          shp = _shape_A_( arr);
 *          minv = [ 0, 0, ... 0];    NB. shp * 0
 *          v' = _attachextrema( shp, minv, shp);
 *
 * @param: arg_node: N_prf
 *         arg_info: your basic arg_info.
 *
 * @result: Possibly updated N_prf node.
 *
 *****************************************************************************/
static node *
BuildExtremaShape (node *arg_node, info *arg_info)
{
    node *zr;
    node *neg1;
    shape *shp;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("IVEXI", "completely untested yet");

    if (!PRF_EXTREMAATTACHED (arg_node)) {
        PRF_EXTREMAATTACHED (arg_node) = TRUE;

        /* Generate minv */
        zr = makeIntScalar (0, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));

        /* Generate minv */
        shp = xxx;
        pavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                      SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (pavis, INFO_VARDECS (arg_info));
        pids = TBmakeIds (pavis, NULL);
        pass
          = TBmakeAssign (TBmakeLet (pids, TCmakePrf2 (F_lt_VxV,
                                                       DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       DUPdoDupNode (shpids))),
                          NULL);
        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (pass))) = TRUE;
        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), pass);
        AVIS_SSAASSIGN (pavis) = pass;
        00000000000000000
          /* Generate shp */
          shp
          = SHgetUnrLen (TYgetShape (AVIS_TYPE (PRF_ARG1 (arg_node))));
        shpavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (shpavis, INFO_VARDECS (arg_info));
        shpids = TBmakeIds (shpavis, NULL);
        shpass
          = TBmakeAssign (TBmakeLet (shpids,
                                     TCmakePrf1 (F_shape_A,
                                                 DUPdoDupNode (PRF_ARG2 (arg_node)))),
                          NULL);
        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (shpass))) = TRUE;
        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), shpass);
        AVIS_SSAASSIGN (shpavis) = shpass;

        /* Generate p */
        pavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                      SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (pavis, INFO_VARDECS (arg_info));
        pids = TBmakeIds (pavis, NULL);
        pass
          = TBmakeAssign (TBmakeLet (pids, TCmakePrf2 (F_lt_VxV,
                                                       DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       DUPdoDupNode (shpids))),
                          NULL);
        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (pass))) = TRUE;
        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), pass);
        AVIS_SSAASSIGN (pavis) = pass;

        /* Attach new extremum to N_prf */
        PRF_ARG3 (arg_node) = TBmakeExprs (TBmakeId (pavis), NULL);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildExtremumValLtShape( node *arg_node, info *arg_info)
 *
 * @brief Construct Extremum computation for F_val_lt_shape_VxA, unless
 *        it is already there.
 *
 *        In the former case, we start with:
 *
 *          v' =  F_val_lt_shape_VxA( v, arr);
 *
 *        And generate:
 *
 *          shp = _shape_A_( arr);
 *          p = _lt_VxV_( v, shp);
 *          v' = F_val_lt_shape_VxA( v, arr, p);
 *
 *        CF should not remove this guard, but
 *        the guard can be exploited by EWLF to generate
 *        better code when p is true. The guard should be removed
 *        after saacyc, IFF p is true.
 *
 *        FIXME: I am not sure what we can do with AVIS_MIN( ID_AVIS( v')),
 *               if anything.
 *
 * @param: arg_node: N_prf
 *         arg_info: your basic arg_info.
 *
 * @result: Possibly updated N_prf node.
 *
 *****************************************************************************/
static node *
BuildExtremumValLtShape (node *arg_node, info *arg_info)
{
    node *shpavis;
    node *shpass;
    node *shpids;
    node *pavis;
    node *pass;
    node *pids;
    int shp;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("IVEXI", "completely untested yet");
    if (NULL == PRF_ARG3 (arg_node)) { /* Do we already have extrema? */

        /* Generate shp */
        shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (PRF_ARG1 (arg_node))));
        shpavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (shpavis, INFO_VARDECS (arg_info));
        shpids = TBmakeIds (shpavis, NULL);
        shpass
          = TBmakeAssign (TBmakeLet (shpids,
                                     TCmakePrf1 (F_shape_A,
                                                 DUPdoDupNode (PRF_ARG2 (arg_node)))),
                          NULL);
        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (shpass))) = TRUE;
        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), shpass);
        AVIS_SSAASSIGN (shpavis) = shpass;

        /* Generate p */
        pavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                      SHcreateShape (1, shp)));
        INFO_VARDECS (arg_info) = TBmakeVardec (pavis, INFO_VARDECS (arg_info));
        pids = TBmakeIds (pavis, NULL);
        pass
          = TBmakeAssign (TBmakeLet (pids, TCmakePrf2 (F_lt_VxV,
                                                       DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       DUPdoDupNode (shpids))),
                          NULL);
        /* Keep us from trying to add extrema to the extrema calculations.  */
        PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_STMT (pass))) = TRUE;
        INFO_PREASSIGNSWITH (arg_info)
          = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), pass);
        AVIS_SSAASSIGN (pavis) = pass;

        /* Attach new extremum to N_prf */
        PRF_ARG3 (arg_node) = TBmakeExprs (TBmakeId (pavis), NULL);
    }
    DBUG_RETURN (arg_node);
}

#endif // FIXME under construction

/******************************************************************************
 *
 * description: Predicates for determining if an N_avis node have extrema,
 *              and macro for setting extrema.
 *
 * @params  arg_node: an N_avis node.
 * @result: True if the node has desired extrema present.
 *
 ******************************************************************************/
bool
IVEXPisAvisHasMin (node *avis)
{
    bool z;

    DBUG_ENTER ();
    z = ((NULL != avis) && NULL != AVIS_MIN (avis));
    DBUG_RETURN (z);
}

bool
IVEXPisAvisHasMax (node *avis)
{
    bool z;

    DBUG_ENTER ();
    z = ((NULL != avis) && NULL != AVIS_MAX (avis));
    DBUG_RETURN (z);
}

bool
IVEXPisAvisHasBothExtrema (node *avis)
{
    bool z;

    DBUG_ENTER ();
    z = IVEXPisAvisHasMin (avis) && IVEXPisAvisHasMax (avis);
    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function: node *makeNarray( node *extrema, info *arg_info, ntype *typ,
 *                             node *nar)
 *
 * description: Create an N_array from an N_exprs chain at extrema,
 *              of frame shape arrxrho. Also creates an assign and
 *              vardec for same.
 *
 * @params:  extrema: an N_exprs chain.
 *           typ: the N_type for the N_array
 *           nar: The N_array for which we are building extrema.
 * @result:
 *           The N_avis for the shiny new N_array.
 *           or NULL if extrema is NULL;
 *
 *****************************************************************************/
static node *
makeNarray (node *extrema, ntype *typ, node *nar, node **vardecs, node **preassigns)
{
    node *narr;
    node *zavis = NULL;

    DBUG_ENTER ();

    if (NULL != extrema) {
        DBUG_ASSERT (N_exprs == NODE_TYPE (extrema), "Expected N_exprs");
        /* Copy the original array, and overwrite its elements */
        narr = DUPdoDupNode (nar);
        ARRAY_AELEMS (narr) = extrema;
#ifdef FIXME
        narr = CFunflattenSimpleScalars (narr);
#endif // ifdef FIXME
        zavis = FLATGexpression2Avis (narr, vardecs, preassigns, TYeliminateAKV (typ));
    }

    DBUG_RETURN (zavis);
}

/******************************************************************************
 *
 * function: void IVEXPsetMinvalIfNotNull( node *snk, node *minv);
 * function: void IVEXPsetMaxvalIfNotNull( node *snk, node *maxv);
 *
 * description: Set extremum from min/maxv if it is not NULL.
 *              If the snk is not NULL, free it first.
 *
 * @params:     snk: an N_avis
 *              minv/maxv: an N_avis or NULL.
 *
 * @result: If src is NULL, or if its ID_AVIS matches minv/maxv,
 *          do nothing.
 *          Otherwise, the snk pointer, if non-NULL, is freed,
 *          and overwritten by an N_id created from minv/maxv.
 *
 ******************************************************************************/
void
IVEXPsetMinvalIfNotNull (node *snk, node *minv)
{

    DBUG_ENTER ();

    if (NULL != minv) {
        DBUG_ASSERT (N_avis == NODE_TYPE (minv), "Expected N_avis minv");
        if (NULL == AVIS_MIN (snk)) {
            AVIS_MIN (snk) = TBmakeId (minv);
            AVIS_ISMINHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MIN(%s) set to %s", AVIS_NAME (snk), AVIS_NAME (minv));
            global.optcounters.ivexp_expr++;
        } else if ((minv != ID_AVIS (AVIS_MIN (snk)))) {
            FREEdoFreeNode (AVIS_MIN (snk));
            AVIS_MIN (snk) = TBmakeId (minv);
            AVIS_ISMINHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MIN(%s) set to %s", AVIS_NAME (snk), AVIS_NAME (minv));
            global.optcounters.ivexp_expr++;
        }
    }

    DBUG_RETURN ();
}

void
IVEXPsetMaxvalIfNotNull (node *snk, node *maxv)
{

    DBUG_ENTER ();

    if (NULL != maxv) {
        DBUG_ASSERT (N_avis == NODE_TYPE (maxv), "Expected N_avis src");
        if (NULL == AVIS_MAX (snk)) {
            AVIS_MAX (snk) = TBmakeId (maxv);
            AVIS_ISMAXHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MAX(%s) set to %s", AVIS_NAME (snk), AVIS_NAME (maxv));
            global.optcounters.ivexp_expr++;
        } else if ((maxv != ID_AVIS (AVIS_MAX (snk)))) {
            FREEdoFreeNode (AVIS_MAX (snk));
            AVIS_MAX (snk) = TBmakeId (maxv);
            AVIS_ISMAXHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MAX(%s) set to %s", AVIS_NAME (snk), AVIS_NAME (maxv));
            global.optcounters.ivexp_expr++;
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   static
 *   node *buildExtremaChain( node *arg_node, int minmax)
 *
 * description:
 *        Little recursive extrema-chain builder,
 *        to get results in correct order.
 *        If no extremum, but value is constant, use that value.
 *
 * @params  arg_node: an N_array node.
 *          minmax: 0 for MIN, 1 for MAX
 *
 * @result: An N_exprs chain of extrema.
 *
 ******************************************************************************/
static node *
buildExtremaChain (node *exprs, int minmax)
{
    node *z = NULL;
    node *avis;
    node *m;
    constant *con;
    constant *kcon;

    DBUG_ENTER ();
    if (NULL != EXPRS_NEXT (exprs)) {
        z = buildExtremaChain (EXPRS_NEXT (exprs), minmax);
    }
    avis = ID_AVIS (EXPRS_EXPR (exprs));
    m = (0 == minmax) ? AVIS_MIN (avis) : AVIS_MAX (avis);
    if ((0 == m) && COisConstant (EXPRS_EXPR (exprs))) {
        m = EXPRS_EXPR (exprs);
        if (1 == minmax) { /* Normalize AVIS_MAX */
            con = COaST2Constant (m);
            kcon = COmakeConstantFromInt (1);
            con = COadd (con, kcon, NULL);
            m = COconstant2AST (con); /* May have to flatten this */
            con = COfreeConstant (con);
            kcon = COfreeConstant (kcon);
        }
    }

    DBUG_ASSERT (NULL != m, "Expected non-NULL m");
    z = TBmakeExprs (DUPdoDupNode (m), z);

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function: node *IVEXPisAllNarrayExtremaPresent( node *arg_node)
 *
 * description: Predicate for determining if an N_array node
 *              has all its AVIS_MIN/AVIS_MAX elements present, for
 *              non-constant elements.
 *
 * @params  arg_node: an N_array node.
 *          minmax: 0 to check AVIS_MIN, 1 to check AVIS_MAX
 *          arg_info: Your basic arg_info
 *
 * @result: A boolean, TRUE if all min/max vals are present for
 *          non-constant elements.
 *
 ******************************************************************************/
bool
IVEXPisAllNarrayExtremaPresent (node *arg_node, int minmax)
{
    node *exprs;
    node *avis;
    node *m;
    bool z;

    DBUG_ENTER ();

    /* Check that we have extrema for all non-constant array elements.
     * All N_array elements must be N_id nodes.
     */
    exprs = ARRAY_AELEMS (arg_node);
    z = (NULL != exprs);
    while (z && (NULL != exprs)) {
        if (N_id == NODE_TYPE (EXPRS_EXPR (exprs))) {
            avis = ID_AVIS (EXPRS_EXPR (exprs));
            m = (0 == minmax) ? AVIS_MIN (avis) : AVIS_MAX (avis);
            z = z && ((NULL != m) || COisConstant (EXPRS_EXPR (exprs)));
        } else {
            z = FALSE;
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPgenerateNarrayExtrema node *arg_node...)
 * description:
 *        Generate N_array extrema.
 *
 * @params  arg_node: an N_let node pointing to an N_array node.
 *
 * @result: Result is
 *
 *          We start with this N_let:
 *
 *               V = [ I, J, K];
 *
 *          If all N_array elements have non_null extrema,
 *          we build LHS extrema values as follows:
 *
 *            minv = [ AVIS_MIN( I), AVIS_MIN( J), AVIS_MIN( K) ]
 *            maxv = [ AVIS_MAX( I), AVIS_MAX( J), AVIS_MAX( K) ]
 *
 *          Then, we set the extrema on V.
 *          We do similar for withids.
 *
 *          If arg_node matches WITHIDS_VEC or WITHIDS_IDS,
 *          we can not generate an AVIS_WITHIDS for arg_node,
 *          because we would end up with a loop in the AST.
 *
 ******************************************************************************/
node *
IVEXPgenerateNarrayExtrema (node *arg_node, node **vardecs, node **preassigns)
{
    node *lhs;
    node *lhsavis;
    node *rhs;
    node *minv = NULL;
    node *maxv = NULL;
    node *z;

    DBUG_ENTER ();

    lhs = LET_IDS (arg_node);
    lhsavis = IDS_AVIS (lhs);
    rhs = LET_EXPR (arg_node);
    z = rhs;

    if (!TYisAKV (AVIS_TYPE (IDS_AVIS (lhs)))) {

        if ((!IVEXPisAvisHasMin (lhsavis)) && (IVEXPisAllNarrayExtremaPresent (rhs, 0))) {
            minv = buildExtremaChain (ARRAY_AELEMS (rhs), 0);
            minv = makeNarray (minv, AVIS_TYPE (lhsavis), rhs, vardecs, preassigns);
            IVEXPsetMinvalIfNotNull (lhsavis, minv);
        }

        if ((!IVEXPisAvisHasMax (lhsavis)) && (IVEXPisAllNarrayExtremaPresent (rhs, 1))) {
            maxv = buildExtremaChain (ARRAY_AELEMS (rhs), 1);
            maxv = makeNarray (maxv, AVIS_TYPE (lhsavis), rhs, vardecs, preassigns);
            IVEXPsetMaxvalIfNotNull (lhsavis, maxv);
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 *
 * @fn node *IVEXPadjustExtremaBound(node *arg_node, int k,
 *                                   node **vardecs, node **preassigns)
 *
 *   @brief arg_node is an N_avis of an AVIS_MIN/AVIS_MAX,
 *          that needs adjusting, by adding +1 or -1 to it.
 *          Alternately, arg_node can be an N_num.
 *
 *          Because arg_node can also be a WITHID_IDS node,
 *          we have to decide whether to generate vector
 *          or scalar add.
 *
 *          We generate, along with a vardec for b2':
 *
 *          b2' = _add_VxS_( b2, k);
 *
 *          NULL arg_node is ignored.
 *
 *          If arg_node points to an N_array, each element of
 *          the N_array is adjusted using _add_SxS_(). This
 *          is required by BuildInverseProjections, which is
 *          unable to exploit CF to do the job.
 *
 *   @param  arg_node: an N_avis node or N_num node.
 *           int k:    Constant value to be used.
 *           vardecs:  Address of a vardecs chain that we will append to.
 *           preassigns: Address of a preassigns chain we will append to.
 *
 *   @return The N_avis result of the adjusted computation.
 *           If the argument was an N_num, the result is also an N_num.
 *
 ******************************************************************************/
node *
IVEXPadjustExtremaBound (node *arg_node, int k, node **vardecs, node **preassigns,
                         char *tagit)
{
    node *zavis = NULL;
    node *zids;
    node *zass;
    node *kavis;
    node *argarray = NULL;
    node *aelems = NULL;
    node *argid;
    pattern *pat;
    size_t i;
    size_t lim;
    node *el;
    node *zarr;
    node *z;
    constant *con;
    constant *kcon;
    prf op;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        kavis = IVEXImakeIntScalar (k, vardecs, preassigns);
        zavis
          = TBmakeAvis (TRAVtmpVarName (tagit), TYeliminateAKV (AVIS_TYPE (arg_node)));
        *vardecs = TBmakeVardec (zavis, *vardecs);
        zids = TBmakeIds (zavis, NULL);

        pat = PMarray (1, PMAgetNode (&argarray), 1, PMskip (0));
        argid = TBmakeId (arg_node);
        if ((PMmatchFlat (pat, argid))) {
            /* Unrolled version of function for N_array nodes */
            lim = SHgetUnrLen (TYgetShape (AVIS_TYPE (arg_node)));
            for (i = 0; i < lim; i++) {
                el = TCgetNthExprsExpr (i, ARRAY_AELEMS (argarray));
                if (N_num == NODE_TYPE (el)) { /* [50], etc. */
                    kcon = COmakeConstantFromInt (k);
                    con = COaST2Constant (el);
                    con = COadd (con, kcon, NULL);
                    z = COconstant2AST (con);
                    kcon = COfreeConstant (kcon);
                    con = COfreeConstant (con);
                } else {
                    z = IVEXPadjustExtremaBound (ID_AVIS (el), k, vardecs, preassigns,
                                                 tagit);
                    z = TBmakeId (z);
                    DBUG_ASSERT (NULL != z, "Expected non-null result");
                }
                aelems = TCappendExprs (aelems, TBmakeExprs (z, NULL));
            }
            zarr = DUPdoDupNode (argarray);
            ARRAY_AELEMS (zarr) = aelems;
            zass = TBmakeAssign (TBmakeLet (zids, zarr), NULL);
        } else {
            op = TUisScalar (AVIS_TYPE (arg_node)) ? F_add_SxS : F_add_VxS;
            zass = TBmakeAssign (TBmakeLet (zids, TCmakePrf2 (op, TBmakeId (arg_node),
                                                              TBmakeId (kavis))),
                                 NULL);
        }
        argid = FREEdoFreeNode (argid);
        AVIS_SSAASSIGN (zavis) = zass;
        *preassigns = TCappendAssign (*preassigns, zass);

        if ((-1) == k) {
            AVIS_ISMINHANDLED (zavis) = TRUE;
        } else {
            AVIS_ISMAXHANDLED (zavis) = TRUE;
        }

        DBUG_PRINT ("introduced adjustment named: %s for: %s", AVIS_NAME (zavis),
                    AVIS_NAME (arg_node));
        pat = PMfree (pat);
    }

    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * Description: Emit code to invoke monadic function on AVIS_MIN/MAX
 *
 * @params  minmaxavis: N_avis for PRF_ARG1.
 * @return N_avis node for result.
 ******************************************************************************/
static node *
InvokeMonadicFn (node *minmaxavis, node *lhsavis, node *rhs, info *arg_info)
{
    DBUG_ENTER ();

    minmaxavis = TCmakePrf1 (PRF_PRF (rhs), TBmakeId (minmaxavis));
    minmaxavis = FLATGexpression2Avis (minmaxavis, &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGNS (arg_info),
                                       TYeliminateAKV (AVIS_TYPE (lhsavis)));
    DBUG_RETURN (minmaxavis);
}

/** <!--********************************************************************-->
 *
 * Description:  Generate extrema for C99 modulus, and for
 *               modulus extended to support
 *               negative arg1 values and zero arg2 values,
 *               per the APL ISO N8485 standard.
 *
 *                            count           modulus
 *               For C99, if (arg1 >=0 ) && ( arg2 > 0):
 *                 AVIS_MIN( res) = 0
 *                 AVIS_MAX( normalize( arg2 - 1));
 *
 *               For aplmod modulus,
 *                    count           modulus
 *               if ( arg1 >= 0) && ( arg2 >= 0), or
 *                                  ( arg2 >= 1)
 *                 then AVIS_MIN( res) = 0
 *
 *                    count          modulus
 *               if ( arg1 > 0) && ( arg2 > 0), then
 *                 AVIS_MAX( normalize( arg2 - 1));
 *
 *               This is wrong if the modulus is 1, but I think
 *               that is harmless.
 *
 *               If the count may be zero, then we must not
 *               set AVIS_MAX, because that will definitely break things,
 *               such as AWLF unit test rotateAKSAKVEmpty.sac.
 *
 * @params arg_node: Your basic N_let node.
 *         arg_info: As usual.
 *         aplmod: if this is APL modulus, TRUE; else FALSE.
 *
 * @return Same arg_node, but with potential side effects on INFO_MINVAL and
 *         INFO_MAXVAL.
 *         Requires AKS arguments.
 *
 * @comment It would be nice to handle the case of arg2 == 0,
 *          but perhaps another day...
 *
 ******************************************************************************/
static node *
GenerateExtremaModulus (node *arg_node, info *arg_info, bool aplmod)
{
    node *arg1avis;
    node *arg2avis;
    node *lhsavis;
    node *rhs;
    node *zr;
    node *nsa;
    bool arg1scalar;
    bool arg2scalar;
    bool isok;

    DBUG_ENTER ();

    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    rhs = LET_EXPR (arg_node);
    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));
    arg1scalar = TUisIntScalar (AVIS_TYPE (arg1avis));
    arg2scalar = TUisIntScalar (AVIS_TYPE (arg2avis));
    /* non-scalar argument */
    nsa = arg2scalar ? PRF_ARG1 (rhs) : PRF_ARG2 (rhs);

    // MINVAL
    if ((!IVEXPisAvisHasMin (lhsavis))
        && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))) {
        isok = (SCSisNonNegative (PRF_ARG1 (rhs)))
               && (((!aplmod) && SCSisPositive (PRF_ARG2 (rhs)))
                   || ((aplmod) && SCSisNonNegative (PRF_ARG2 (rhs))));
        isok = isok || (aplmod && SCSisPositive (PRF_ARG2 (rhs)));
        if (isok) {
            zr = SCSmakeZero (nsa); /* Create zero minval */
            if (NULL != zr) {       // nsa may not be AKS
                zr = FLATGexpression2Avis (zr, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info),
                                           TYeliminateAKV (AVIS_TYPE (lhsavis)));
                AVIS_ISMINHANDLED (zr) = TRUE;
                INFO_MINVAL (arg_info) = zr;
            }
        }
    }

    // MAXVAL
    if ((!IVEXPisAvisHasMax (lhsavis))
        && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
        && (SCSisPositive (PRF_ARG2 (rhs)))
        && (((!aplmod) && SCSisNonNegative (PRF_ARG1 (rhs)))
            || ((aplmod) && SCSisPositive (PRF_ARG1 (rhs))))) {
        /* Create maximum from PRF_ARG2, unless it is scalar and
         * PRF_ARG1 is vector. We cheat here a bit because
         * we want the maximum to be PRF_ARG2-1, but we have to
         *oops normalize it, so we can just copy the value.
         */
        if ((nsa == PRF_ARG2 (rhs)) || arg1scalar) {
            INFO_MAXVAL (arg_info) = ID_AVIS (PRF_ARG2 (rhs));
        } else {
            // vector/scalar case: PRF_ARG2( rhs) + ( 0 * PRF_ARG2( rhs));
            zr = SCSmakeZero (nsa);
            if (NULL != zr) { // nsa may not be AKS
                zr = FLATGexpression2Avis (zr, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info),
                                           TYeliminateAKV (AVIS_TYPE (lhsavis)));
                zr = TCmakePrf2 (F_add_VxS, TBmakeId (zr),
                                 TBmakeId (ID_AVIS (PRF_ARG2 (rhs))));
                zr = FLATGexpression2Avis (zr, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info),
                                           TYeliminateAKV (AVIS_TYPE (lhsavis)));
                INFO_MAXVAL (arg_info) = zr;
                AVIS_ISMAXHANDLED (zr) = TRUE;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Description: Generate normalized extrema calculation for dyadic primitives
 *
 * @params See callers
 *
 * @return No explicit result. Updated INFO_MINVAL, INFO_MAXVAL, side effects
 *         from new vardecs and assigns.
 *
 ******************************************************************************/
static void
GenExCalc (node *rhs, node *minarg1, node *minarg2, node *maxarg1, node *maxarg2,
           node *lhsavis, info *arg_info)
{
    node *minv = NULL;
    node *maxv = NULL;

    DBUG_ENTER ();

    if (NULL != minarg1) {
        DBUG_ASSERT (NULL != minarg2, "NULL minarg2!");
        minv = TCmakePrf2 (PRF_PRF (rhs), TBmakeId (minarg1), TBmakeId (minarg2));
        minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                     &INFO_PREASSIGNS (arg_info),
                                     TYeliminateAKV (AVIS_TYPE (lhsavis)));
        INFO_MINVAL (arg_info) = minv;
    }

    if (NULL != maxarg1) {
        DBUG_ASSERT (NULL != maxarg2, "NULL maxarg2!");
        maxv = TCmakePrf2 (PRF_PRF (rhs), TBmakeId (maxarg1), TBmakeId (maxarg2));
        maxv = FLATGexpression2Avis (maxv, &INFO_VARDECS (arg_info),
                                     &INFO_PREASSIGNS (arg_info),
                                     TYeliminateAKV (AVIS_TYPE (lhsavis)));
        /* Normalize maxv */
        maxv = IVEXPadjustExtremaBound (maxv, +1, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNS (arg_info), "gexc");
        INFO_MAXVAL (arg_info) = maxv;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * Description: Generate extrema computation for
 *              dyadic scalar function.
 *
 *    We want to compute the extrema between
 *    a constant argument and the extremum of the non-constant
 *    argument. This gives us these cases for commutative functions:
 *
 *      f( const, nonconst), with minval on non-const:
 *          --> minval = f( const, minval)
 *      f( nonconst, const), with minval on non-const:
 *          --> minval = f( minval, const)
 *
 *      f( non_const, non_const) with minval on first arg:
 *          --> minval = f( minval, non_const)
 *      f( non_const, non_const) with minval on second arg:
 *          --> minval = f( non_const, minval)
 *
 *  For maxval, we have to pre- and post-adjust AVIS_MAX, or _mul_
 *  will get wrong answer, i.e.:
 *
 *      f( const, nonconst), with maxval on non-const:
 *          --> maxval =  Normalize( f( const, Denormalize( maxval)))
 *      f( nonconst, const), with maxval on non-const:
 *          --> maxval =  Normalize( f( Denormalize( maxval), const))
 *
 *      f( non_const, non_const) with maxval on first arg:
 *          --> maxval = Normalize( f( Denormalize( maxval), non_const))
 *      f( non_const, non_const) with maxval on second arg:
 *          --> maxval = Normalize( f( non_const, Denormalize( maxval)))
 *
 * For subtract (the only non-commutative function), we have:
 *
 *     For Min or Max on PRF_ARG1:
 *        minmax = minmax( PRF_ARG1) - PRF_ARG2.
 *
 *     For Min or Max on PRF_ARG2:
 *        min = PRF_ARG1 - Denormalize( AVIS_MAX( PRF_ARG2));
 *        max = Normalize( PRF_ARG1 - AVIS_MIN( PRF_ARG2));
 *
 * @return argument arg_node is returned, unchanged.
 *         All changes are side effects.
 *
 * @note In the following, "normalized" means that extrema follow
 *       the same rules as with-loop bounds. I.e., AVIS_MIN
 *       is exact, like GENERATOR_BOUND1, and AVIS_MAX is one
 *       greater, like GENERATOR_BOUND2. Hence, we have:
 *
 *         Normalizeize( x) = x + 1;
 *         Denormalizeize( x) = x - 1;
 *
 * @note If the min/max derives from a WITHID, we accept
 *       the extrema only on that argument.
 *       Otherwise, sumrotateiota2AKD.sac, realrelaxAKD.sac, and
 *       other benchmarks will fail. This would happen if
 *       both N_prf args have a min/max and we pick the wrong
 *       one to propagate, or if only one has a min/max, and it
 *       is not the one derived from a WITHID.
 *
 *       This restriction may degrade the amount of guard removal
 *       that can be done statically. If this is a serious problem,
 *       consider rewriting AWLFI to avoid extrema entirely,
 *       by computing the intersection using the search method
 *       used by AWLFI in BuildInverseProjections.
 *       That rewriting should actually allow elimination of
 *       IVEXP and IVEXI, and should speed up compilation with
 *       -doawlf considerably, due to less code explosion.
 *
 ******************************************************************************/
static node *
GenerateExtremaComputationsCommutativeDyadicScalarPrf (node *arg_node, info *arg_info)
{
    node *minarg1 = NULL;
    node *minarg2 = NULL;
    node *maxarg1 = NULL;
    node *maxarg2 = NULL;
    node *lhsavis;
    node *rhs;
    bool min1;
    bool min2;
    bool max1;
    bool max2;
    int parentarg;
    node *wid;
    node *arg1avis;
    node *arg2avis;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));

    /* Slight dance to circumvent bug #693, in which DL generates
     * PRF_ARGs with N_num nodes.
     */
    min1 = IVEXPisAvisHasMin (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));
    max1 = IVEXPisAvisHasMax (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));

    min2 = IVEXPisAvisHasMin (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));
    max2 = IVEXPisAvisHasMax (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));

    parentarg = AWLFIfindPrfParent2 (rhs, INFO_WITHIDIDS (arg_info), &wid);
    switch (parentarg) {
    case 1: // PRF_ARG1 derives from WITHID
        min2 = FALSE;
        max2 = FALSE;
        break;

    case 2: // PRF_ARG2 derives from WITHID
        min1 = FALSE;
        max1 = FALSE;
        break;

    default:
        break;
    }

    /* Compute AVIS_MIN, perhaps */
    if (!IVEXPisAvisHasMin (lhsavis)) {
        if (min1) {
            minarg1 = ID_AVIS (AVIS_MIN (arg1avis));
            minarg2 = ID_AVIS (PRF_ARG2 (rhs));
        } else if (min2) {
            minarg1 = ID_AVIS (PRF_ARG1 (rhs));
            minarg2 = ID_AVIS (AVIS_MIN (arg2avis));
        }
    }

    /* Compute AVIS_MAX, perhaps */
    if (!IVEXPisAvisHasMax (lhsavis)) {
        if (max1) {
            maxarg1 = ID_AVIS (AVIS_MAX (arg1avis));
            maxarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               maxarg1, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "dsf3");
            maxarg2 = ID_AVIS (PRF_ARG2 (rhs));
        } else if (max2) {
            maxarg1 = ID_AVIS (PRF_ARG1 (rhs));
            maxarg2 = ID_AVIS (AVIS_MAX (arg2avis));
            maxarg2 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               maxarg2, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "dsf4");
        }
    }

    GenExCalc (rhs, minarg1, minarg2, maxarg1, maxarg2, lhsavis, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Description: Conditionally set AVIS_MIN(z ) = 0
 *              for z = mul( x, x);
 *
 * @params arg_node: N_let node for mul()
 *
 * @return N_let node, with possible side effect on AVIS_MIN( z)
 *
 ******************************************************************************/
static node *
XtimesX (node *arg_node, info *arg_info)
{
    node *rhs;
    node *lhsavis;
    node *arg;
    node *minv;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    if ((!IVEXPisAvisHasMin (lhsavis)) && (SCSisMatchPrfargs (rhs, NULL))) {
        if (F_mul_SxV == PRF_PRF (rhs)) {
            arg = PRF_ARG2 (rhs);
        } else { // SxS VxS VxS
            arg = PRF_ARG1 (rhs);
        }
        minv = SCSmakeZero (arg);
        minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                     &INFO_PREASSIGNS (arg_info), NULL);
        IVEXPsetMinvalIfNotNull (lhsavis, minv);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function: node *GenerateExtremaComputationsMultiply(node *arg_node, info *arg_info)
 *
 * description: Generate extrema for multiply.
 *
 * @params  arg_node: an N_prf node
 *          arg_info: your basic arg_info
 *
 * @result: Regardless of argument order, if we have:
 *
 *             f( non_negative_const, nonconst), with minval on non-const
 *
 *          then:
 *
 *             minval = f( non_negative_const,       minval)
 *
 *           Ditto for maxval, except that we denormalize and normalize:
 *
 *             maxval = N( f( non_negative_const, D( maxval)))
 *
 *           If the constant is negative, we have to swap extrema:
 *
 *             minval = f( negative_constant,     D( maxval))
 *             maxval = N( negative_constant,        minval)
 *
 *           Also, if the "const" is non-constant, but has a
 *           constant minval/maxval, so that we can determine signum( const),
 *           then treat it the same way.
 *
 *           Case 42: If we have no extrema, but
 *           have ( N times non_negative_even_const),
 *           then minval = 0.
 *
 * @note: See note in previous section re WITHID.
 *
 ******************************************************************************/
static node *
GenerateExtremaComputationsMultiply (node *arg_node, info *arg_info)
{
    node *minarg1 = NULL;
    node *minarg2 = NULL;
    node *maxarg1 = NULL;
    node *maxarg2 = NULL;
    node *lhsavis;
    node *rhs;
    bool min1;
    bool min2;
    bool max1;
    bool max2;
    int parentarg;
    node *wid = NULL;
    node *arg1avis;
    node *arg2avis;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));

    /* Slight dance to circumvent bug #693, in which DL generates
     * unflattened PRF_ARGs with N_num nodes.
     */

    min1 = IVEXPisAvisHasMin (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));
    max1 = IVEXPisAvisHasMax (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));

    min2 = IVEXPisAvisHasMin (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));
    max2 = IVEXPisAvisHasMax (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));

    parentarg = AWLFIfindPrfParent2 (rhs, INFO_WITHIDIDS (arg_info), &wid);
    switch (parentarg) {
    case 1: // PRF_ARG1 derives from WITHID, so don't use extrema from PRF_ARG2
        min2 = FALSE;
        max2 = FALSE;
        break;

    case 2: // PRF_ARG2 derives from WITHID, so don't use extrema from PRF_ARG1
        min1 = FALSE;
        max1 = FALSE;
        break;

    default:
        break;
    }

    /* AVIS_MIN is present */
    if (min1) {
        if ((!IVEXPisAvisHasMin (lhsavis)) && (SCSisNonNegative (PRF_ARG2 (rhs)))) {
            minarg1 = ID_AVIS (AVIS_MIN (arg1avis));
            minarg2 = arg2avis;
        }
        if ((!IVEXPisAvisHasMax (lhsavis)) && (SCSisNegative (PRF_ARG2 (rhs)))) {
            maxarg1 = ID_AVIS (AVIS_MIN (arg1avis));
            maxarg2 = arg2avis;
        }
    }

    if (min2) {
        if ((!IVEXPisAvisHasMin (lhsavis)) && (SCSisNonNegative (PRF_ARG1 (rhs)))) {
            minarg1 = arg1avis;
            minarg2 = ID_AVIS (AVIS_MIN (arg2avis));
        }
        if ((!IVEXPisAvisHasMax (lhsavis)) && (SCSisNegative (PRF_ARG1 (rhs)))) {
            maxarg1 = arg1avis;
            maxarg2 = ID_AVIS (AVIS_MIN (arg2avis));
        }
    }

    /* AVIS_MAX1 is present */
    if (max1) {
        if ((!IVEXPisAvisHasMax (lhsavis)) && (SCSisNonNegative (PRF_ARG2 (rhs)))) {
            maxarg1 = ID_AVIS (AVIS_MAX (arg1avis));
            maxarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               maxarg1, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "muldenorm1");
            maxarg2 = arg2avis;
        }

        if ((!IVEXPisAvisHasMin (lhsavis)) && (SCSisNegative (PRF_ARG2 (rhs)))) {
            minarg1 = ID_AVIS (AVIS_MAX (arg1avis));
            minarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               minarg1, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "muldenorm2");
            minarg2 = arg2avis;
        }
    }

    /* AVIS_MAX2 is present */
    if (max2) {
        if ((!IVEXPisAvisHasMax (lhsavis)) && (SCSisNonNegative (PRF_ARG1 (rhs)))) {
            maxarg1 = arg1avis;
            maxarg2 = ID_AVIS (AVIS_MAX (arg2avis));
            maxarg2 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               maxarg2, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "muldenorm3");
        }

        if ((!IVEXPisAvisHasMin (lhsavis)) && (SCSisNegative (PRF_ARG1 (rhs)))) {
            minarg1 = arg1avis;
            minarg2 = ID_AVIS (AVIS_MAX (arg2avis));
            minarg2 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               minarg2, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "muldenorm4");
        }
    }

    GenExCalc (rhs, minarg1, minarg2, maxarg1, maxarg2, lhsavis, arg_info);

    arg_node = XtimesX (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Description:  Generate extrema for _sub_().
 *
 * @params arg_node: Your basic N_let node.
 *         arg_info: As usual.
 *
 * @return Same arg_node, but with potential side effects on INFO_MINVAL and
 *         INFO_MAXVAL.
 *
 * @notes:
 *
 *   Case 1: minv( z) = minv( A) - B);
 *
 *   Case 2: minv( z) = A - denormalize( maxv( B));
 *
 ******************************************************************************/
static node *
GenerateExtremaComputationsSubtract (node *arg_node, info *arg_info)
{ // F_sub_() only!

    node *minarg1 = NULL;
    node *minarg2 = NULL;
    node *maxarg1 = NULL;
    node *maxarg2 = NULL;
    node *lhsavis;
    node *rhs;
    bool min1;
    bool min2;
    bool max1;
    bool max2;
    int parentarg;
    node *wid = NULL;
    node *arg1avis;
    node *arg2avis;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));

    /* Slight dance to circumvent bug #693, in which DL generates
     * unflattened PRF_ARGs with N_num nodes.
     */

    min1 = IVEXPisAvisHasMin (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));
    max1 = IVEXPisAvisHasMax (arg1avis)
           && SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info));

    min2 = IVEXPisAvisHasMin (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));
    max2 = IVEXPisAvisHasMax (arg2avis)
           && SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info));

    parentarg = AWLFIfindPrfParent2 (rhs, INFO_WITHIDIDS (arg_info), &wid);
    switch (parentarg) {
    case 1: // PRF_ARG1 derives from WITHID
        min2 = FALSE;
        max2 = FALSE;
        break;

    case 2: // PRF_ARG2 derives from WITHID
        min1 = FALSE;
        max1 = FALSE;
        break;

    default:
        break;
    }

    /* Compute AVIS_MIN, perhaps */
    // I think that, ideally, we would want to compute both
    // of these, and take the min of those. It might just lead
    // to more problems, perhaps.

    if (!IVEXPisAvisHasMin (lhsavis)) {
        if (max2) {
            /*  Case 2: minv( z) = A - denormalize( maxv( B)); */
            minarg1 = arg1avis;
            minarg2 = ID_AVIS (AVIS_MAX (arg2avis));
            minarg2 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               minarg2, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "dsfmaxa");
        } else if (min1) {
            /*  Case 1: minv( z) = minv( A) - B);  */
            minarg1 = ID_AVIS (AVIS_MIN (arg1avis));
            minarg2 = arg2avis;
        }
    }

    /* Compute AVIS_MAX, perhaps */
    if (!IVEXPisAvisHasMax (lhsavis)) {
        if (max1) {
            /*  Case 1: maxv( z) =  normalize( denormalize( maxv( A)) - B); */
            maxarg1 = ID_AVIS (AVIS_MAX (arg1avis));
            maxarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                               maxarg1, -1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info), "dsfmaxb");
            maxarg2 = arg2avis;
        } else if (min2) {
            /*  Case 2: maxv( z) = normalize( A - minv( B)); */
            maxarg1 = arg1avis;
            maxarg2 = ID_AVIS (AVIS_MIN (arg2avis));
        }
    }

    GenExCalc (rhs, minarg1, minarg2, maxarg1, maxarg2, lhsavis, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Description: Scalar-extend arg1avis, if scalar, to shape of arg2.
 *              A non-scalar arg2 must be AKS/AKV.
 *
 * @params arg1avis: an N_avis
 *         arg2: an N_id
 *
 * @return N_avis node for flattened N_array result.
 *         If arg2 is scalar, N_id for arg1avis.
 *
 ******************************************************************************/
static node *
ScalarExtend (node *arg1avis, node *arg2, info *arg_info)
{
    node *z;
    shape *shp;
    node *arg2avis;

    DBUG_ENTER ();

    arg2avis = ID_AVIS (arg2);
    if ((TUisScalar (AVIS_TYPE (arg1avis))) && (!TUisScalar (AVIS_TYPE (arg2avis)))
        && (TYisAKV (AVIS_TYPE (arg2avis)) || TYisAKS (AVIS_TYPE (arg2avis)))) {
        shp = SHcopyShape (TYgetShape (AVIS_TYPE (arg2avis)));
        z = SCSmakeVectorArray (shp, TBmakeId (arg1avis));
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info), NULL);
    } else {
        z = arg1avis;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * Description: Attempt to compute AVIS_MAX for min( a, b).
 *
 * Generate combos of argument and extrema and see if CF can
 * simplify it.
 *
 * @params:
 *
 *
 ******************************************************************************/
static node *
MinOnExtrema (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *zavis;
    node *arg1;
    node *arg2;
    node *marg1;
    node *marg2;
    node *mprf;

    DBUG_ENTER ();

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    marg1 = AVIS_MAX (ID_AVIS (arg1));
    marg2 = AVIS_MAX (ID_AVIS (arg2));

    if (NULL != marg2) {
        mprf = DUPdoDupTree (arg_node);
        PRF_ARG2 (mprf) = FREEdoFreeNode (PRF_ARG2 (mprf));
        PRF_ARG2 (mprf) = DUPdoDupNode (marg2);
        z = SCSprf_min_SxS (mprf, arg_info);
        mprf = FREEdoFreeTree (mprf);
    }

    if (NULL == z) {
        if (NULL != marg1) {
            mprf = DUPdoDupTree (arg_node);
            PRF_ARG1 (mprf) = FREEdoFreeNode (PRF_ARG1 (mprf));
            PRF_ARG1 (mprf) = DUPdoDupNode (marg1);
            z = SCSprf_min_SxS (mprf, arg_info);
            mprf = FREEdoFreeTree (mprf);
        }
    }

    // INFO_MAXVAL wants N_avis, not N_id
    if ((NULL != z) && (N_id == NODE_TYPE (z))) {
        zavis = ID_AVIS (z);
        z = FREEdoFreeNode (z);
        z = zavis;
    }

    // Flatten any result
    if (NULL != z) {
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info), NULL);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * Description: Attempt to compute AVIS_MIN for max( a, b).
 *
 * Generate combos of argument and extrema and see if CF can
 * simplify it.
 *
 * @params:
 *
 *
 ******************************************************************************/
static node *
MaxOnExtrema (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *zavis;
    node *arg1;
    node *arg2;
    node *marg1;
    node *marg2;
    node *mprf;

    DBUG_ENTER ();

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    marg1 = AVIS_MIN (ID_AVIS (arg1));
    marg2 = AVIS_MIN (ID_AVIS (arg2));

    if (NULL != marg2) {
        mprf = DUPdoDupTree (arg_node);
        PRF_ARG2 (mprf) = FREEdoFreeNode (PRF_ARG2 (mprf));
        PRF_ARG2 (mprf) = DUPdoDupNode (marg2);
        z = SCSprf_max_SxS (mprf, arg_info);
        mprf = FREEdoFreeTree (mprf);
    }

    if (NULL == z) {
        if (NULL != marg1) {
            mprf = DUPdoDupTree (arg_node);
            PRF_ARG1 (mprf) = FREEdoFreeNode (PRF_ARG1 (mprf));
            PRF_ARG1 (mprf) = DUPdoDupNode (marg1);
            z = SCSprf_max_SxS (mprf, arg_info);
            mprf = FREEdoFreeTree (mprf);
        }
    }

    // INFO_MINVAL wants N_avis, not N_id
    if ((NULL != z) && (N_id == NODE_TYPE (z))) {
        zavis = ID_AVIS (z);
        z = FREEdoFreeNode (z);
        z = zavis;
    }

    // Flatten any result
    if (NULL != z) {
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info), NULL);
    }

    DBUG_RETURN (z);
}
/** <!--********************************************************************-->
 *
 * Description:
 *
 fixme
 * Case 1: min( constant, nonconstant-without-maxval):
 *         the constant becomes the maxval.
 *
 * Case 2: min( constant, nonconstant-with-maxval):
 *         We conservatively estimate the non-constant extremum as the maxval.
 *
 * Case 3:
 *            min( non-constant, non-constant with maxval)
 *          or
 *            min( non-constant with maxval, non-constant)
 *
 *          maxval becomes result maxval.
 *
 * @params lhsavis, rhs, arg_info - as expected.
 *
 * @return No explicit results. However, new N_assigns and N_vardecs
 *         may be generated by flattening, and INFO_MINVAL/MAXVAL
 *         may be set.
 *
 ******************************************************************************/
static void
GenerateExtremaForMin (node *lhsavis, node *rhs, info *arg_info)
{
    node *arg1avis;
    node *arg2avis;
    bool c1;
    bool c2;
    bool e1;
    bool e2;
    bool z = FALSE;

    DBUG_ENTER ();

    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));

    if ((!IVEXPisAvisHasMax (lhsavis))
        && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
        && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))) {

        c1 = COisConstant (PRF_ARG1 (rhs)); // Could use TYisAKV here,)
        c2 = COisConstant (PRF_ARG2 (rhs)); // if flatness guaranteed.
        e1 = NULL != AVIS_MAX (arg1avis);
        e2 = NULL != AVIS_MAX (arg2avis);

        // Case 5: constant vs extrema present
        INFO_MAXVAL (arg_info) = MinOnExtrema (rhs, arg_info);

        // Case 5: constant vs extrema present
        if ((c1 && e2) && (NULL == INFO_MAXVAL (arg_info))
            && (SCScanOptMINOnDyadicFn (PRF_ARG1 (rhs), AVIS_MAX (arg2avis), &z))) {
            INFO_MAXVAL (arg_info) = z ? arg1avis : ID_AVIS (AVIS_MAX (arg2avis));
        }

        if ((c2 && e1) && (NULL == INFO_MAXVAL (arg_info))
            && (SCScanOptMINOnDyadicFn (PRF_ARG2 (rhs), AVIS_MAX (arg1avis), &z))) {
            INFO_MAXVAL (arg_info) = z ? arg2avis : ID_AVIS (AVIS_MAX (arg1avis));
        }

        // Case 1: max( constant, nonconstant-without-maxval),
        //    or   max( nonconstant-without-maxval, constant):
        //    the constant becomes the result maxval.
        if (c1 && (!c2) && (!e2) && (NULL == INFO_MAXVAL (arg_info))) {
            INFO_MAXVAL (arg_info) = ScalarExtend (arg1avis, PRF_ARG2 (rhs), arg_info);
        }
        // Case 2:
        if (c2 && (!c1) && (!e1) && (NULL == INFO_MAXVAL (arg_info))) {
            INFO_MAXVAL (arg_info) = ScalarExtend (arg2avis, PRF_ARG1 (rhs), arg_info);
        }

        // Case 3:
        if ((!c1) && (!c2) && (e2) && (NULL == INFO_MAXVAL (arg_info))) {
            INFO_MAXVAL (arg_info)
              = ScalarExtend (ID_AVIS (AVIS_MAX (arg2avis)), PRF_ARG1 (rhs), arg_info);
        }

        // Case 4:
        if ((!c2) && (!c1) && (e1) && (NULL == INFO_MAXVAL (arg_info))) {
            INFO_MAXVAL (arg_info)
              = ScalarExtend (ID_AVIS (AVIS_MAX (arg1avis)), PRF_ARG2 (rhs), arg_info);
        }

        if (NULL != INFO_MAXVAL (arg_info)) {
            // Normalize AVIS_MAX
            INFO_MAXVAL (arg_info)
              = IVEXPadjustExtremaBound (INFO_MAXVAL (arg_info), 1,
                                         &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), "gefmin");
        }
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * Description:
 *
 * Case 1: max( constant, nonconstant-without-minval):
 *         the constant becomes the minval.
 *
 * Case 2: max( constant, nonconstant-with-minval):
 *         We conservatively estimate the extremum as the minval.
 *
 * Case 3:   max( non-constant, non-constant with minval)
 *        or
 *           max( non-constant with minval, non-constant)
 *        minval becomes result minval.
 *
 * @params lhsavis, rhs, arg_info - as expected.
 *
 * @return No explicit results. However, new N_assigns and N_vardecs
 *         may be generated by flattening, and INFO_MINVAL/MAXVAL
 *         may be set.
 *
 ******************************************************************************/
static void
GenerateExtremaForMax (node *lhsavis, node *rhs, info *arg_info)
{
    node *arg1avis;
    node *arg2avis;
    bool c1;
    bool c2;
    bool e1;
    bool e2;
    bool z = FALSE;

    DBUG_ENTER ();

    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
    arg2avis = ID_AVIS (PRF_ARG2 (rhs));

    if ((!IVEXPisAvisHasMin (lhsavis))
        && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
        && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))) {

        c1 = COisConstant (PRF_ARG1 (rhs)); // Could use TYisAKV here,
        c2 = COisConstant (PRF_ARG2 (rhs)); // if flatness guaranteed.
        e1 = NULL != AVIS_MIN (arg1avis);
        e2 = NULL != AVIS_MIN (arg2avis);

        // Case 5: constant vs extrema present
        INFO_MINVAL (arg_info) = MaxOnExtrema (rhs, arg_info);

        if ((c2 && e1) && (NULL == INFO_MINVAL (arg_info))
            && (SCScanOptMAXOnDyadicFn (PRF_ARG2 (rhs), AVIS_MIN (arg1avis), &z))) {
            INFO_MINVAL (arg_info) = z ? arg2avis : ID_AVIS (AVIS_MIN (arg1avis));
        }

        // Case 1:
        if (c1 && (!c2) && (!e2) && (NULL == INFO_MINVAL (arg_info))) {
            INFO_MINVAL (arg_info) = ScalarExtend (arg1avis, PRF_ARG2 (rhs), arg_info);
        }

        // Case 2:
        if (c2 && (!c1) && (!e1) && (NULL == INFO_MINVAL (arg_info))) {
            INFO_MINVAL (arg_info) = ScalarExtend (arg2avis, PRF_ARG1 (rhs), arg_info);
        }

        // Case 3:
        if ((!c1) && (!c2) && (e2) && (NULL == INFO_MINVAL (arg_info))) {
            INFO_MINVAL (arg_info)
              = ScalarExtend (ID_AVIS (AVIS_MIN (arg2avis)), PRF_ARG1 (rhs), arg_info);
        }

        // Case 4:
        if ((!c2) && (!c1) && (e1) && (NULL == INFO_MINVAL (arg_info))) {
            INFO_MINVAL (arg_info)
              = ScalarExtend (ID_AVIS (AVIS_MIN (arg1avis)), PRF_ARG2 (rhs), arg_info);
        }
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * static
 * node *GenerateExtremaComputationsPrf(node *arg_node, info *arg_info)
 *
 * description:
 *      For some primitives, where CF and friends are able to perform
 *      expression simplification, we insert code to (attempt to)
 *      compute the extrema for this function application.
 *
 *      If this insertion is already done, this function is an identity.
 *      Ditto if no insertion is desired or possible.
 *
 *      This code operates on AKS/AKV results only.
 *
 * Arguments:
 *      arg_node: N_let for the N_prf.
 *
 * Results:
 *      The updated arg_node, with iv' -> iv''.
 *
 *      INFO_VARDECS are appended to, with entries for:
 *         minv, maxv, and iv''.
 *
 *      INFO_PREASSIGNS are appended to, with the assigns for
 *        minv, maxv, and iv''.
 *
 *      INFO_MINVAL/INFO_MAXVAL may contain "hot" N_id nodes that
 *      will have to be stored in AVIS_MIN/AVIS_MAX.
 *
 * Restrictions:
 *      A subset of N_prfs are supported, namely, those for which
 *      we can meaningfully compute their extrema.
 *
 *      There are four cases here for dyadic scalar functions:
 *
 *      1,2. One argument constant; the other non-constant:
 *           we compute new extrema.
 *
 *      3.   Neither argument constant: subcases:
 *
 *             4a. Neither argument has extrema:
 *                 we do nothing.
 *
 *             4b. One argument has extrema:
 *                 we compute new extrema.
 *
 *             4c. Both arguments have extrema, as when we
 *                 have IV + JV, and both arguments are WITH_IDs.
 *                 We blindly pick one side and compute extrema.
 *
 *                 FIXME: We could get a bit fancier here. For example,
 *                 max and min could be handled easily and correctly.
 *                 For functions such as add, sub, it gets messy
 *                 and the results may diverge. Perhaps we could
 *                 do worst-case computation here, and win sometimes.
 *
 *      4.   TC handles both arguments constant.
 *
 * Example 1:
 *
 *      The N_prf is:
 *          offset = 42;
 *          lhs = _add_SxS_( offset, iv);
 *
 *      We know that iv' has a minval extremum attached, so we transform this
 *      into:
 *
 *          offset = 42;
 *          minv = _add_SxV__( offset, AVIS_MIN( iv));
 *          lhs  = _add_SxV_( offset, iv);
 *
 *      Then, we attach the extremum to lhs:
 *
 *          AVIS_MIN( ID_AVIS( lhs)) = minv;
 *
 *
 *       If we also have a maxval extremum attached, we get this:
 *
 *          offset = 42;
 *          maxv  = _add_SxV__( offset, AVIS_MAX( iv));
 *          lhs   = _add_SxV_( offset, iv);
 *
 *          AVIS_MAX( ID_AVIS( lhs)) = maxv;
 *
 * Example 2:
 *
 *      Subtraction is a little tricky when extrema are PRF_ARG2,
 *      because that we have to swap minv and maxv. The N_prf is:
 *
 *          offset = 42;
 *          lhs = _sub_SxV_( offset, iv);
 *
 *      We know that iv' has extrema attached, so we transform this
 *      into:
 *
 *          offset = 42;
 *          one = 1;
 *          maxv = _sub_SxV_( offset, AVIS_MIN( iv));
 *          maxv = _add_VxS_( maxv, one);
 *          minv = _sub_VxS_( AVIS_MAX( iv), one);
 *          minv = _sub_SxV_( offset, minv);
 *          lhs = _sub_SxV_( offset, iv);
 *
 * Example 3:
 *
 *      Subtraction with extrema as PRF_ARG1 is same as Example 1.
 *      The N_prf is:
 *
 *          offset = 42;
 *          lhs = _sub_VxS_( iv, offset);
 *
 *      We know that iv' has extrema attached, so we transform this
 *      into:
 *
 *          offset = 42;
 *          minv = _sub_VxS__( AVIS_MIN( iv), offset);
 *          maxv = _sub_VxS__( AVIS_MAX( iv), offset);
 *          lhs = _sub_VxS_( iv, offset);
 *
 *      Then, we attach the extrema:
 *
 *          AVIS_MIN( ID_AVIS( lhs)) = minv;
 *          AVIS_MAX( ID_AVIS( lhs)) = maxv;
 *
 ******************************************************************************/
static node *
GenerateExtremaComputationsPrf (node *arg_node, info *arg_info)
{
    node *rhs;
    node *minv = NULL;
    node *maxv = NULL;
    node *lhsavis = NULL;
    node *lhsavis2 = NULL;
    node *arg1avis = NULL;
    node *arg2avis = NULL;
    node *zr;
    node *withid;
    prf nprf;
    simpletype arg2type;

    DBUG_ENTER ();

    INFO_MINVAL (arg_info) = NULL;
    INFO_MAXVAL (arg_info) = NULL;
    withid = INFO_CURWITH (arg_info);
    withid = (NULL != withid) ? PART_WITHID (WITH_PART (withid)) : NULL;
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    rhs = LET_EXPR (arg_node);
    if ((!IVEXPisAvisHasBothExtrema (lhsavis)) && (TYisAKS (AVIS_TYPE (lhsavis)))
        && (TUisIntScalar (AVIS_TYPE (lhsavis)) || TUisIntVect (AVIS_TYPE (lhsavis)))
        && (!TYisAKV (AVIS_TYPE (lhsavis)))) {

        /* This switch controls the decision to build extrema code
         * and builds the minv and/or maxv expressions
         */
        switch
            PRF_PRF (rhs)
            { /* sheep vs. goats - ignore oddball N_prfs.*/

            case F_accu:
                /* Try to fix maxoptcyc looping in prdreverseAKD.sac */
                AVIS_ISMINHANDLED (lhsavis) = TRUE;
                AVIS_ISMAXHANDLED (lhsavis) = TRUE;
                break;

            case F_non_neg_val_V:
            case F_non_neg_val_S:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                if ((!IVEXPisAvisHasMin (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
                    && (TYisAKV (AVIS_TYPE (arg1avis))
                        || TYisAKS (AVIS_TYPE (arg1avis)))) {
                    /* Create zero minimum */
                    zr = SCSmakeZero (PRF_ARG1 (rhs));
                    minv = FLATGexpression2Avis (zr, &INFO_VARDECS (arg_info),
                                                 &INFO_PREASSIGNS (arg_info),
                                                 TYeliminateAKV (AVIS_TYPE (lhsavis)));
                    AVIS_ISMINHANDLED (minv) = TRUE;
                    INFO_MINVAL (arg_info) = minv;
                }
                break;

            case F_same_shape_AxA:
                // Propagate extrema on both arguments.
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                lhsavis2 = IDS_AVIS (IDS_NEXT (LET_IDS (arg_node)));
                minv = AVIS_MIN (arg1avis);
                if (NULL != minv) {
                    IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (minv));
                }
                minv = AVIS_MIN (arg2avis);
                if (NULL != minv) {
                    IVEXPsetMinvalIfNotNull (lhsavis2, ID_AVIS (minv));
                }
                maxv = AVIS_MAX (arg1avis);
                if (NULL != maxv) {
                    IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (maxv));
                }
                maxv = AVIS_MAX (arg2avis);
                if (NULL != maxv) {
                    IVEXPsetMaxvalIfNotNull (lhsavis2, ID_AVIS (maxv));
                }

                break;

            case F_val_le_val_SxS:
            case F_val_le_val_VxV:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                if ((!IVEXPisAvisHasMax (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
                    && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))
                    && (TYisAKV (AVIS_TYPE (arg1avis))
                        || TYisAKS (AVIS_TYPE (arg1avis)))) {
                    /* Create maximum as normalize( PRF_ARG2) */
                    maxv = IVEXPadjustExtremaBound (ID_AVIS (PRF_ARG2 (rhs)), +1,
                                                    &INFO_VARDECS (arg_info),
                                                    &INFO_PREASSIGNS (arg_info), "valle");
                    INFO_MAXVAL (arg_info) = maxv;
                }
                break;

            case F_val_lt_val_SxS:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                if ((!IVEXPisAvisHasMax (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
                    && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))
                    && (TYisAKV (AVIS_TYPE (arg1avis))
                        || TYisAKS (AVIS_TYPE (arg1avis)))) {
                    /* Create max as PRF_ARG2. No need to normalize because it's lt() */
                    maxv = ID_AVIS (PRF_ARG2 (rhs));
                    INFO_MAXVAL (arg_info) = maxv;
                }
                break;

            case F_abs_S: /* AVIS_MIN is now zero */
            case F_abs_V: /* AVIS_MIN is now zero */
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                if ((!IVEXPisAvisHasMin (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))) {
                    minv = SCSmakeZero (PRF_ARG1 (rhs));
                    minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                                 &INFO_PREASSIGNS (arg_info),
                                                 TYeliminateAKV (AVIS_TYPE (lhsavis)));
                    INFO_MINVAL (arg_info) = minv;
                }
                break;

            case F_min_SxV:
            case F_min_VxS:
            case F_min_SxS:
            case F_min_VxV:
                GenerateExtremaForMin (lhsavis, rhs, arg_info);
                break;

            case F_max_SxV:
            case F_max_VxS:
            case F_max_SxS:
            case F_max_VxV:
                GenerateExtremaForMax (lhsavis, rhs, arg_info);
                break;

            case F_neg_S:
            case F_neg_V:
                /* Min becomes max and vice versa */
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                if ((!IVEXPisAvisHasMax (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
                    && (IVEXPisAvisHasMin (arg1avis))) {
                    minv = InvokeMonadicFn (ID_AVIS (AVIS_MIN (arg1avis)), lhsavis, rhs,
                                            arg_info);
                    minv = IVEXPadjustExtremaBound (minv, +1, &INFO_VARDECS (arg_info),
                                                    &INFO_PREASSIGNS (arg_info), "dsf8");
                    INFO_MAXVAL (arg_info) = minv;
                }

                if ((!IVEXPisAvisHasMin (lhsavis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))
                    && (IVEXPisAvisHasMax (arg1avis))) {
                    /* Instead of generating (-(AVIS_MAX(arg1avis)-1)),
                     * we generate           (1-AVIS_MAX(arg1avis)).
                     */
                    maxv = IVEXImakeIntScalar (1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info));
                    nprf = (F_neg_S == PRF_PRF (rhs)) ? F_sub_SxS : F_sub_SxV;
                    maxv = TCmakePrf2 (nprf, TBmakeId (maxv),
                                       TBmakeId (ID_AVIS (AVIS_MAX (arg1avis))));
                    maxv = FLATGexpression2Avis (maxv, &INFO_VARDECS (arg_info),
                                                 &INFO_PREASSIGNS (arg_info),
                                                 TYeliminateAKV (AVIS_TYPE (lhsavis)));
                    AVIS_ISMINHANDLED (maxv) = TRUE;
                    INFO_MINVAL (arg_info) = maxv;
                }
                break;

            /* Non-commutative dyadic functions */
            case F_sub_SxS:
            case F_sub_SxV:
            case F_sub_VxS:
            case F_sub_VxV:
                arg_node = GenerateExtremaComputationsSubtract (arg_node, arg_info);
                break;

            /* Commutative dyadic functions */
            case F_add_SxS:
            case F_add_SxV:
            case F_add_VxS:
            case F_add_VxV:
                arg_node
                  = GenerateExtremaComputationsCommutativeDyadicScalarPrf (arg_node,
                                                                           arg_info);
                break;

            /* Multiply is commutative dyadic function, but negatives need help */
            case F_mul_SxS:
            case F_mul_SxV:
            case F_mul_VxS:
            case F_mul_VxV:
                arg_node = GenerateExtremaComputationsMultiply (arg_node, arg_info);
                break;

            /* Modulus */
            case F_mod_SxS:
            case F_mod_SxV:
            case F_mod_VxS:
            case F_mod_VxV:
                arg_node = GenerateExtremaModulus (arg_node, arg_info, FALSE);
                break;

            /* APL Modulus */
            case F_aplmod_SxS:
            case F_aplmod_SxV:
            case F_aplmod_VxS:
            case F_aplmod_VxV:
                arg_node = GenerateExtremaModulus (arg_node, arg_info, TRUE);
                break;

            /* prod_matches_prod_shape and similar prfs merely propagate
             * PRF_ARG1 extrema.
             */
            case F_prod_matches_prod_shape_VxA:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                if ((!IVEXPisAvisHasMin (lhsavis)) && (IVEXPisAvisHasMin (arg1avis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))) {
                    INFO_MINVAL (arg_info)
                      = ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG1 (rhs))));
                }

                if ((!IVEXPisAvisHasMax (lhsavis)) && (IVEXPisAvisHasMax (arg1avis))
                    && (SWLDisDefinedInThisBlock (arg1avis, INFO_DEFDEPTH (arg_info)))) {
                    INFO_MAXVAL (arg_info)
                      = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs))));
                }
                break;

            /* Selection: _sel_VxA_( constant, argwithextrema)
             *             _idxsel_( constant, argwithextrema)
             * Generate similar selection on the extrema
             */
            case F_idx_sel:
            case F_sel_VxA:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                if ((!IVEXPisAvisHasMin (lhsavis)) && (IVEXPisAvisHasMin (arg2avis))
                    && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))
                    && (COisConstant (PRF_ARG1 (rhs)))) {
                    /* select from AVIS_MIN */
                    minv = DUPdoDupNode (rhs);
                    FREEdoFreeNode (PRF_ARG2 (minv));
                    PRF_ARG2 (minv) = DUPdoDupNode (AVIS_MIN (arg2avis));
                    arg2type = TUgetBaseSimpleType (AVIS_TYPE (arg2avis));
                    minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                                 &INFO_PREASSIGNS (arg_info),
                                                 TYmakeAKS (TYmakeSimpleType (arg2type),
                                                            SHmakeShape (0)));
                    AVIS_ISMINHANDLED (minv) = TRUE;
                    INFO_MINVAL (arg_info) = minv;
                }

                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))
                    && (IVEXPisAvisHasMax (arg2avis))
                    && (SWLDisDefinedInThisBlock (arg2avis, INFO_DEFDEPTH (arg_info)))
                    && (COisConstant (PRF_ARG1 (rhs)))) {
                    /* select from AVIS_MAX */
                    maxv = DUPdoDupNode (rhs);
                    FREEdoFreeNode (PRF_ARG2 (maxv));
                    PRF_ARG2 (maxv) = DUPdoDupNode (AVIS_MAX (arg2avis));
                    arg2type = TUgetBaseSimpleType (AVIS_TYPE (arg2avis));
                    maxv = FLATGexpression2Avis (maxv, &INFO_VARDECS (arg_info),
                                                 &INFO_PREASSIGNS (arg_info),
                                                 TYmakeAKS (TYmakeSimpleType (arg2type),
                                                            SHmakeShape (0)));
                    AVIS_ISMAXHANDLED (maxv) = TRUE;
                    INFO_MAXVAL (arg_info) = maxv;
                }
                break;

            default:
                break;

            } /* end of switch */
    }

    IVEXPsetMinvalIfNotNull (lhsavis, INFO_MINVAL (arg_info));
    IVEXPsetMaxvalIfNotNull (lhsavis, INFO_MAXVAL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateExtremaComputations( node *arg_node, info *arg_info)
 *
 * description:
 *   Introduce any extrema computations required by the arg_node N_let.
 *
 * Notes: Constants do not get extrema.
 *
 * result: The updated arg_node, with side effects from
 *         VARDECS and POSTASSIGNS.
 *
 ******************************************************************************/
static node *
GenerateExtremaComputations (node *arg_node, info *arg_info)
{
    node *rhs;
    node *lhsavis;

    DBUG_ENTER ();

    DBUG_ASSERT (N_let == NODE_TYPE (arg_node), "Expected N_let");
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    rhs = LET_EXPR (arg_node);
    /* If extrema already exist,
     * or if we have already generated extrema for this node,
     * or if LHS is constant,
     * or if LHS is not integer scalar/vector,
     * then do not attempt to add extrema computations.
     *
     */

    switch (NODE_TYPE (rhs)) {

    case N_prf:
#ifdef VERBOSE
        DBUG_PRINT ("Looking at N_prf %s", AVIS_NAME (lhsavis));
#endif // VERBOSE
        arg_node = GenerateExtremaComputationsPrf (arg_node, arg_info);
        break;

    case N_array:
#ifdef VERBOSE
        DBUG_PRINT ("Looking at N_array %s", AVIS_NAME (lhsavis));
#endif // VERBOSE
        if ((!CFisFullyConstantNode (rhs)) && (TUisIntVect (AVIS_TYPE (lhsavis)))) {
            rhs = IVEXPgenerateNarrayExtrema (arg_node, &INFO_VARDECS (arg_info),
                                              &INFO_PREASSIGNS (arg_info));
            LET_EXPR (arg_node) = rhs;
        }
        break;

    case N_ap:
    case N_id:
    case N_funcond:
    case N_bool:
    case N_num:
    case N_float:
    case N_double:
    case N_numbyte:
    case N_numulong:
    case N_char:
    case N_numlonglong:
    case N_numulonglong:
    case N_numushort:
    case N_numuint:
        /* FIXME: Add other simple scalar types */
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *PropagatePrfExtrema( node *arg_node, info *arg_info)
 *
 * @brief Propagate any extrema from RHS (etc.) to LHS for N_prf nodes.
 *
 * @param arg_node N_let node.
 *
 * @returns The updated arg_node.
 *
 ******************************************************************************/
static node *
PropagatePrfExtrema (node *arg_node, info *arg_info)
{
    node *lhsid;
    node *lhsavis;
    node *rhs;
    node *rhsavis;
    node *withid;
    node *minv;
    node *maxv;
    node *zer;
    pattern *patlhs;
    pattern *patrhs;
    pattern *pat;
    constant *lhsmin = NULL;
    constant *rhsmin = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;

    DBUG_ENTER ();

    lhsid = LET_IDS (arg_node);
    lhsavis = IDS_AVIS (lhsid);
    rhs = LET_EXPR (arg_node);
    withid = INFO_CURWITH (arg_info);
    withid = (withid != NULL) ? PART_WITHID (WITH_PART (withid)) : NULL;

    switch (PRF_PRF (rhs)) {

    case F_saabind:
        rhsavis = ID_AVIS (PRF_ARG3 (rhs));
        if (NULL != AVIS_MIN (rhsavis)) {
            IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
        }
        if (NULL != AVIS_MAX (rhsavis)) {
            IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (AVIS_MAX (rhsavis)));
        }
        break;

    /**
     * Case 1: If AVIS_MIN (lhsavis) is negative, replace it by zero, as this
     * tightens the constraint.
     *
     * Case 2: If AVIS_MIN (lhsavis) is not NULL, and AVIS_MIN (rhsavis) are
     * both are constant, propagate the larger one.
     * This can arise if GenerateExtremaComputationsPrf has created a zero
     * AVIS_MIN (lhsavis), and we have a non-NULL AVIS_MIN (rhsavis).
     * If we are unable to compute the larger of the two, we must propagate the
     * rhsavis. E.g., rhsavis is a WITHIDS_IDS with non-zero GENERATOR_BOUND1.
     *
     * Case 3: If AVIS_MIN (lhsavis) is NULL, propagate AVIS_MIN (rhsavis).
     */
    case F_non_neg_val_S:
    case F_non_neg_val_V:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));

        if ((NULL != AVIS_MIN (lhsavis)) && (SCSisConstantNonZero (AVIS_MIN (lhsavis)))
            && (SCSisNonPositive (AVIS_MIN (lhsavis)))) {
            // Case 1
            AVIS_MIN (lhsavis) = FREEdoFreeNode (AVIS_MIN (lhsavis));
            zer = SCSmakeZero (PRF_ARG1 (rhs));
            zer = FLATGexpression2Avis (zer, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNS (arg_info),
                                        TYeliminateAKV (AVIS_TYPE (lhsavis)));
            IVEXPsetMinvalIfNotNull (lhsavis, zer);
        } else {
            // Case 2 Propagate larger constraint
            patlhs = PMconst (1, PMAgetVal (&lhsmin), 0);
            patrhs = PMconst (1, PMAgetVal (&rhsmin), 0);
            if ((NULL != AVIS_MIN (lhsavis)) && (NULL != AVIS_MIN (rhsavis))
                && (PMmatchFlat (patlhs, AVIS_MIN (lhsavis)))
                && (PMmatchFlat (patrhs, AVIS_MIN (rhsavis)))) {
                if (COgt (rhsmin, lhsmin, NULL)) {
                    AVIS_MIN (lhsavis) = FREEdoFreeNode (AVIS_MIN (lhsavis));
                    IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
                }
            }
            lhsmin = (NULL != lhsmin) ? COfreeConstant (lhsmin) : NULL;
            rhsmin = (NULL != rhsmin) ? COfreeConstant (rhsmin) : NULL;
            patlhs = PMfree (patlhs);
            patrhs = PMfree (patrhs);
        }

        // Case 3
        if (NULL != AVIS_MIN (rhsavis)) {
            IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
        }
        if (NULL != AVIS_MAX (rhsavis)) {
            IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (AVIS_MAX (rhsavis)));
        }
        break;

    /**
     * x1', .., xn' = guard (x1, .., xn, p1, .., pm)
     * - propagate extrema of xi to xi'
     * - ignore pi
     */
    case F_guard:
        // Propagate extrema of xi to xi'
        while (lhsid != NULL) {
            rhsavis = ID_AVIS (EXPRS_EXPR (rhs));
            lhsavis = IDS_AVIS (lhsid);

            if (AVIS_MIN (rhsavis) != NULL) {
                IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
            }
            if (AVIS_MAX (rhsavis) != NULL) {
                IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (AVIS_MAX (rhsavis)));
            }
            if (TYisAKV (AVIS_TYPE (rhsavis))) {
                IVEXPsetMinvalIfNotNull (lhsavis, rhsavis);
                if (AVIS_MAX (lhsavis) == NULL) {
                    maxv = IVEXPadjustExtremaBound (rhsavis, 1,
                                                    &INFO_VARDECS (arg_info),
                                                    &INFO_PREASSIGNS (arg_info),
                                                    "dsf9");
                    IVEXPsetMaxvalIfNotNull (lhsavis, maxv);
                }
            }

            rhs = EXPRS_NEXT (rhs);
            lhsid = IDS_NEXT (lhsid);
        }
        break;

    case F_noteintersect:
    case F_shape_matches_dim_VxA:
    case F_val_lt_shape_VxA:
    case F_val_le_val_SxS:
    case F_val_le_val_VxV:
    case F_val_lt_val_SxS:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        if (AVIS_MIN (rhsavis) != NULL) {
            IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
        }
        if (AVIS_MAX (rhsavis) != NULL) {
            IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (AVIS_MAX (rhsavis)));
        }
        if (TYisAKV (AVIS_TYPE (rhsavis))) {
            IVEXPsetMinvalIfNotNull (lhsavis, rhsavis);
            if (AVIS_MAX (lhsavis) == NULL) {
                maxv = IVEXPadjustExtremaBound (rhsavis, 1,
                                                &INFO_VARDECS (arg_info),
                                                &INFO_PREASSIGNS (arg_info),
                                                "dsf9");
                IVEXPsetMaxvalIfNotNull (lhsavis, maxv);
            }
        }
        break;

    case F_noteminval:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (PRF_ARG2 (rhs)));
        if (NULL != AVIS_MAX (rhsavis)) {
            IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (AVIS_MAX (rhsavis)));
        }
        break;

    case F_notemaxval:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (PRF_ARG2 (rhs)));
        if (NULL != AVIS_MIN (rhsavis)) {
            IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (AVIS_MIN (rhsavis)));
        }
        break;

    case F_min_SxS:
    case F_min_VxV:
        /* If either argument has a maxval, propagate it, but
         * do not override an extant value  */
        if (NULL == AVIS_MAX (lhsavis)) {

            maxv = AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs)));
            if (NULL != maxv) {
                IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (maxv));
            }

            maxv = AVIS_MAX (ID_AVIS (PRF_ARG2 (rhs)));
            if (NULL != maxv) {
                IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (maxv));
            }
        }
        break;

    case F_max_SxS:
    case F_max_VxV:
        /* If either argument has a minval, propagate it, but
         * do not override an extant value  */
        if (NULL == AVIS_MIN (lhsavis)) {

            minv = AVIS_MIN (ID_AVIS (PRF_ARG1 (rhs)));
            if (NULL != minv) {
                IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (minv));
            }

            minv = AVIS_MIN (ID_AVIS (PRF_ARG2 (rhs)));
            if (NULL != minv) {
                IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (minv));
            }
        }
        break;

    case F_sub_SxS:
        /*
         *  [I would like to find a better place for this sort of thing,
         *   but let's see how this works.]
         *
         *           AWLF intersect and null-intersect calculation involve
         *           this calculation (at least):
         *
         *            tmp = N - min( N, k);
         *            p = tmp < 0;
         *
         *           With Case 1 or Case 2, we end up with AVIS_MIN( tmp) = N - k;
         *           this does not allow AWLF to proceed.
         *
         *           However, observe that:
         *
         *            k <= N --> tmp = ( N - k), but since k is <= than N,
         *                       tmp is >0.
         *            k >  N --> tmp = ( N - N), so
         *                       tmp = 0;
         *
         *            Hence, setting AVIS_MIN( tmp) = 0 is safe;
         *
         *
         * I think we can get away with doing this one on scalars only.
         * I don't know if we need similar code for max().
         */
        if (!IVEXPisAvisHasMin (lhsavis)) {
            pat = PMprf (1, PMAisPrf (F_min_SxS), 2, PMvar (1, PMAgetNode (&arg1), 0),
                         PMvar (1, PMAgetNode (&arg2), 0));
            if ((PMmatchFlat (pat, PRF_ARG2 (rhs)))
                && (TULSisValuesMatch (PRF_ARG1 (rhs), arg1)
                    || TULSisValuesMatch (PRF_ARG1 (rhs), arg2))) {
                minv = SCSmakeZero (PRF_ARG1 (rhs)); /* Create zero minimum */
                minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGNS (arg_info),
                                             TYeliminateAKV (AVIS_TYPE (lhsavis)));
                IVEXPsetMinvalIfNotNull (lhsavis, minv);
            }

            pat = PMfree (pat);
        }
        break;

#ifdef UNDERCONSTRUCTION
    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        /* If either argument is a non-negative even constant, AVIS_MIN
         * is zero
         */
        if (!IVEXPisAvisHasMin (lhsavis)) {
            minv stuff;
            minv = SCSmakeZero (PRF_ARG1 (rhs)); /* Create zero minimum */
            fixSv minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info),
                                               TYeliminateAKV (AVIS_TYPE (lhsavis)));
            IVEXPsetMinvalIfNotNull (lhsavis, minv);
        }
        break;
#endif /* UNDERCONSTRUCTION */

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PropagateExtrema( node *arg_node, info *arg_info)
 *
 * description:
 *   Propagate any extrema from RHS (etc.) to LHS.
 *
 * result: The updated arg_node.
 *
 ******************************************************************************/
static node *
PropagateExtrema (node *arg_node, info *arg_info)
{
    node *rhs;
    node *lhsavis;
    node *rhsavis;
    node *minv;
    node *maxv;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    switch (NODE_TYPE (rhs)) {
    case N_id:
        rhsavis = ID_AVIS (rhs);

        minv = AVIS_MIN (rhsavis);
        if (NULL != minv) {
            IVEXPsetMinvalIfNotNull (lhsavis, ID_AVIS (minv));
        }

        maxv = AVIS_MAX (rhsavis);
        if (NULL != maxv) {
            IVEXPsetMaxvalIfNotNull (lhsavis, ID_AVIS (maxv));
        }
        break;

    case N_prf:
        arg_node = PropagatePrfExtrema (arg_node, arg_info);
        break;

    case N_array:
        /* handled by GenerateNarrayExtrema */

        break;

    case N_with:
    case N_ap:
    case N_funcond:
    case N_bool:
    case N_num:
    case N_float:
    case N_double:
    case N_numbyte:
    case N_numulong:
    case N_char:
    case N_numlonglong:
    case N_numulonglong:
    case N_numushort:
    case N_numuint:
        /* FIXME: Add other simple scalar types */
        break;

    default:
        DBUG_PRINT ("IVEXP ISMOP: please fix this RHS for LHS: %s", AVIS_NAME (lhsavis));
        break;
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
 *   node *IVEXPassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Place preassigns in their proper locations.
 *   This is slightly tricky, because we start with:
 *
 *    arg_node-> instr -> next
 *
 *   and want to end up with:
 *
 *    arg_node -> preassigns -> next
 *
 *
 *
 ******************************************************************************/
node *
IVEXPassign (node *arg_node, info *arg_info)
{
    node *mypreassigns;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* We want PREASSIGNS ++ arg_node ++ POSTASSIGNS ++ ASSIGN_NEXT */
    if (NULL != INFO_POSTASSIGNS (arg_info)) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }

    mypreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    DBUG_ASSERT (NULL == INFO_PREASSIGNS (arg_info), "preassign confusion");
    INFO_PREASSIGNS (arg_info) = mypreassigns;

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 *    0. If result (LHS) is AKV, do nothing.
 *
 *    1. Propagate iv extrema from RHS to LHS, but only for
 *       integer scalars and vectors.
 *
 *    2. If extrema exist for one argument, and other argument is
 *       AKV, introduce code to compute extrema for this LHS.
 *       Also, we then have to rename the LHS. This is done
 *       by the extrema insertion code.
 *
 ******************************************************************************/
node *
IVEXPlet (node *arg_node, info *arg_info)
{
    node *lhsavis;
#ifdef VERBOSE
    char *lhsname;
#endif // VERBOSE
    ntype *typ;

    DBUG_ENTER ();

    lhsavis = IDS_AVIS (LET_IDS (arg_node));

#ifdef VERBOSE
    lhsname = AVIS_NAME (lhsavis); /* Handy for ddd condition-setting */

    DBUG_PRINT ("Looking at %s", lhsname);
#endif // VERBOSE

    typ = AVIS_TYPE (lhsavis);
    if ((!TYisAKV (typ)) && (TUisIntScalar (typ) || TUisIntVect (typ))) {

        INFO_LET (arg_info) = arg_node;
        arg_node = GenerateExtremaComputations (arg_node, arg_info);
        arg_node = PropagateExtrema (arg_node, arg_info);
        INFO_LET (arg_info) = NULL;
    }

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPwith( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *  We have to save current WL pointer, because we don't want
 *  IVEXPlet running off to attempt extrema introduction outside
 *  of a WL.
 *
 ******************************************************************************/
node *
IVEXPwith (node *arg_node, info *arg_info)
{
    node *oldwith;
    int olddepth;

    DBUG_ENTER ();

    oldwith = INFO_CURWITH (arg_info);
    olddepth = INFO_DEFDEPTH (arg_info);
    INFO_CURWITH (arg_info) = arg_node;
    INFO_DEFDEPTH (arg_info) = INFO_DEFDEPTH (arg_info) + 1;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    INFO_CURWITH (arg_info) = oldwith;
    INFO_DEFDEPTH (arg_info) = olddepth;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPpart( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 *   We also maintain the WITHID for the partition, for use by
 *   AWLFIfindParent2.
 *
 ******************************************************************************/
node *
IVEXPpart (node *arg_node, info *arg_info)
{
    node *oldwithidids;

    DBUG_ENTER ();

    oldwithidids = INFO_WITHIDIDS (arg_info);
    INFO_WITHIDIDS (arg_info) = WITHID_IDS (PART_WITHID (arg_node));

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    INFO_WITHIDIDS (arg_info) = oldwithidids;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPcond( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPcond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPfuncond( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPwhile( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WHILE_COND (arg_node) = TRAVdo (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *  Insert new vardecs into fundef node.
 *
 ******************************************************************************/
node *
IVEXPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("IVEXP in %s %s begins",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    arg_node = SWLDdoSetWithloopDepth (arg_node);

    DBUG_ASSERT (INFO_VARDECS (arg_info) == NULL, "INFO_VARDECS not NULL");

    INFO_FUNDEF (arg_info) = arg_node;

    INFO_FUNDEF (arg_info) = NULL;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("IVEXP in %s %s ends",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
