/*
 * $Id$
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
 * @brief:  This phase counts the number of _sel_VxA_(iv, foldeeWL)
 *          references within a folderWL to the foldeeWL.
 *
 *          The intent is to determine whether or not all references
 *          to the foldeeWL are from the folderWL.
 *          If so, the foldeeWL may be foldable into the folderWL.
 *          If not, no folding can occur.
 *
 *****************************************************************************/
#include "wl_needcount.h"

#include "tree_basic.h"
#include "tree_compound.h"
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
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void incrementNeedcount( node *avis, info *arg_info)
 *
 * @brief  Conditionally increments WL use count for the
 *         foldeeWL result.
 *
 *****************************************************************************/
static void
incrementNeedcount (node *avis, info *arg_info)
{

    DBUG_ENTER ("incrementNeedCount");

    if (((NULL == AVIS_COUNTING_WL (avis))
         || (AVIS_COUNTING_WL (avis) == INFO_WITH (arg_info)))) {
        AVIS_WL_NEEDCOUNT (avis) += 1;
        AVIS_COUNTING_WL (avis) = INFO_WITH (arg_info);
        DBUG_PRINT ("WLNC", ("WLNCid incremented AVIS_WL_NEEDCOUNT(%s)=%d",
                             AVIS_NAME (avis), AVIS_WL_NEEDCOUNT (avis)));
    } else {
        DBUG_PRINT ("WLNC", ("incrementNeedCount(%s) reference from different WL.",
                             ID_AVIS (avis)));
    }

    DBUG_VOID_RETURN;
}

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

    DBUG_PRINT ("WLNC", ("WL-needcounting for %s %s begins",
                         (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                         FUNDEF_NAME (arg_node)));
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_PRINT ("WLNC", ("WL-needcounting for %s %s ends",
                         (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                         FUNDEF_NAME (arg_node)));

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
 * @brief  Reset all counters. This works because we traverse
 *         the N_vardec entries upon entry to each block.
 *
 *****************************************************************************/
node *
WLNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLNCavis");

    DBUG_PRINT ("WLNC", ("Zeroing AVIS_WL_NEEDCOUNT(%s)", AVIS_NAME (arg_node)));
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
    node *avis;

    DBUG_ENTER ("WLNCwith");

    outer_with = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    /* Account for modarray(foldeeWL). This could be done
     * with a traversal  through WITH_WITHOP, but I'm lazy.
     * Actually, it looks like wl_needcount.c, which does the
     * non-WL needcount work, ignores WITH_WITHOP, so this
     * code should not be needed.
    if ( N_modarray == NODE_TYPE( WITH_WITHOP( arg_node))) {
      avis = ID_AVIS( MODARRAY_ARRAY( WITH_WITHOP( arg_node)));
      incrementNeedcount( avis, arg_info);
    }
     */

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
 * @brief Count sel() references in WLs for one WL only.
 *        I am not sure what the default case was originally intended to do.
 *
 *        The AVIS_COUNTING_WL code is to ensure that only one
 *        folderWL contributes to the count.
 *        That is the first-past-the-post WL: if more than one WL
 *        tries to contribute, the latter ones will be ignored.
 *        This will cause a mismatch in SWLF between AVIS_NEEDCOUNT
 *        and AVIS_WL_NEEDCOUNT, properly preventing the folding from
 *        happening.
 *
 *****************************************************************************/
node *
WLNCid (node *arg_node, info *arg_info)
{
    node *avis;
    node *parent;
    DBUG_ENTER ("WLNCid");

    parent = INFO_FUN (arg_info);
    if ((parent != NULL) && (N_prf == NODE_TYPE (parent))) {
        switch (PRF_PRF (parent)) {
        case F_sel_VxA:
            avis = ID_AVIS (PRF_ARG2 (parent));
            DBUG_EXECUTE ("WLNC", PRTdoPrintNode (parent););
            DBUG_PRINT ("WLNC", ("WLNCid looking at %s.", AVIS_NAME (avis)));
            if ((avis == ID_AVIS (arg_node))) {
                incrementNeedcount (avis, arg_info);
            }
            break;
        case F_idx_shape_sel: /* Don't count these */
        case F_shape_A:
        case F_saabind:
        case F_dim_A:
            break;
        default:
            break;
            /* FIXME I think this is garbage
             *
             *    AVIS_WL_NEEDCOUNT( avis) += 2;
             */
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Need Count of With-Loop -->
 *****************************************************************************/
