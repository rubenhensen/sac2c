/*
 * $Id: Index Vector Extrema Cleanup.c 16070 2009-05-12 04:11:07Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexc Index Vector Extrema Cleanup
 *
 * @brief  This phase resets AVIS_MIN, AVIS_MAX,
 *         and WL_COUNTING_WL fields.
 *
 *         It gets called after the SAACYC optimization cycle
 *         is complete, on the N_module.
 *
 *         It also gets called during the SAACYC cycle, to
 *         sanitize a WL partition's code, by removing AVIS_MIN, AVIS_MIN,
 *         noteminval, notemaxval, noteintersect. It does this
 *         under two circumstances:
 *
 *           - The code is part of a producerWL, and is about to
 *             be merged into a consumerWL.
 *
 *           - The code is part of a consumerWL partition that is being
 *             cube-sliced.
 *
 * @ingroup ivexc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivexcleanup.c
 *
 * Prefix: IVEXC
 *
 *****************************************************************************/
#include "ivexcleanup.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "IVEXC"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "free.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCdoIndexVectorExtremaCleanup( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEXCdoIndexVectorExtremaCleanup (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("Extrema cleanup strip traversal starts.");

    TRAVpush (TR_ivexc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("Extrema cleanup traversal complete.");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCdoIndexVectorExtremaCleanupPartition( node *arg_node)
 *
 *****************************************************************************/
node *
IVEXCdoIndexVectorExtremaCleanupPartition (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("Extrema partition cleanup traversal starts.");

    TRAVpush (TR_ivexc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("Extrema partition cleanup traversal complete.");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCpart(node *arg_node, info *arg_info)
 *
 * @brief  Resets N_with fields no longer needed by AWLF.
 *
 *****************************************************************************/
node *
IVEXCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCcode(node *arg_node, info *arg_info)
 *
 * @brief  Resets N_with fields no longer needed by AWLF.
 *
 *****************************************************************************/
node *
IVEXCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_HASEXTREMA (arg_node) = FALSE;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCid(node *arg_node, info *arg_info)
 *
 * @brief does nothing.
 *
 *****************************************************************************/
node *
IVEXCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXClet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IVEXClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCids(node *arg_node, info *arg_info)
 *
 * @brief Clears AVIS_MIN, AVIS_MAX from any value that was
 *        set within this block.
 *
 *****************************************************************************/
node *
IVEXCids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Cleaning up %s", AVIS_NAME (IDS_AVIS (arg_node)));
    IDS_AVIS (arg_node) = TRAVdo (IDS_AVIS (arg_node), arg_info);
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCavis(node *arg_node, info *arg_info)
 *
 * @brief Clears AVIS_MIN, AVIS_MAX references
 *
 *****************************************************************************/
node *
IVEXCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Cleaning up %s", AVIS_NAME (arg_node));
    AVIS_MIN (arg_node)
      = (NULL != AVIS_MIN (arg_node)) ? FREEdoFreeNode (AVIS_MIN (arg_node)) : NULL;
    AVIS_MAX (arg_node)
      = (NULL != AVIS_MAX (arg_node)) ? FREEdoFreeNode (AVIS_MAX (arg_node)) : NULL;
    AVIS_ISMINHANDLED (arg_node) = FALSE;
    AVIS_ISMAXHANDLED (arg_node) = FALSE;
    AVIS_COUNTING_WL (arg_node) = NULL;
    AVIS_WL_NEEDCOUNT (arg_node) = 0;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCprf(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IVEXCprf (node *arg_node, info *arg_info)
{
    node *res;
    DBUG_ENTER ();

    res = arg_node;
    switch (PRF_PRF (arg_node)) {
    case F_noteminval:
    case F_notemaxval:
    case F_noteintersect:
        DBUG_PRINT_TAG ("IVEXCprf", "Deleting extrema for prf %s...",
                        PRF_NAME (PRF_PRF (arg_node)));
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        break;

    default:
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        res = arg_node;
        break;
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
