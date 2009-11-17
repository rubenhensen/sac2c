/** <!--********************************************************************-->
 *
 * @defgroup Minimize the number of host<->device transfers in a
 *           sequential block of instructions.
 *
 *   This modules tries to eliminate <host2device>/<device2host> instructions
 *   in a sequential block of code. Two difference cases expose the opportunities
 *   for elimination:
 *
 *   1) a_host = device2host( b_dev);
 *      ...
 *      ...
 *      a_dev = host2device( a_host);
 *
 *      The second memory transfer, i.e. a_dev = host2device( a_host)
 *      can be eliminated. Any reference to a_dev after it will be
 *      replaced by b_dev.
 *
 *
 *
 *   2) b_dev = host2device( a_host);
 *      ...
 *      ...
 *      c_dev = host2device( a_host);
 *
 *      The second memory transfer, i.e. c_dev = host2device( a_host)
 *      can be eliminated. Any reference to c_dev after it will be
 *      replaced by b_dev.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file minimize_block_transfers.c
 *
 * Prefix: MBTRAN
 *
 *****************************************************************************/
#include "minimize_block_transfers.h"

#include <stdlib.h>
#include "new_types.h"
#include "tree_compound.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "LookUpTable.h"
#include "memory.h"
#include "dbug.h"
#include "deadcoderemoval.h"
#include "cuda_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lastassign;
    node *letids;
    bool incudawl;
    lut_t *lut;
};

#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INCUDAWL(n) (n->incudawl)
#define INFO_LUT(n) (n->lut)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LASTASSIGN (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_LUT (result) = NULL;

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
 * @fn node *MBTRANdoMinimizeBlockTransfers( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANdoMinimizeBlockTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("MBTRANdoMinimizeBlockTransfers");

    info = MakeInfo ();

    TRAVpush (TR_mbtran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /* We rely on Dead Code Removal to remove the
     * unused <host2device>/<device2host> */
    syntax_tree = DCRdoDeadCodeRemovalModule (syntax_tree);

    DBUG_RETURN (syntax_tree);
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
 * @fn node *MBTRANfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MBTRANfundef");

    /* We create a lookup table for each N_fundef */
    INFO_LUT (arg_info) = LUTgenerateLut ();
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MBTRANassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MBTRANassign");

    INFO_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MBTRANcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANcond (node *arg_node, info *arg_info)
{
    lut_t *old_lut;

    DBUG_ENTER ("MBTRANcond");

    /* Stack LUT */
    old_lut = INFO_LUT (arg_info);

    /* For a conditional, we traverse it's two branches
     * independently, each with its own lookup table. */
    INFO_LUT (arg_info) = LUTgenerateLut ();
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    INFO_LUT (arg_info) = LUTgenerateLut ();
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    /* Pop LUT */
    INFO_LUT (arg_info) = old_lut;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MBTRANlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MBTRANlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MBTRANwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MBTRANwith (node *arg_node, info *arg_info)
{
    bool old_incudawl;

    DBUG_ENTER ("MBTRANwith");

    /* We only traverse cudarizable N_with */
    if (WITH_CUDARIZABLE (arg_node)) {
        old_incudawl = INFO_INCUDAWL (arg_info);
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = old_incudawl;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MBTRANid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRANid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("MBTRANid");

    if (ISHOST2DEVICE (INFO_LASTASSIGN (arg_info))) {
        /*
         * e.g. a_dev = host2device( a_host);
         * If the a search of N_avis(host) in the table hits, it means
         * either one of the following instruction has been executed
         * before:
         * 1) a_host = device2host( b_dev);
         * 2) b_dev = host2device( a_host);
         *
         * This is becuase 1) everytime we come across <device2host>,
         * we insert N_avis(host)->N_avis(dev) into the table and 2)
         * everytime we come across <host2device> and the N_avis(host)
         * is not in the table yet, we insert N_avis(host)->N_avis(dev)
         * into the table. If the search hits, the accociated N_avis must
         * be a device N_avis. We then insert pair N_avis(a_dev)->N_avis(b_dev)
         * into the table. Thereafter, each later reference to a_dev
         * will be replaced by b_dev.
         *
         * If the search doesn't hit, insert N_avis(host)->N_avis(dev)
         * into the table, i.e. case 2) above.
         */
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

        /* If the search hit in the table */
        if (avis != ID_AVIS (arg_node)) {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                                   avis);
        }
        /* If the search doesn't hit in the table */
        else {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (arg_node),
                                   IDS_AVIS (INFO_LETIDS (arg_info)));
        }
    } else if (ISDEVICE2HOST (INFO_LASTASSIGN (arg_info))) {
        /* Insert the pair N_avis(host) -> N_avis(dev) */
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                               ID_AVIS (arg_node));
    } else {
        /* If the N_id is not in <host2device>/<device2host> */
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        /* If the N_avis hits in the table */
        if (avis != ID_AVIS (arg_node)) {
            if (INFO_INCUDAWL (arg_info)) {
                /* If the N_id is in a cudarizbale N_with, we update its N_avis */
                ID_AVIS (arg_node) = avis;
            } else {
                /* if their types are equal, i.e. they are all of device type */
                if (TYeqTypes (AVIS_TYPE (avis), AVIS_TYPE (ID_AVIS (arg_node)))) {
                    ID_AVIS (arg_node) = avis;
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
