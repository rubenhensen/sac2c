/*****************************************************************************
 *
 *
 * file:   minimize_block_transfers.c
 *
 * prefix: MBTRAN
 *
 * description:
 *
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

/*
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *letids;
    bool incudawl;
    lut_t *lut;
};

/*
 * INFO macros
 */

#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INCUDAWL(n) (n->incudawl)
#define INFO_LUT(n) (n->lut)

/*
 * INFO functions
 */
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
 *
 * @fn
 *
 * @brief node *MBTRANdoMinimizeBlockTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
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

    syntax_tree = DCRdoDeadCodeRemovalModule (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MBTRANfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MBTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MBTRANfundef");

    INFO_LUT (arg_info) = LUTgenerateLut ();
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MBTRANassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
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
 * @fn
 *
 * @brief node *MBTRANlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
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
 * @fn
 *
 * @brief node *MBTRANwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MBTRANwith (node *arg_node, info *arg_info)
{
    bool old_incudawl;

    DBUG_ENTER ("MBTRANwith");

    /* We only traverse cudarizable WL */
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
 * @fn
 *
 * @brief node *MBTRANid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
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
         * If the a search of a_host in the table hits, it means
         * there's a a_host = device2host( b_dev);
         * executed before. In this case, insert pair a_dev->b_dev
         * into the table. Thereafter, each later reference to
         * a_dev will be replaced by b_dev.
         * However, if the search doesn't hit, insert a_host->a_dev
         * into the table. Therefore a subsequent transfer such as
         * c_dev = host2device( a_host) will cause the pair c_dev->a_dev
         * being inserted into the table.
         */
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

        /* a_host has been transfered from device before and there's
         * no reference of a_host between these two transfers, i.e no pair
         * of avis->N_empty exists in the table. For example:
         *
         *   a_host = device2host( b_dev);
         *   ...
         *   ... (Arbitary code containing no reference to a_host)
         *   ...
         *   a_dev = host2device( a_host);
         *
         * will cause a_dev->b_dev being inserted into the table. Therefor,
         * later code such as:
         *
         *   c_dev = with { ...a_dev... }:genarray( shp);
         *
         * will be transformed into:
         *
         *   c_dev = with { ...b_dev... }:genarray( shp);
         */
        if (avis != ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                                   avis);
        } else if (avis == ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (arg_node),
                                   IDS_AVIS (INFO_LETIDS (arg_info)));
        }
    } else if (ISDEVICE2HOST (INFO_LASTASSIGN (arg_info))) {
        /* Insert the pair a_host -> a_dev */
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                               ID_AVIS (arg_node));
    } else {
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (avis != ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            if (INFO_INCUDAWL (arg_info)) {
                /* If the ID is in a CUDA Withloop, we replace its AVIS with
                 * the AVIS of the corresponding device variable AVIS.
                 */
                ID_AVIS (arg_node) = avis;
            } else {
                /* if their types are equal, i.e. they are all of device tyoe */
                if (TYeqTypes (AVIS_TYPE (avis), AVIS_TYPE (ID_AVIS (arg_node)))) {
                    ID_AVIS (arg_node) = avis;
                } else {
                    /* If the ID occurs in code other than a CUDA Withloop, we
                     * insert a pair avis -> N_empty to indicate that the following
                     * host2device primitive cannot be removed.
                     */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, TBmakeEmpty ());
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}
