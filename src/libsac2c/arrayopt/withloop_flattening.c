/** <!--********************************************************************-->
 *
 * @defgroup wlflt Withloop Flattening Traversal
 *
 * This optimization is supposed to turn higher-dimensional
 * WLs into 1-dimensional ones whenever this can be done easily.
 *
 * Currently, we are looking for the following code pattern:
 *
 * a0, ..., am = with {
 *                ( lb <= iv = [ i0,..., in] < shp) {
 *                   <assignments>
 *                } : (val0, ..., valm);
 *              } : ( genarray( shp, def0),
 *                    ...
 *                    genarray( shp, defm) );
 *
 * where - neither iv nor i0...in are being referenced in <assignments>
 *         or val0, ..., valm
 *       - lb is a constant consisting of 0's only
 *
 * we then replace the definition of a0, ...., am by:
 *
 * lb' = [0];
 * shp' = sacprelude::prod( shp);
 * a0', ..., am' = with {
 *                   ( lb' <= iv' = [ i0' ] < shp') {
 *                      <assignments>
 *                   } : (val0, ..., valm);
 *                 } : ( genarray( shp', def0),
 *                       ...
 *                       genarray( shp', defm) );
 * a0 = _reshape_VxA_( shp, a0');
 * ...
 * am = _reshape_VxA_( shp, am');
 *
 * and we need to insert new variable declarations:
 *
 * <basetype(a)>[.] a0';
 * ...
 * <basetype(a)>[.] am';
 * int[1]{0} lb';
 * int[1] shp';
 * int[1] iv';
 * int i0';
 *
 *
 *
 * @ingroup wlflt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file withloop_flattening.c
 *
 * Prefix: WLFLT
 *
 *****************************************************************************/
#include "withloop_flattening.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "WLFLT"
#include "debug.h"

#include "traverse.h"
#include "compare_tree.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"
#include "deserialize.h"
#include "namespaces.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *     GENARRAYS  : counts the number of N_genarray operators of the
 *                  current WL ( if any) NB: it does ONLY count those
 *                  N_genarray nodes whose shapes are identical to the
 *                  shape of the first N_genarray found.
 *     SHAPE      : points to the N_avis of the shape description used
 *                  in the first N_genarray found.
 *     IDSUSED    : communicates from WLFLTwithid to WLFLTwith how many
 *                  WL identifiers are being used.
 *     ISFULLPARTITION : communicates from WLFLTgenerator to WLFLTwith
 *                       whether *all* partitions are full.
 *
 *     VARDECS    : accumulates vardec chain created on the fly
 *     PREASSIGNS : accumulates assign chain created on the fly to be
 *                  inserted before the current assign
 *     POSTASSIGNS : accumulates assign chain created on the fly to be
 *                   inserted after the current assign
 *     NEWSHP     : communicates the avis of the new upper bound from
 *                  WLFLTMgenerator to WLFLTMgenarray.
 *     LHS        : stores the lhs ids for the current expression
 *
 *****************************************************************************/
struct INFO {
    size_t genarrays;
    node *shape;
    int idsused;
    bool isfullpartition;
    node *vardecs;
    node *preassigns;
    node *postassigns;
    bool replace;
    node *newshp;
    node *lhs;
};

/**
 * INFO macros
 */
#define INFO_GENARRAYS(n) ((n)->genarrays)
#define INFO_SHAPE(n) ((n)->shape)
#define INFO_IDSUSED(n) ((n)->idsused)
#define INFO_ISFULLPARTITION(n) ((n)->isfullpartition)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_REPLACE(n) ((n)->replace)
#define INFO_NEWSHP(n) ((n)->newshp)
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_GENARRAYS (result) = 0;
    INFO_SHAPE (result) = NULL;
    INFO_IDSUSED (result) = 0;
    INFO_ISFULLPARTITION (result) = FALSE;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_REPLACE (result) = FALSE;
    INFO_NEWSHP (result) = NULL;
    INFO_LHS (result) = NULL;

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
 * @fn node *WLFLTdoWithloopFlattening( node *syntax_tree)
 *
 *****************************************************************************/
