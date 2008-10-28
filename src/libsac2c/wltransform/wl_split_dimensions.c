/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlsd With-Loop Split Dimensions
 *
 * Module description goes here.
 *
 * @ingroup wlsd
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wl_split_dimensions.c
 *
 * Prefix: WLSD
 *
 *****************************************************************************/
#include "wl_split_dimensions.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 * INFO_WITH2_IVECT:           points to the ids of the index vector of the
 *                             with2 loop that is being transformed
 * INFO_WITH2_ISCLS:           points to the ids of the index scalars of the
 *                             with2 loop that is being transformed
 * INFO_WITH2_OFFSET:          points to the offset (wlidx) of the with2 loop
 *                             that is being transformed
 * INFO_WITH2_WITHOPS:         points to the withops of the with2 loop that is
 *                             being transformed
 *****************************************************************************/
struct INFO {
    node *with2_ivect;
    node *with2_iscls;
    node *with2_offset;
    node *with2_withops;
};

#define INFO_WITH2_IVECT(n) ((n)->with2_ivect)
#define INFO_WITH2_ISCLS(n) ((n)->with2_iscls)
#define INFO_WITH2_OFFSET(n) ((n)->with2_offset)
#define INFO_WITH2_WITHOPS(n) ((n)->with2_withops)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH2_IVECT (result) = NULL;
    INFO_WITH2_ISCLS (result) = NULL;
    INFO_WITH2_OFFSET (result) = NULL;
    INFO_WITH2_WITHOPS (result) = NULL;

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
 * @fn node *WLSDdoWithLoopSplitDimensions( node *syntax_tree)
 *
 *****************************************************************************/
node *
WLSDdoWithLoopSplitDimensions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLSDdoWithLoopSplitDimensions");

    info = MakeInfo ();

    DBUG_PRINT ("WLSD", ("Starting to split with-loops into 1-dimensional ones."));

    TRAVpush (TR_wlsd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("WLSD", ("With-loop splitting complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

#if 0
/** <!--********************************************************************-->
 *
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static 
node *DummyStaticHelper(node *arg_node)
{
  DBUG_ENTER( "DummyStaticHelper");

  DBUG_RETURN( arg_node);
}
#endif

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
 * @fn node *WLSDfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDassign");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwith2(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwith2 (node *arg_node, info *arg_info)
{
    node *with3;

    DBUG_ENTER ("WLSDwith2");

    /*
     * First of all, we transform the code blocks. As we migth potentially
     * copy them into multiple with3 bodies later on, transforming them
     * first reduces complexity. Furthermore, this ensures that we never
     * nest transformations.
     */
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    /*
     * we keep the withops as a template for later use in the new with3.
     */
    INFO_WITH2_WITHOPS (arg_info) = WITH2_WITHOP (arg_node);

    /*
     * We need the to grab the indexvector, indexscalars and withloop offset
     * from the withid.
     */
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    /*
     * Finally, go off and transform :)
     *
     * Note that this will return the new with3 structure, so we have
     * to replace this with2 node with that result and free the old
     * structure.
     */
    with3 = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    DBUG_ASSERT (((with3 != NULL) && (NODE_TYPE (with3) == N_with3)),
                 "transformation into with3 went wrong.");

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (with3);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwithid(node *arg_node, info *arg_info)
 *
 * @brief Stores the avis of the index vector, the index scalars and
 *        the withloop offset in the info structure.
 *
 *****************************************************************************/
node *
WLSDwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwithid");

    INFO_WITH2_IVECT (arg_info) = WITHID_VEC (arg_node);
    INFO_WITH2_ISCLS (arg_info) = WITHID_IDS (arg_node);
    INFO_WITH2_OFFSET (arg_info) = WITHID_IDXS (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlseg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlseg");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlsegvar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlsegvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlsegvar");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlstride(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlstride (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlstride");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlstridevar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlstridevar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlstridevar");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlgrid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlgrid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlgrid");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlgridvar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlgridvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSDwlgridvar");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With-loop Split Dimensions -->
 *****************************************************************************/
