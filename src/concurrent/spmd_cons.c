/*
 *
 * $Log$
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:02:28  sacbase
 * new release made
 *
 * Revision 1.6  1999/11/11 10:31:30  jhs
 * Inferring of first sync blocks begins at spmdfun-block not at the spmdfun
 * itself, preventing neverending recursions.
 *
 * Revision 1.5  1999/08/03 11:43:37  jhs
 * Expanded comments.
 *
 * Revision 1.4  1999/07/30 14:12:39  jhs
 * Bug removed in DBUG_PRINT arguments.
 *
 * Revision 1.3  1999/07/30 13:48:35  jhs
 * Brushed DBUG_PRINTs.
 *
 * Revision 1.2  1999/07/28 13:04:45  jhs
 * Finished implementation of all needed routines.
 *
 * Revision 1.1  1999/07/26 11:35:24  jhs
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_cons.c
 *
 * prefix: SPMDC
 *
 * description: spmd-constraining
 *   Brings Information to the spmd-blocks first available after spmd_init,
 *   but not while spmd_init or spmd_lift. So this Information has to be
 *   inferred in another traversal.
 *   These extra Informations are:
 *     - additional in-parameters, needed due extraction of prolog-icms
 *       of the first sync-block of each spmd-function.
 *       These are inferable only after sync_opt.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "internal_lib.h"
#include "free.h"
#include "typecheck.h"

#include "concurrent_lib.h"

#include "spmd_cons.h"

/******************************************************************************
 *
 * function:
 *   node *SPMDCspmd (node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
SPMDCspmd (node *arg_node, node *arg_info)
{
    int own_arg_info;
    node *old_firstsync = NULL;
    node *first_sync;
    char *name;
    node *fundef;
    node *old_vardec, *new_vardec, *next_vardec;
    node *new_args;

    DBUG_ENTER ("SPMDCspmd");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_spmd, "Wrong NODE_TYPE");

    /*
     *  build new arg_info or secure to be changed values of prepared one
     */
    if (arg_info == NULL) {
        own_arg_info = TRUE;
        arg_info = MakeInfo ();
    } else {
        own_arg_info = FALSE;
        old_firstsync = INFO_SPMDC_FIRSTSYNC (arg_info);
    }

    /*
     *  Evaluate the first sync-block of this spmd-block
     *  (the sync-block is contained in the lifted function).
     *
     *  Same traversal-tab is used here!
     */
    INFO_SPMDC_FIRSTSYNC (arg_info) = NULL;
    FUNDEF_BODY (SPMD_FUNDEF (arg_node))
      = Trav (FUNDEF_BODY (SPMD_FUNDEF (arg_node)), arg_info);
    first_sync = INFO_SPMDC_FIRSTSYNC (arg_info);

    DBUG_ASSERT (first_sync != NULL, "no first sync-block found");

    CONLDisplayMask ("SPMDC", "sync-inout", SYNC_INOUT (first_sync));
    CONLDisplayMask ("SPMDC", "spmd-in", SPMD_IN (arg_node));

    /*
     *  To pass values produced by extracted prolog-icms, one needs to
     *  add the inout-variables of the first sync-block of a spmd-block
     *  to the in-variables of this spmd-block.
     *
     *  This includes to move the vardecs (in the spmd-function), belonging
     *  to these inout-variables to the args (of the spmd-function).
     */
    fundef = SPMD_FUNDEF (arg_node);
    old_vardec = BLOCK_VARDEC (FUNDEF_BODY (fundef));
    new_vardec = NULL;
    new_args = FUNDEF_ARGS (fundef);
    while (old_vardec != NULL) {
        /*
         *  the old_vardec-node is modified, so the actual next-pointer will be not
         *  available after the following changes, so get catch it here.
         */
        next_vardec = VARDEC_NEXT (old_vardec);
        if (DFMTestMaskEntry (SYNC_INOUT (first_sync), NULL, old_vardec)) {
            /* move to args */
            name = VARDEC_NAME (old_vardec);
            /* adding to in-mask of spmd */
            DFMSetMaskEntrySet (SPMD_IN (arg_node), name, NULL);
            /*
             *  **** Now happens something very ugly: ****
             *  Because of back-references, one wants to keep the pointer to the
             *  vardecs, but these have to be args now ...
             *  (otherwise one would have to traverse the whole thing again).
             *  So the NODE_TYPE is changed here from N_vardec to N_arg!!!
             */
            NODE_TYPE (old_vardec) = N_arg;
            ARG_NEXT (old_vardec) = new_args;
            ARG_FUNDEF (old_vardec) = fundef;
            ARG_STATUS (old_vardec) = ST_regular;
            ARG_ATTRIB (old_vardec) = ST_regular;
            ARG_REFCNT (old_vardec) = GET_STD_REFCNT (VARDEC, old_vardec);
            ARG_NAIVE_REFCNT (old_vardec) = GET_STD_REFCNT (VARDEC, old_vardec);
            new_args = old_vardec;
        } else {
            /* no mask is changed, keep as vardec */
            VARDEC_NEXT (old_vardec) = new_vardec;
            new_vardec = old_vardec;
        }
        old_vardec = next_vardec;
    }
    BLOCK_VARDEC (FUNDEF_BODY (fundef)) = new_vardec;
    FUNDEF_ARGS (fundef) = new_args;

    CONLDisplayMask ("SPMDC", "sync-inout", SYNC_INOUT (first_sync));
    CONLDisplayMask ("SPMDC", "spmd-in", SPMD_IN (arg_node));

    /*
     *  delete or reset arg_info
     */
    if (own_arg_info) {
        FreeTree (arg_info);
    } else {
        INFO_SPMDC_FIRSTSYNC (arg_info) = old_firstsync;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDCsync (node *arg_node, node *arg_info)
 *
 * description:
 *   Traversal of N_sync, during SPMDC.
 *   First sync-block reached from a spmd-block annotates itself at the
 *   arg_info-node. So one knews which sync-block is the first of an
 *   spmd-block.
 *
 ******************************************************************************/
node *
SPMDCsync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDCsync");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_sync, "Wrong NODE_TYPE");
    DBUG_ASSERT (arg_info != NULL, "arg_info == NULL");

    /*
     *  First sync-block reached annotates itself to the arg_info node.
     */
    if (INFO_SPMDC_FIRSTSYNC (arg_info) == NULL) {
        DBUG_PRINT ("SPMDC", ("First sync-block found"));
        INFO_SPMDC_FIRSTSYNC (arg_info) = arg_node;
    }

    DBUG_RETURN (arg_node);
}
