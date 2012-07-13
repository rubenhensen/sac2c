/*
 * $Id: ivexpropagation.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexp Index Vector Extrema Propagation Traversal
 *
 * @brief:
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
        zavis = FLATGflattenExpression (narr, vardecs, preassigns, TYeliminateAKV (typ));
    }

    DBUG_RETURN (zavis);
}

/******************************************************************************
 *
 * function: void IVEXPsetMinvalIfNotNull( node *snk, node *src, bool dup);
 * function: void IVEXPsetMaxvalIfNotNull( node *snk, node *src, bool dup);
 *
 * description: Set extremum from src if it is not NULL.
 *              If the snk is not NULL, free it first.
 *
 * @params:     snk: pointer to an N_avis
 *              src: pointer to an N_id or NULL.
 *              dup: if TRUE, DUP the src.
 *
 * @result: If src is NULL, or if both N_id nodes point to
 *          the same N_avis, no change.
 *          Otherwise, the snk pointer, if non-NULL, is freed,
 *          and overwritten by the src pointer.
 *          We assume that any DUP required was performed by
 *          the caller.
 *
 ******************************************************************************/
void
IVEXPsetMinvalIfNotNull (node *snk, node *src, bool dup)
{

    DBUG_ENTER ();

    if (NULL != src) {
        DBUG_ASSERT (N_id == NODE_TYPE (src), "Expected N_id src");
        if (NULL == AVIS_MIN (snk)) {
            AVIS_MIN (snk) = dup ? TBmakeId (ID_AVIS (src)) : src;
            AVIS_ISMINHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MIN(%s) set to %s", AVIS_NAME (snk),
                        AVIS_NAME (ID_AVIS (src)));
            global.optcounters.ivexp_expr++;
        } else if ((ID_AVIS (src) != ID_AVIS (AVIS_MIN (snk)))) {
            FREEdoFreeNode (AVIS_MIN (snk));
            AVIS_MIN (snk) = dup ? TBmakeId (ID_AVIS (src)) : src;
            AVIS_ISMINHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MIN(%s) set to %s", AVIS_NAME (snk),
                        AVIS_NAME (ID_AVIS (src)));
            global.optcounters.ivexp_expr++;
        }
    }

    DBUG_RETURN ();
}