node *
WLFLTdoWithloopFlattening (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module,
                 "WLFLTdoWithloopFlattening can only be called on entire "
                 "modules!");

    DSinitDeserialize (syntax_tree);

    info = MakeInfo ();

    DBUG_PRINT ("Starting withloop flattening traversal.");

    TRAVpush (TR_wlflt);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Withloop flattening complete.");

    info = FreeInfo (info);

    DSfinishDeserialize (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

static bool
equalShapes (node *shp1, node *shp2)
{
#ifdef FLATTEN_IS_PROPER
    return (ID_AVIS (shp1) == ID_AVIS (shp2));
#else
    return (CMPTdoCompareTree (shp1, shp2) == CMPT_EQ);
#endif
}

/** <!--********************************************************************-->
 *
 * @fn node *createLowerBound( info* arg_info)
 *
 * @brief create an assignment
 *          tmp = [0];
 *        and a vardec
 *          int[1] tmp;
 *
 *        insert these into INFO_VARDECS, and INFO_PREASSIGNS, respectively
 *
 *        and return an N_id for   tmp
 *
 *****************************************************************************/
static node *
createLowerBound (info *arg_info)
{
    node *lb_avis;
    node *lb_assign;
    constant *lb_const;

    DBUG_ENTER ();

    lb_const = COmakeZero (T_int, SHcreateShape (1, 1));
    lb_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKV (TYmakeSimpleType (T_int), lb_const));
    INFO_VARDECS (arg_info) = TBmakeVardec (lb_avis, INFO_VARDECS (arg_info));

    lb_assign = TBmakeAssign (TBmakeLet (TBmakeIds (lb_avis, NULL),
                                         TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHcreateShape (0)),
                                                      SHcreateShape (1, 1),
                                                      TBmakeExprs (TBmakeNum (0), NULL))),
                              INFO_PREASSIGNS (arg_info));
    INFO_PREASSIGNS (arg_info) = lb_assign;

    AVIS_SSAASSIGN (lb_avis) = lb_assign;

    DBUG_RETURN (TBmakeId (lb_avis));
}

/** <!--********************************************************************-->
 *
 * @fn node *createUpperBound( node *bound, info* arg_info)
 *
 * @brief create an assignment
 *          tmp = sac_prelude::prod( bound);
 *        and a vardec
 *          int[1] tmp;
 *
 *        insert these into INFO_VARDECS, and INFO_PREASSIGNS, respectively
 *        insert the N_avis of tmp into INFO_NEWSHP
 *        and return an N_id for   tmp
 *
 *****************************************************************************/
static node *
createUpperBound (node *bound, info *arg_info)
{
    node *ub_avis;
    node *prod_avis;
    node *ap_node;
    node *bound_avis = NULL;
    node *bound_id;

    DBUG_ENTER ();

    prod_avis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (prod_avis, INFO_VARDECS (arg_info));

    ub_avis = TBmakeAvis (TRAVtmpVar (),
                          TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
    INFO_VARDECS (arg_info) = TBmakeVardec (ub_avis, INFO_VARDECS (arg_info));

    INFO_NEWSHP (arg_info) = ub_avis;

    /* ensure upper bound is N_id */
    if (NODE_TYPE (bound) != N_id) {
        bound_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKD (TYmakeSimpleType (T_int), 1,
                                                           SHcreateShape (0)));
        bound_id = TBmakeId (bound_avis);
        INFO_VARDECS (arg_info) = TBmakeVardec (bound_avis, INFO_VARDECS (arg_info));
    } else {
        bound_id = DUPdoDupNode (bound);
    }

    ap_node = DSdispatchFunCall (NSgetNamespace (SAC_PRELUDE_NAME), "prod",
                                 TBmakeExprs (bound_id, NULL));

    DBUG_ASSERT (ap_node != NULL, "cannot find `" SAC_PRELUDE_NAME "::prod'.");

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (ub_avis, NULL),
                                 TCmakeIntVector (
                                   TBmakeExprs (TBmakeId (prod_avis), NULL))),
                      INFO_PREASSIGNS (arg_info));
    AVIS_SSAASSIGN (ub_avis) = INFO_PREASSIGNS (arg_info);

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (prod_avis, NULL), ap_node),
                      INFO_PREASSIGNS (arg_info));
    AVIS_SSAASSIGN (prod_avis) = INFO_PREASSIGNS (arg_info);

    if (NODE_TYPE (bound) != N_id) {
        AVIS_SSAASSIGN (bound_avis) = INFO_PREASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (bound_avis, NULL), DUPdoDupNode (bound)),
                          INFO_PREASSIGNS (arg_info));
    }

    DBUG_RETURN (TBmakeId (ub_avis));
}

