/*
 * $Id: wl_needcount.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlnc Need Count in With-Loop
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wl_needcount.c
 *
 * Prefix: WLNC
 *
 *****************************************************************************/
#include "wl_needcount.h"

#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "print.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *with;
    node *fun;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_WITH(n) ((n)->with)
#define INFO_FUN(n) ((n)->fun)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH (result) = NULL;
    INFO_FUN (result) = NULL;

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
 * @fn node *WLNCdoWLNeedCount( node *fundef)
 *
 * @brief starting point of needcount inference
 *
 * @param fundef
 * @return
 *
 *****************************************************************************/
node *
WLNCdoWLNeedCount (node *fundef)
{
    info *info;

    DBUG_ENTER ("WLNCdoWLNeedCount");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "WLNCdoWLNeedCount called for non-fundef node");

    info = MakeInfo ();

    TRAVpush (TR_wlnc);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLNCfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCblock( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCavis( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCavis");

    AVIS_WL_NEEDCOUNT (arg_node) = 0;
    AVIS_COUNTING_WL (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCwith( node *arg_node, info *arg_info)
 *
 * @brief applies WLNC on a with loop
 *
 *****************************************************************************/
node *
WLNCwith (node *arg_node, info *arg_info)
{
    node *outer_with;

    DBUG_ENTER ("WLNCwith");

    outer_with = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    INFO_WITH (arg_info) = outer_with;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCpart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCpart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCprf");

    INFO_FUN (arg_info) = arg_node;

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    INFO_FUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCap");

    INFO_FUN (arg_info) = arg_node;

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    INFO_FUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLNCid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLNCid (node *arg_node, info *arg_info)
{
    node *avis;
    node *parent;
    bool done;
    DBUG_ENTER ("WLNCid");

    avis = ID_AVIS (arg_node);
    parent = INFO_FUN (arg_info);
    done = FALSE;

    if (parent != NULL) {
        if (NODE_TYPE (parent) == N_prf && PRF_PRF (parent) == F_sel) {
            if (AVIS_COUNTING_WL (avis) == NULL
                || AVIS_COUNTING_WL (avis) != INFO_WITH (arg_info)) {
                AVIS_WL_NEEDCOUNT (avis) += 1;
                AVIS_COUNTING_WL (avis) = INFO_WITH (arg_info);
                done = TRUE;
            }
        }
    }

    if (!done)
        AVIS_WL_NEEDCOUNT (avis) += 2;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Need Count of With-Loop -->
 *****************************************************************************/
