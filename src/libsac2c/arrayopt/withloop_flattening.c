/*
 * $Id$
 */

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
#include "dbug.h"
#include "traverse.h"
#include "compare_tree.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *     GENARRAYS : counts the number of N_genarray operators of the
 *                 current WL ( if any) NB: it does ONLY count those
 *                 N_genarray nodes whose shapes are identical to the
 *                 shape of the first N_genarray found.
 *     SHAPE     : points to the N_avis of the shape descrition used
 *                 in the first N_genarray found.
 *     IDSUSED   : communicates from WLFLTwithid to WLFLTwith how many
 *                 WL identifiers are being used.
 *     ISFULLPARTITION : communicates from WLFLTgenerator to WLFLTwith
 *                       whether *all* partitions are full.
 *
 *     VARDECS   : accumulates vardec chain created on the fly
 *     PREASSIGN : accumulates assign chain created on the fly
 *****************************************************************************/
struct INFO {
    int genarrays;
    node *shape;
    int idsused;
    bool isfullpartition;
    node *vardecs;
    node *assigns;
};

/**
 * INFO macros
 */
#define INFO_GENARRAYS(n) ((n)->genarrays)
#define INFO_SHAPE(n) ((n)->shape)
#define INFO_IDSUSED(n) ((n)->idsused)
#define INFO_ISFULLPARTITION(n) ((n)->isfullpartition)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGN(n) ((n)->assigns)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_GENARRAYS (result) = 0;
    INFO_SHAPE (result) = NULL;
    INFO_IDSUSED (result) = 0;
    INFO_ISFULLPARTITION (result) = FALSE;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

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

    DBUG_ENTER ("WLFLTdoWithloopFlattening");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "WLFLTdoWithloopFlattening can only be called on entire "
                 "modules!");

    info = MakeInfo ();

    DBUG_PRINT ("WLFLT", ("Starting withloop flattening traversal."));

    TRAVpush (TR_wlflt);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("WLFLT", ("Withloop flattening complete."));

    info = FreeInfo (info);

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

static node *
createLowerBound (info *arg_info)
{
    DBUG_ENTER ("createLowerBound");

    node *lb_avis;
    constant *lb_const;
    node *lb_assign;

    lb_const = COmakeZero (T_int, SHcreateShape (1, 1));
    lb_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKV (TYmakeSimpleType (T_int), lb_const));
    INFO_VARDECS (arg_info) = TBmakeVardec (lb_avis, INFO_VARDECS (arg_info));

    lb_assign = TBmakeAssign (TBmakeLet (TBmakeIds (lb_avis, NULL),
                                         TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHcreateShape (0)),
                                                      SHcreateShape (1, 1),
                                                      TBmakeExprs (TBmakeNum (0), NULL))),
                              INFO_PREASSIGN (arg_info));
    INFO_PREASSIGN (arg_info) = lb_assign;

    AVIS_SSAASSIGN (lb_avis) = lb_assign;

    DBUG_RETURN (TBmakeId (lb_avis));
}

static node *
createUpperBound (node *bound, info *arg_info)
{
    DBUG_ENTER ("createUpperBound");
    DBUG_RETURN ((node *)NULL);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

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
    DBUG_ENTER ("WLFLTid");

    DBUG_PRINT ("WLFLT_TAG", ("Tagging %s as used.", AVIS_NAME (ID_AVIS (arg_node))));

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
    DBUG_ENTER ("WLFLTfundef");

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
    DBUG_ENTER ("WLFLTblock");

    /*
     * Clear left-over information from the AVIS_ISUSED flag.
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    /*
     * Clear the AVIS_ISUSED flag, again.
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {

        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (arg_node)
              = TCappendVardec (INFO_VARDECS (arg_info), BLOCK_VARDEC (arg_node));
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
    DBUG_ENTER ("WLFLTavis");

    DBUG_PRINT ("WLFLT_TAG", ("Clearing %s's used flag.", AVIS_NAME (arg_node)));

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
    int wlopsno;

    DBUG_ENTER ("WLFLTwith");

    DBUG_ASSERT ((WITH_WITHOP (arg_node) != NULL), "Malformed withloop: withop missing.");

    /*
     * For the use analysis, we have to always traverse the entire tree,
     * regardless of whether we do the actual optimisation or not.
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
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
        && (INFO_ISFULLPARTITION (arg_info))) {
        DBUG_PRINT ("WLFLT", ("Found victim!"));
    }

    INFO_GENARRAYS (arg_info) = 0;
    INFO_ISFULLPARTITION (arg_info) = FALSE;
    INFO_SHAPE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFLTMgenerator(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLFLTMgenerator (node *arg_node, info *arg_info)
{
    node *lb_id, *ub_id;

    DBUG_ENTER ("WLFLTMgenerator");

    lb_id = createLowerBound (arg_info);
    ub_id = createUpperBound (GENERATOR_BOUND2 (arg_node), arg_info);

    arg_node = FREEdoFreeTree (arg_node);

    arg_node = TBmakeGenerator (F_wl_le, F_wl_lt, lb_id, ub_id, NULL, NULL);

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
    DBUG_ENTER ("WLFLTgenarray");

#ifdef FLATTEN_IS_PROPER
    DBUG_ASSERT ((NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id),
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
    DBUG_ENTER ("WLFLTids");

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
    DBUG_ENTER ("WLFLTwithid");

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

    DBUG_ENTER ("WLFLTgenerator");

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
    DBUG_ENTER ("WLFLTassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal withloop flattening -->
 *****************************************************************************/