void
IVEXPsetMaxvalIfNotNull (node *snk, node *src, bool dup)
{

    DBUG_ENTER ();

    if (NULL != src) {
        DBUG_ASSERT (N_id == NODE_TYPE (src), "Expected N_id src");
        if (NULL == AVIS_MAX (snk)) {
            AVIS_MAX (snk) = dup ? TBmakeId (ID_AVIS (src)) : src;
            AVIS_ISMAXHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MAX(%s) set to %s", AVIS_NAME (snk),
                        AVIS_NAME (ID_AVIS (src)));
            global.optcounters.ivexp_expr++;
        } else if ((ID_AVIS (src) != ID_AVIS (AVIS_MAX (snk)))) {
            FREEdoFreeNode (AVIS_MAX (snk));
            AVIS_MAX (snk) = dup ? TBmakeId (ID_AVIS (src)) : src;
            AVIS_ISMAXHANDLED (snk) = TRUE;
            DBUG_PRINT ("AVIS_MAX(%s) set to %s", AVIS_NAME (snk),
                        AVIS_NAME (ID_AVIS (src)));
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
 * function: node *isAllNarrayExtremumPresent( node *arg_node)
 *
 * description: Predicate for determining if an N_array node
 *              has all its AVIS_MIN/AVIS_MAX elements present, for
 *              non-constant elements.
 *
 *
 * @params  arg_node: an N_array node.
 *          minmax: 0 to check AVIS_MIN, 1 to check AVIS_MAX
 *
 * @result: A boolean, TRUE if all min/max vals are present for
 *          non-constant elements.
 *
 ******************************************************************************/
static bool
isAllNarrayExtremumPresent (node *arg_node, int minmax)
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

        if ((!AVIS_ISMINHANDLED (IDS_AVIS (lhs)))
            && (isAllNarrayExtremumPresent (rhs, 0))) {
            minv = buildExtremaChain (ARRAY_AELEMS (rhs), 0);
            minv
              = makeNarray (minv, AVIS_TYPE (IDS_AVIS (lhs)), rhs, vardecs, preassigns);
            IVEXPsetMinvalIfNotNull (lhsavis, TBmakeId (minv), FALSE);
        }

        if ((!AVIS_ISMAXHANDLED (IDS_AVIS (lhs)))
            && (isAllNarrayExtremumPresent (rhs, 1))) {
            maxv = buildExtremaChain (ARRAY_AELEMS (rhs), 1);
            maxv
              = makeNarray (maxv, AVIS_TYPE (IDS_AVIS (lhs)), rhs, vardecs, preassigns);
            IVEXPsetMaxvalIfNotNull (lhsavis, TBmakeId (maxv), FALSE);
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
    int i;
    int lim;
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

        DBUG_PRINT ("adjustExtremaBound introduced adjustment named: %s for: %s",
                    AVIS_NAME (zavis), AVIS_NAME (arg_node));
        pat = PMfree (pat);
    }

    DBUG_RETURN (zavis);
}

/******************************************************************************
 *
 * function:
 *   node *GetExtremaOnNonconstantArg( node *arg_node, info *arg_info)
 *
 * description: Function for obtaining dyadic scalar function nprf
 *              extrema on non-constant argument
 *
 * @params  arg_node: an N_prf node
 *          arg_info: your basic arg_info
 * @result: If the non-constant argument has an AVIS_MIN/AVIS_MAX,
 *          return that MIN/MAX. Else NULL.
 *
 ******************************************************************************/
static node *
GetMinvalOnNonconstantArg (node *arg_node, info *arg_info)
{
    node *z = NULL;
    bool arg1c;
    bool arg2c;

    DBUG_ENTER ();

    arg1c = COisConstant (PRF_ARG1 (arg_node));
    arg2c = COisConstant (PRF_ARG2 (arg_node));

    if ((arg1c != arg2c) && arg1c) {
        z = AVIS_MIN (ID_AVIS (PRF_ARG2 (arg_node)));
    }

    if ((arg1c != arg2c) && arg2c) {
        z = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
    }

    if (NULL != z) {
        DBUG_PRINT ("Found minval %s for non-constant argument", AVIS_NAME (ID_AVIS (z)));
    }

    DBUG_RETURN (z);
}

static node *
GetMaxvalOnNonconstantArg (node *arg_node, info *arg_info)
{
    node *z = NULL;
    bool arg1c;
    bool arg2c;

    DBUG_ENTER ();

    arg1c = COisConstant (PRF_ARG1 (arg_node));
    arg2c = COisConstant (PRF_ARG2 (arg_node));

    if ((arg1c != arg2c) && arg1c) {
        z = AVIS_MAX (ID_AVIS (PRF_ARG2 (arg_node)));
    }

    if ((arg1c != arg2c) && arg2c) {
        z = AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)));
    }

    if (NULL != z) {
        DBUG_PRINT ("Found maxval %s for non-constant argument", AVIS_NAME (ID_AVIS (z)));
    }

    DBUG_RETURN (z);
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
    minmaxavis = FLATGflattenExpression (minmaxavis, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info),
                                         TYeliminateAKV (AVIS_TYPE (lhsavis)));
    DBUG_RETURN (minmaxavis);
}