/** <!--********************************************************************-->
 *
 * @fn node *createReshapeAssignments( node *lhs, node* old_shp,
 *                                     node* new_shp, arg_info)
 *
 * @brief create assignments
 *          lhs1 = _reshape_VxA_( old_shp, tmp1);
 *          ...
 *          lhsn = _reshape_VxA_( old_shp, tmpn);
 *
 *        and vardecs
 *          <basetype(lhs1)>[ prod( old_shp)] tmp1;
 *          ...
 *          <basetype(lhsn)>[ prod( old_shp)] tmpn;
 *
 *        insert these into INFO_POSTASSIGNS and INFO_VARDECS, respectively
 *        and returns in N_ids chain     tmp1, ..., tmpn
 *
 *****************************************************************************/
static node *
createReshapeAssignments (node *lhs, node *old_shp, node *new_shp, info *arg_info)
{
    node *avis;
    node *new_lhs = NULL;
    ntype *newtype;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (old_shp) == N_array) || (NODE_TYPE (old_shp) == N_id)),
                 "N_array expected as 2nd arg in createReshapeAssignments");
    DBUG_ASSERT (NODE_TYPE (new_shp) == N_avis,
                 "N_avis expected as 3nd arg in createReshapeAssignments");

    if (lhs != NULL) {
        node *old_shp_id;
        node *old_shp_avis = NULL;

        new_lhs = createReshapeAssignments (IDS_NEXT (lhs), old_shp, new_shp, arg_info);

        newtype = TYmakeSimpleType (TUgetBaseSimpleType (IDS_NTYPE (lhs)));
        if (TUshapeKnown (IDS_NTYPE (lhs))) {
            newtype
              = TYmakeAKS (newtype,
                           SHcreateShape (1, SHgetUnrLen (TYgetShape (IDS_NTYPE (lhs)))));
        } else {
            newtype = TYmakeAKD (newtype, 1, SHmakeShape (0));
        }

        avis = TBmakeAvis (TRAVtmpVar (), newtype);
        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

        if (NODE_TYPE (old_shp) != N_id) {
            old_shp_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (newtype));
            INFO_VARDECS (arg_info)
              = TBmakeVardec (old_shp_avis, INFO_VARDECS (arg_info));
            old_shp_id = TBmakeId (old_shp_avis);
        } else {
            old_shp_id = DUPdoDupNode (old_shp);
        }

        INFO_POSTASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs),
                                     TCmakePrf2 (F_reshape_VxA, old_shp_id,
                                                 TBmakeId (avis))),
                          INFO_POSTASSIGNS (arg_info));
        AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_POSTASSIGNS (arg_info);

        if (old_shp_avis != NULL) {
            AVIS_SSAASSIGN (old_shp_avis) = INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (old_shp_avis, NULL),
                                         DUPdoDupNode (old_shp)),
                              INFO_POSTASSIGNS (arg_info));
        }

        new_lhs = TBmakeIds (avis, new_lhs);
    }

    DBUG_RETURN (new_lhs);
}

/** <!--********************************************************************-->
 *
 * @fn node *createWLAssignAndReshapes( node *with, info* arg_info)
 *
 * @brief create a assignments
 *          tmp1, ..., tmpn = with...;
 *          lhs1 = _reshape_VxA_( SHAPE( arg_info), tmp1);
 *          ...
 *          lhsn = _reshape_VxA_( SHAPE( arg_info), tmpn);
 *
 *        and vardecs
 *          <basetype(lhs1)>[NEWSHP(arg_info)] tmp1;
 *          ...
 *          <basetype(lhsn)>[NEWSHP(arg_info)] tmpn;
 *
 *        insert these into INFO_VARDECS, set INFO_REPLACE
 *        and return the assignment chain.
 *
 *****************************************************************************/
