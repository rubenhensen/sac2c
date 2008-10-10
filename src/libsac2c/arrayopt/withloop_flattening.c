/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlflt Withloop Flattening Traversal
 *
 * Module description goes here.
 *
 * For an example, take a look at src/refcount/explicitcopy.c
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
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "constants.h"
#include "tree_compound.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    int genarrays;
    node *shape;
    int idsused;
    bool isfullpartition;
};

/**
 * INFO macros
 */
#define INFO_GENARRAYS(n) ((n)->genarrays)
#define INFO_SHAPE(n) ((n)->shape)
#define INFO_IDSUSED(n) ((n)->idsused)
#define INFO_ISFULLPARTITION(n) ((n)->isfullpartition)

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

    DBUG_PRINT ("WLFLT", ("Tagging %s as used.", AVIS_NAME (ID_AVIS (arg_node))));

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

    DBUG_PRINT ("WLFLT", ("Clearing %s's used flag.", AVIS_NAME (arg_node)));

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
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    INFO_IDSUSED (arg_info) = 0;
    INFO_ISFULLPARTITION (arg_info) = TRUE;
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    wlopsno = TCcountWithops (WITH_WITHOP (arg_node));
    if ((INFO_GENARRAYS (arg_info) == wlopsno) && (INFO_IDSUSED (arg_info) == 0)
        && (INFO_ISFULLPARTITION (arg_info))) {
        DBUG_PRINT ("WLFLT", ("Found victim!"));

        /* do the optimisation */
    }

    INFO_GENARRAYS (arg_info) = 0;

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

    DBUG_ASSERT ((NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id),
                 "Malformed withloop: non-id node as genarray shape.");

    if (INFO_SHAPE (arg_info) == NULL) {
        INFO_SHAPE (arg_info) = ID_AVIS (GENARRAY_SHAPE (arg_node));
        INFO_GENARRAYS (arg_info)++;

        arg_node = TRAVcont (arg_node, arg_info);
    } else if (INFO_SHAPE (arg_info) == ID_AVIS (GENARRAY_SHAPE (arg_node))) {
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
    node *loweravis;

    DBUG_ENTER ("WLFLTgenerator");

    stepok = (GENERATOR_STEP (arg_node) == NULL);
    widthok = (GENERATOR_WIDTH (arg_node) == NULL);
    upperok = (ID_AVIS (GENERATOR_BOUND2 (arg_node)) == INFO_SHAPE (arg_info))
              && (GENERATOR_OP2 (arg_node) == F_lt_VxV);

    loweravis = ID_AVIS (GENERATOR_BOUND1 (arg_node));
    lowerok = (GENERATOR_OP1 (arg_node) == F_le_VxV) && (TYisAKV (AVIS_TYPE (loweravis)))
              && (COisZero (TYgetValue (AVIS_TYPE (loweravis)), TRUE));

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

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal withloop flattening -->
 *****************************************************************************/