/** <!--********************************************************************-->
 *
 * Description: Generate extrema computation for commutative
 *              dyadic scalar function.
 *
 *    Ideally, we want to compute the extrema between
 *    a constant argument and the extremum of the non-constant
 *    argument. This gives us these cases:
 *
 *     f( const, nonconst), with minval on non-const:
 *  --> minval = f( const, minval)
 *     f( nonconst, const), with minval on non-const:
 *  --> minval = f( minval, const)
 *
 *      f( non_const, non_const) with minval on first arg:
 *  --> minval = f( minval, non_const)
 *      f( non_const, non_const) with minval on second arg:
 *  --> minval = f( non_const, minval)
 *
 *  We have to pre- and post-adjust AVIS_MAX, or _mul_
 *  will get wrong answer, i.e.:
 *
 *     f( const, nonconst), with maxval on non-const:
 *  --> maxval =  1 + f( const, maxval -1)
 *     f( nonconst, const), with maxval on non-const:
 *  --> maxval =  1 + f( maxval - 1, const)
 *
 *      f( non_const, non_const) with maxval on first arg:
 *  --> maxval = 1 + f( maxval - 1, non_const)
 *      f( non_const, non_const) with maxval on second arg:
 *  --> maxval = 1 + f( non_const, maxval - 1)
 *
 *  Just to make things messier, we have avoid the situation in
 *  which we use PRF_ARG1 extrema for minv, and PRF_ARG2 extrema
 *  for maxv (or vice versa). That would lead us to the final
 *  code wanting to swap minv/maxv for PRF_ARG2, but not for
 *  PRG_ARG1. We arrange things so that if we use the PRF_ARG2
 *  extremum for minv, we never use the PRF_ARG1 extremum for maxv, by
 *  refuting the fact that PRF_ARG1 has AVIS_MAX, and vice versa.
 *
 * @return argument arg_node is returned, unchanged.
 *         All changes are side effects.
 *
 ******************************************************************************/