static node *
createWLAssignAndReshapes (node *with, info *arg_info)
{
    node *new_lhs, *assigns;

    DBUG_ENTER ();

    new_lhs = createReshapeAssignments (INFO_LHS (arg_info), INFO_SHAPE (arg_info),
                                        INFO_NEWSHP (arg_info), arg_info);
    INFO_SHAPE (arg_info) = FREEdoFreeNode (INFO_SHAPE (arg_info));
    INFO_NEWSHP (arg_info) = NULL;

    assigns = TBmakeAssign (TBmakeLet (new_lhs, DUPdoDupTree (with)),
                            INFO_POSTASSIGNS (arg_info));

    while (new_lhs != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (new_lhs)) = assigns;
        new_lhs = IDS_NEXT (new_lhs);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTMgenerator(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static node *
WLFLTMgenerator (node *arg_node, info *arg_info)
{
    node *lb_id, *ub_id;

    DBUG_ENTER ();

    lb_id = createLowerBound (arg_info);
    ub_id = createUpperBound (GENERATOR_BOUND2 (arg_node), arg_info);

    arg_node = FREEdoFreeTree (arg_node);

    arg_node = TBmakeGenerator (F_wl_le, F_wl_lt, lb_id, ub_id, NULL, NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTMgenarray(node *arg_node, info *arg_info)
 *
 * @brief  replace existing shape by INFO_NEWSHP(arg_info)!
 *
 *****************************************************************************/
static node *
WLFLTMgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SHAPE (arg_info) = GENARRAY_SHAPE (arg_node);
    GENARRAY_SHAPE (arg_node) = TBmakeId (INFO_NEWSHP (arg_info));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        INFO_SHAPE (arg_info) = FREEdoFreeTree (INFO_SHAPE (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTMwithid(node *arg_node, info *arg_info)
 *
 * @brief replace existing withids by
 *              tmp2 = [tmp];
 *
 *        add decls
 *              int    tmp;
 *              int[1] tmp2;
 *        to INFO_VARDECS( arg_info)
 *
 *****************************************************************************/
static node *
WLFLTMwithid (node *arg_node, info *arg_info)
{
    node *ids_avis, *vec_avis;

    DBUG_ENTER ();

    if (WITHID_IDS (arg_node) != NULL) {
        WITHID_IDS (arg_node) = FREEdoFreeTree (WITHID_IDS (arg_node));
    }

    ids_avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (ids_avis, INFO_VARDECS (arg_info));
    WITHID_IDS (arg_node) = TBmakeIds (ids_avis, NULL);

    WITHID_VEC (arg_node) = FREEdoFreeNode (WITHID_VEC (arg_node));
    vec_avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
    INFO_VARDECS (arg_info) = TBmakeVardec (vec_avis, INFO_VARDECS (arg_info));
    WITHID_VEC (arg_node) = TBmakeIds (vec_avis, NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("WLFLT_TAG", "Tagging %s as used.", AVIS_NAME (ID_AVIS (arg_node)));

    AVIS_ISUSED (ID_AVIS (arg_node)) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTfundef(node *arg_node, info *arg_info)
 *
 * @brief Does stuff.
 *
 *****************************************************************************/
node *
WLFLTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * As we do not trust the previous state, we clear the AVIS_ISUSED.
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Remove the information we have stored in AVIS_ISUSED again.
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Clear left-over information from the AVIS_ISUSED flag.
     */
    if (BLOCK_VARDECS (arg_node) != NULL) {
        BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
    }

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /*
     * Clear the AVIS_ISUSED flag, again.
     */
    if (BLOCK_VARDECS (arg_node) != NULL) {

        BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDECS (arg_info), BLOCK_VARDECS (arg_node));
            INFO_VARDECS (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTavis(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("WLFLT_TAG", "Clearing %s's used flag.", AVIS_NAME (arg_node));

    AVIS_ISUSED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTwith (node *arg_node, info *arg_info)
{
    size_t wlopsno;
    node *oldlhs;

    DBUG_ENTER ();

    DBUG_ASSERT (WITH_WITHOP (arg_node) != NULL, "Malformed withloop: withop missing.");

    /*
     * For the use analysis, we have to always traverse the entire tree,
     * regardless of whether we do the actual optimisation or not.
     * we have to stack the lhs here, as the code may contain an
     * assignments chain with further let nodes.
     */
    oldlhs = INFO_LHS (arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_LHS (arg_info) = oldlhs;

    /*
     * Now, we infer INFO_GENARRAYS and INFO_SHAPE
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    INFO_IDSUSED (arg_info) = 0;
    INFO_ISFULLPARTITION (arg_info) = TRUE;
    /**
     * The traversal of N_part serves 2 purposes:
     *
     * - count the number of used(!) N_ids nodes encountered during traversal
     * of N_withids, i.e., check whether the index vector or its scalarized
     * components are referred to anywhere in the code (INFO_IDSUSED > 0)!
     *
     * - check whether all generators are full partitions while traversing
     * N_generator.
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    wlopsno = TCcountWithops (WITH_WITHOP (arg_node));
    if ((INFO_GENARRAYS (arg_info) == wlopsno) && (INFO_IDSUSED (arg_info) == 0)
        && (INFO_ISFULLPARTITION (arg_info))
        && ((!TUdimKnown (IDS_NTYPE (INFO_LHS (arg_info))))
            || (TYgetDim (IDS_NTYPE (INFO_LHS (arg_info))) > 1))) {
        anontrav_t modtrav[4] = {{N_generator, &WLFLTMgenerator},
                                 {N_genarray, &WLFLTMgenarray},
                                 {N_withid, &WLFLTMwithid},
                                 {(nodetype)0, NULL}};

        DBUG_PRINT ("Found victim!");

        TRAVpushAnonymous (modtrav, &TRAVsons);

        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        TRAVpop ();

        INFO_POSTASSIGNS (arg_info) = createWLAssignAndReshapes (arg_node, arg_info);
        INFO_REPLACE (arg_info) = TRUE;
    }

    INFO_GENARRAYS (arg_info) = 0;
    INFO_ISFULLPARTITION (arg_info) = FALSE;
    INFO_SHAPE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTgenarray(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#ifdef FLATTEN_IS_PROPER
    DBUG_ASSERT (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id,
                 "Malformed withloop: non-id node as genarray shape.");
#else
    DBUG_ASSERT ((NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id)
                   || (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_array),
                 "Malformed withloop: non-id/array node as genarray shape.");
#endif

    if (INFO_SHAPE (arg_info) == NULL) {
        INFO_SHAPE (arg_info) = GENARRAY_SHAPE (arg_node);
        INFO_GENARRAYS (arg_info)++;

        arg_node = TRAVcont (arg_node, arg_info);
    } else if (equalShapes (INFO_SHAPE (arg_info), GENARRAY_SHAPE (arg_node))) {
        INFO_GENARRAYS (arg_info)++;

        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTids(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_ISUSED (IDS_AVIS (arg_node))) {
        INFO_IDSUSED (arg_info)++;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTwithid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITHID_VEC (arg_node) != NULL) {
        WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
    }

    if (WITHID_IDS (arg_node) != NULL) {
        WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
    }

    /*
     * We intentionally do not traverse into WITHID_IDXS.
     */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTgenerator(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTgenerator (node *arg_node, info *arg_info)
{
    bool stepok, widthok, lowerok, upperok;

    DBUG_ENTER ();

    stepok = (GENERATOR_STEP (arg_node) == NULL);
    widthok = (GENERATOR_WIDTH (arg_node) == NULL);
    upperok = equalShapes (GENERATOR_BOUND2 (arg_node), INFO_SHAPE (arg_info))
              && (GENERATOR_OP2 (arg_node) == F_wl_lt);

#if FLATTEN_IS_PROPER
    {
        node *loweravis;
        loweravis = ID_AVIS (GENERATOR_BOUND1 (arg_node));
        lowerok = (GENERATOR_OP1 (arg_node) == F_wl_le)
                  && (TYisAKV (AVIS_TYPE (loweravis)))
                  && (COisZero (TYgetValue (AVIS_TYPE (loweravis)), TRUE));
    }
#else
    {
        constant *lower_const = NULL;

        lower_const = COaST2Constant (GENERATOR_BOUND1 (arg_node));
        lowerok = (lower_const != NULL) && (GENERATOR_OP1 (arg_node) == F_wl_le)
                  && COisZero (lower_const, TRUE);
    }
#endif

    INFO_ISFULLPARTITION (arg_info)
      = INFO_ISFULLPARTITION (arg_info) && stepok && widthok && upperok && lowerok;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }

    if (INFO_REPLACE (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REPLACE (arg_info) = FALSE;
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal withloop flattening -->
 *****************************************************************************/

#undef DBUG_PREFIX