static node *
GenerateExtremaComputationsDyadicScalarPrf (node *arg_node, info *arg_info)
{
    node *minarg1 = NULL;
    node *minarg2 = NULL;
    node *maxarg1 = NULL;
    node *maxarg2 = NULL;
    node *minv = NULL;
    node *maxv = NULL;
    node *lhsavis;
    node *rhs;
    bool arg1c;
    bool arg2c;
    bool min1;
    bool min2;
    bool max1;
    bool max2;

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));

    arg1c = COisConstant (PRF_ARG1 (rhs));
    arg2c = COisConstant (PRF_ARG2 (rhs));

    /* Slight dance to circumvent bug #693, in which DL generates
     * PRF_ARGs with N_num nodes.
     */

    min1 = IVEXPisAvisHasMin (ID_AVIS (PRF_ARG1 (rhs)));
    max1 = IVEXPisAvisHasMax (ID_AVIS (PRF_ARG1 (rhs)));
    min2 = IVEXPisAvisHasMin (ID_AVIS (PRF_ARG2 (rhs)));
    max2 = IVEXPisAvisHasMax (ID_AVIS (PRF_ARG2 (rhs)));

    /* Compute AVIS_MIN, perhaps */
    if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))) {
        switch (PRF_PRF (rhs)) {
        case F_sub_SxS:
        case F_sub_SxV:
        case F_sub_VxS:
        case F_sub_VxV:
            if (max2) {
                /*  Case 1: minv( z) = A - denormalize( maxv( B));  */
                minarg1 = ID_AVIS (PRF_ARG1 (rhs));
                minarg2 = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG2 (rhs))));
                minarg2 = IVEXPadjustExtremaBound (/* Normalize maxv */
                                                   minarg2, -1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "dsf1");
            } else if (min1) {
                /*  Case 2: minv( z) = minv( A) - B; */
                minarg1 = ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG1 (rhs))));
                minarg2 = ID_AVIS (PRF_ARG2 (rhs));
            } else if (max1) {
                /* Case 3: minv( z) = denormalize( maxv( A)) - B; */
                minarg1 = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs))));
                minarg1 = IVEXPadjustExtremaBound (/* Normalize maxv */
                                                   minarg1, -1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "dsf5");
                minarg2 = ID_AVIS (PRF_ARG2 (rhs));
            }
            break;

        default:
            if (min1) {
                minarg1 = ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG1 (rhs))));
                minarg2 = ID_AVIS (PRF_ARG2 (rhs));
            } else if (min2) {
                minarg1 = ID_AVIS (PRF_ARG1 (rhs));
                minarg2 = ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG2 (rhs))));
            }
            break;
        }
    }

    /* Compute AVIS_MAX, perhaps */
    if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))) {
        switch (PRF_PRF (rhs)) {
        case F_sub_SxS:
        case F_sub_SxV:
        case F_sub_VxS:
        case F_sub_VxV:
            if (min2) {
                /*  Case 1: maxv( z) = normalize( A - minv( B)); */
                maxarg1 = ID_AVIS (PRF_ARG1 (rhs));
                maxarg2 = ID_AVIS (AVIS_MIN (ID_AVIS (PRF_ARG2 (rhs))));
            } else if (max1) {
                /*  Case 2: maxv( z) = normalize( denormalize( maxv( A)) - B); */
                maxarg1 = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs))));
                maxarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                                   maxarg1, -1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "dsf2");
                maxarg2 = ID_AVIS (PRF_ARG2 (rhs));
            }
            break;

        default:
            if (max1) {
                maxarg1 = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs))));
                maxarg1 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                                   maxarg1, -1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "dsf3");
                maxarg2 = ID_AVIS (PRF_ARG2 (rhs));
            } else if (max2) {
                maxarg1 = ID_AVIS (PRF_ARG1 (rhs));
                maxarg2 = ID_AVIS (AVIS_MAX (ID_AVIS (PRF_ARG2 (rhs))));
                maxarg2 = IVEXPadjustExtremaBound (/* Denormalize maxv */
                                                   maxarg2, -1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "dsf4");
            }
            break;
        }
    }

    /* Generate normalized extrema calculation for dyadic primitives */
    if (NULL != minarg1) {
        minv = TCmakePrf2 (PRF_PRF (rhs), TBmakeId (minarg1), TBmakeId (minarg2));
        minv = FLATGflattenExpression (minv, &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGNS (arg_info),
                                       TYeliminateAKV (AVIS_TYPE (lhsavis)));
    }

    if (NULL != maxarg1) {
        maxv = TCmakePrf2 (PRF_PRF (rhs), TBmakeId (maxarg1), TBmakeId (maxarg2));
        maxv = FLATGflattenExpression (maxv, &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGNS (arg_info),
                                       TYeliminateAKV (AVIS_TYPE (lhsavis)));
    }

    /* Denormalize maxv */
    maxv = IVEXPadjustExtremaBound (maxv, +1, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), "dsf5");

    INFO_MINVAL (arg_info) = (NULL != minv) ? TBmakeId (minv) : minv;
    INFO_MAXVAL (arg_info) = (NULL != maxv) ? TBmakeId (maxv) : maxv;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Description: For min( constant, non-constant with constant AVIS_MIN),
 *              compute AVIS_MIN. For other cases, return NULL.
 *
 * @params prfarg: PRF_ARG1 or PRF_ARG2 from the min().
 *         nca: the non-constant argument.
 *         lhsavis: N_avis for LHS.
 *         arg_info: your basic arg_info.
 *
 * @return N_avis node for result, or NULL
 *
 ******************************************************************************/
static node *
GetMinvalForMin (node *prfarg, node *nca, node *lhsavis, info *arg_info)
{
    constant *con1;
    constant *con2;
    constant *conz;
    node *m = NULL;

    DBUG_ENTER ();

    if ((NULL != nca) && (COisConstant (prfarg))) {
        con1 = COaST2Constant (prfarg);
        con2 = COaST2Constant (nca);
        if (NULL != con2) {
            conz = COmin (con1, con2, NULL);
            m = COconstant2AST (conz);
            m = FLATGflattenExpression (m, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNS (arg_info),
                                        TYeliminateAKV (AVIS_TYPE (lhsavis)));
            AVIS_ISMINHANDLED (m) = TRUE;
            con1 = COfreeConstant (con1);
            con2 = COfreeConstant (con2);
            conz = COfreeConstant (conz);
        }
    }

    DBUG_RETURN (m);
}

/** <!--********************************************************************-->
 *
 * Description: For max( constant, non-constant with constant AVIS_MAX),
 *              compute AVIS_MAX. For other cases, return NULL.
 *
 * @params prfarg: PRF_ARG1 or PRF_ARG2 from the min().
 *         nca: the non-constant argument.
 *         lhsavis: N_avis for LHS.
 *         arg_info: your basic arg_info.
 *
 * @return N_avis node for result, or NULL
 *
 ******************************************************************************/
static node *
GetMaxvalForMax (node *prfarg, node *nca, node *lhsavis, info *arg_info)
{
    constant *con1;
    constant *con2;
    constant *conz;
    node *m = NULL;

    DBUG_ENTER ();

    if ((NULL != nca) && (COisConstant (prfarg))) {
        con1 = COaST2Constant (prfarg);
        con2 = COaST2Constant (nca);
        if (NULL != con2) {
            conz = COmax (con1, con2, NULL);
            m = COconstant2AST (conz);
            m = FLATGflattenExpression (m, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNS (arg_info),
                                        TYeliminateAKV (AVIS_TYPE (lhsavis)));
            AVIS_ISMAXHANDLED (m) = TRUE;
            con1 = COfreeConstant (con1);
            con2 = COfreeConstant (con2);
            conz = COfreeConstant (conz);
        }
    }

    DBUG_RETURN (m);
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
    node *lhsavis;
    node *arg1avis;
    node *arg2avis;
    node *nca = NULL;
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
    if ((!IVEXPisAvisHasBothExtrema (lhsavis))
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
                if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))
                    && (TYisAKV (AVIS_TYPE (arg1avis))
                        || TYisAKS (AVIS_TYPE (arg1avis)))) {
                    /* Create zero minimum */
                    zr = SCSmakeZero (PRF_ARG1 (rhs));
                    minv = FLATGflattenExpression (zr, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info),
                                                   TYeliminateAKV (AVIS_TYPE (lhsavis)));
                    AVIS_ISMINHANDLED (minv) = TRUE;
                    AVIS_ISMAXHANDLED (minv) = TRUE;
                    INFO_MINVAL (arg_info) = TBmakeId (minv);
                }
                break;

            case F_abs_S: /* AVIS_MIN is now zero */
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))) {
                    minv = IVEXImakeIntScalar (0, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info));
                    INFO_MINVAL (arg_info) = TBmakeId (minv);
                }
                break;

                /* FIXME Have to make constant scalar into vector, or vice versa.
                 * ISMOP
                case F_min_SxV:
                case F_min_VxS:
                 FIXME Have to make constant scalar into vector, or vice versa.
                */

            case F_min_SxS:
            case F_min_VxV:
                /* Case 1: min( constant, nonconstant-without-maxval):
                 *         the constant becomes the maxval.
                 *
                 * Case 2: min( constant, nonconstant-with-maxval):
                 *         We conservatively estimate the extremum as the maxval.
                 */
                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))) {
                    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                    nca = GetMaxvalOnNonconstantArg (rhs, arg_info);

                    if (NULL == nca) {
                        /* Case 1 */
                        if (COisConstant (PRF_ARG1 (rhs))) {
                            maxv = IVEXPadjustExtremaBound (ID_AVIS (PRF_ARG1 (rhs)), 1,
                                                            &INFO_VARDECS (arg_info),
                                                            &INFO_PREASSIGNS (arg_info),
                                                            "dsf6");

                            INFO_MAXVAL (arg_info) = TBmakeId (maxv);
                            AVIS_ISMAXHANDLED (maxv) = TRUE;
                        } else {
                            if (COisConstant (PRF_ARG2 (rhs))) {
                                maxv
                                  = IVEXPadjustExtremaBound (ID_AVIS (PRF_ARG2 (rhs)), 1,
                                                             &INFO_VARDECS (arg_info),
                                                             &INFO_PREASSIGNS (arg_info),
                                                             "dsf7");
                                INFO_MAXVAL (arg_info) = TBmakeId (maxv);
                                AVIS_ISMAXHANDLED (maxv) = TRUE;
                            }
                        }
                    } else {
                        /* Case 2*/
                        INFO_MAXVAL (arg_info) = DUPdoDupNode (nca);
                    }
                }

                /* min( constant, non-constant with constant minval) */
                if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))) {
                    nca = GetMinvalOnNonconstantArg (rhs, arg_info);
                    minv = GetMinvalForMin (PRF_ARG1 (rhs), nca, lhsavis, arg_info);
                    if (NULL == minv) {
                        minv = GetMinvalForMin (PRF_ARG2 (rhs), nca, lhsavis, arg_info);
                    }
                    if (NULL != minv) {
                        INFO_MINVAL (arg_info) = TBmakeId (minv);
                    }
                }
                break;

                /* FIXME Have to make constant scalar into vector, or vice versa.
                case F_max_SxV:
                case F_max_VxS:
                 FIXME Have to make constant scalar into vector, or vice versa.
                */

            case F_max_SxS:
            case F_max_VxV:
                /* Case 1: max( constant, nonconstant-without-minval):
                 *         the constant becomes the minval.
                 *
                 * Case 2: max( constant, nonconstant-with-minval):
                 *         We conservatively estimate the extremum as the minval.
                 */
                if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))) {
                    arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                    nca = GetMinvalOnNonconstantArg (rhs, arg_info);

                    /* Case 1 */
                    if (NULL == nca) {
                        if (COisConstant (PRF_ARG1 (rhs))) {
                            INFO_MINVAL (arg_info) = DUPdoDupNode (PRF_ARG1 (rhs));
                        } else {
                            if (COisConstant (PRF_ARG2 (rhs))) {
                                INFO_MINVAL (arg_info) = DUPdoDupNode (PRF_ARG2 (rhs));
                            }
                        }
                    } else {
                        /* Case 2 */
                        INFO_MINVAL (arg_info) = DUPdoDupNode (nca);
                    }
                }

                /* max( constant, non-constant with constant maxval) */
                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))) {
                    nca = GetMaxvalOnNonconstantArg (rhs, arg_info);
                    maxv = GetMaxvalForMax (PRF_ARG1 (rhs), nca, lhsavis, arg_info);
                    if (NULL == maxv) {
                        maxv = GetMaxvalForMax (PRF_ARG2 (rhs), nca, lhsavis, arg_info);
                    }
                    if (NULL != maxv) {
                        INFO_MINVAL (arg_info) = TBmakeId (maxv);
                    }
                }
                break;

            case F_neg_S:
            case F_neg_V:
                /* Min becomes max and vice versa */
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))
                    && (IVEXPisAvisHasMin (arg1avis))) {
                    minv = InvokeMonadicFn (ID_AVIS (AVIS_MIN (arg1avis)), lhsavis, rhs,
                                            arg_info);
                    minv = IVEXPadjustExtremaBound (minv, +1, &INFO_VARDECS (arg_info),
                                                    &INFO_PREASSIGNS (arg_info), "dsf8");
                    INFO_MAXVAL (arg_info) = TBmakeId (minv);
                }

                if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))
                    && (IVEXPisAvisHasMax (arg1avis))) {
                    /* Instead of generating (-(AVIS_MAX(arg1avis)-1)),
                     * we generate           (1-AVIS_MAX(arg1avis)).
                     */
                    maxv = IVEXImakeIntScalar (1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNS (arg_info));
                    nprf = (F_neg_S == PRF_PRF (rhs)) ? F_sub_SxS : F_sub_SxV;
                    maxv = TCmakePrf2 (nprf, TBmakeId (maxv),
                                       TBmakeId (ID_AVIS (AVIS_MAX (arg1avis))));
                    maxv = FLATGflattenExpression (maxv, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info),
                                                   TYeliminateAKV (AVIS_TYPE (lhsavis)));
                    AVIS_ISMINHANDLED (maxv) = TRUE;
                    INFO_MINVAL (arg_info) = TBmakeId (maxv);
                }
                break;

            /* Non-commutative dyadic functions */
            case F_sub_SxS:
            case F_sub_SxV:
            case F_sub_VxS:
            case F_sub_VxV:

            /* Commutative dyadic functions */
            case F_add_SxS:
            case F_add_SxV:
            case F_add_VxS:
            case F_add_VxV:
            case F_mul_SxS:
            case F_mul_SxV:
            case F_mul_VxS:
            case F_mul_VxV:
                arg_node
                  = GenerateExtremaComputationsDyadicScalarPrf (arg_node, arg_info);
                break;

            /* Selection: _sel_VxA_( constant, argwithextrema)
             *             _idxsel_( constant, argwithextrema)
             * Generate similar selection on the extrema
             */
            case F_idx_sel:
            case F_sel_VxA:
                arg1avis = ID_AVIS (PRF_ARG1 (rhs));
                arg2avis = ID_AVIS (PRF_ARG2 (rhs));
                if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))
                    && (IVEXPisAvisHasMin (arg2avis))
                    && (COisConstant (PRF_ARG1 (rhs)))) {
                    /* select from AVIS_MIN */
                    minv = DUPdoDupNode (rhs);
                    FREEdoFreeNode (PRF_ARG2 (minv));
                    PRF_ARG2 (minv) = DUPdoDupNode (AVIS_MIN (arg2avis));
                    arg2type = TUgetBaseSimpleType (AVIS_TYPE (arg2avis));
                    minv = FLATGflattenExpression (minv, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info),
                                                   TYmakeAKS (TYmakeSimpleType (arg2type),
                                                              SHmakeShape (0)));
                    AVIS_ISMINHANDLED (minv) = TRUE;
                    INFO_MINVAL (arg_info) = TBmakeId (minv);
                }

                if ((!IVEXPisAvisHasMax (lhsavis)) && (!AVIS_ISMAXHANDLED (lhsavis))
                    && (IVEXPisAvisHasMax (arg2avis))
                    && (COisConstant (PRF_ARG1 (rhs)))) {
                    /* select from AVIS_MAX */
                    maxv = DUPdoDupNode (rhs);
                    FREEdoFreeNode (PRF_ARG2 (maxv));
                    PRF_ARG2 (maxv) = DUPdoDupNode (AVIS_MAX (arg2avis));
                    arg2type = TUgetBaseSimpleType (AVIS_TYPE (arg2avis));
                    maxv = FLATGflattenExpression (maxv, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info),
                                                   TYmakeAKS (TYmakeSimpleType (arg2type),
                                                              SHmakeShape (0)));
                    AVIS_ISMAXHANDLED (maxv) = TRUE;
                    INFO_MAXVAL (arg_info) = TBmakeId (maxv);
                }
                break;

            default:
                break;

            } /* end of switch */
    }

    IVEXPsetMinvalIfNotNull (lhsavis, INFO_MINVAL (arg_info), FALSE);
    IVEXPsetMaxvalIfNotNull (lhsavis, INFO_MAXVAL (arg_info), FALSE);

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
 * function:
 *   node *PropagatePrfExtrema( node *arg_node, info *arg_info)
 *
 * description:
 *   Propagate any extrema from RHS (etc.) to LHS for N_prf nodes
 *
 * param: arg_node: N_let node.
 *
 * result: The updated arg_node.
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
    node *maxv;

    DBUG_ENTER ();

    lhsid = LET_IDS (arg_node);
    lhsavis = IDS_AVIS (lhsid);
    rhs = LET_EXPR (arg_node);
    withid = INFO_CURWITH (arg_info);
    withid = (NULL != withid) ? PART_WITHID (WITH_PART (withid)) : NULL;

    switch (PRF_PRF (rhs)) {

    case F_saabind:
        rhsavis = ID_AVIS (PRF_ARG3 (rhs));
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (rhsavis), TRUE);
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (rhsavis), TRUE);
        break;

    case F_non_neg_val_S:
    case F_non_neg_val_V:
    case F_afterguard:
    case F_noteintersect:
    case F_shape_matches_dim_VxA:
    case F_val_lt_shape_VxA:
    case F_val_le_val_SxS:
    case F_val_le_val_VxV:
    case F_val_lt_val_SxS:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (rhsavis), TRUE);
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (rhsavis), TRUE);
        if (TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG1 (rhs))))) {
            IVEXPsetMinvalIfNotNull (lhsavis, PRF_ARG1 (rhs), TRUE);
            if (NULL == AVIS_MAX (lhsavis)) {
                maxv = IVEXPadjustExtremaBound (rhsavis, 1, &INFO_VARDECS (arg_info),
                                                &INFO_PREASSIGNS (arg_info), "dsf9");
                IVEXPsetMaxvalIfNotNull (lhsavis, TBmakeId (maxv), FALSE);
            }
        }
        break;

    case F_noteminval:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        IVEXPsetMinvalIfNotNull (lhsavis, PRF_ARG2 (rhs), TRUE);
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (rhsavis), TRUE);
        break;

    case F_notemaxval:
        rhsavis = ID_AVIS (PRF_ARG1 (rhs));
        IVEXPsetMaxvalIfNotNull (lhsavis, PRF_ARG2 (rhs), TRUE);
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (rhsavis), TRUE);
        break;

    case F_min_SxS:
    case F_min_VxV:
        /* If either argument has a maxval, propagate it */
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (ID_AVIS (PRF_ARG1 (rhs))), TRUE);
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (ID_AVIS (PRF_ARG2 (rhs))), TRUE);
        break;

    case F_max_SxS:
    case F_max_VxV:
        /* If either argument has a minval, propagate it */
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (ID_AVIS (PRF_ARG1 (rhs))), TRUE);
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (ID_AVIS (PRF_ARG2 (rhs))), TRUE);
        break;

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

    DBUG_ENTER ();

    rhs = LET_EXPR (arg_node);
    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    switch (NODE_TYPE (rhs)) {
    case N_id:
        rhsavis = ID_AVIS (rhs);
        IVEXPsetMinvalIfNotNull (lhsavis, AVIS_MIN (rhsavis), TRUE);
        IVEXPsetMaxvalIfNotNull (lhsavis, AVIS_MAX (rhsavis), TRUE);
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
    char *lhsname;
    ntype *typ;

    DBUG_ENTER ();

    lhsavis = IDS_AVIS (LET_IDS (arg_node));

    lhsname = AVIS_NAME (lhsavis); /* Handy for ddd condition-setting */
#ifdef VERBOSE
    DBUG_PRINT ("Looking at %s", AVIS_NAME (lhsavis));
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

    DBUG_ENTER ();

    oldwith = INFO_CURWITH (arg_info);
    INFO_CURWITH (arg_info) = arg_node;
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    INFO_CURWITH (arg_info) = oldwith;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPpart( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

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

    DBUG_ASSERT (INFO_VARDECS (arg_info) == NULL, "IVEXPfundef INFO_VARDECS not NULL");

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
