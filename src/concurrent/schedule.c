/*
 *
 * $Log$
 * Revision 3.3  2001/03/14 15:58:54  ben
 * MakeDefaultSchedulingVarSegment() modified (assertion deleted)
 *
 * Revision 3.2  2000/12/15 13:07:13  dkr
 * DBUG_ASSERT for BlockVar scheduler added
 *
 * Revision 3.1  2000/11/20 18:02:27  sacbase
 * new release made
 *
 * Revision 2.6  2000/10/31 23:18:50  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 2.5  1999/09/01 17:14:23  jhs
 * Remove SYNC_SCHEDULING.
 *
 * Revision 2.4  1999/07/23 12:44:08  cg
 * Bug fixed in compilation of multi-threaded nested with-loops.
 * Now, schedulings for inner with-loops in with-loop nestings
 * are succesfully removed.
 *
 * Revision 2.3  1999/07/07 15:50:38  jhs
 * Corrected some copypasted bugs in some assertions.
 *
 * Revision 2.2  1999/07/07 14:29:45  jhs
 * Removed SYNC_WITH_PTRS from routine SCHEDsync.
 *
 * Revision 2.1  1999/02/23 12:44:07  sacbase
 * new release made
 *
 * Revision 1.3  1998/08/07 15:28:27  dkr
 * SCHEDwlsegVar added
 *
 * Revision 1.2  1998/08/06 20:57:53  dkr
 * SCHEDwlseg, SCHEDwlsegvar must traverse NEXT-nodes
 *
 * Revision 1.1  1998/06/18 14:37:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   schedule.c
 *
 * prefix: SCHED
 *
 * description:
 *
 *   This file implements a syntax tree traversal that reorganizes scheduling
 *   information. Existing scheduling information is optional and originates
 *   from applications of the wlcomp pragma in the source code. It is either
 *   tied to with-loops or to segments.
 *
 *   During this traversal, all scheduling specifications outside spmd-functions
 *   are removed. Within spmd-functions existing segment scheduling information
 *   is checked for suitability and missing schedulings are supplied on the basis
 *   of an inference scheme. All scheduling information of with-loops within
 *   a single synchronisation block is gathered, combined, checked, and tied
 *   to the synchronisation block. Missing scheduling information is again
 *   supplied by applying an inference scheme.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "scheduling.h"

/******************************************************************************
 *
 * function:
 *   SCHsched_t MakeDefaultSchedulingConstSegment()
 *   SCHsched_t MakeDefaultSchedulingVarSegment()
 *   SCHsched_t MakeDefaultSchedulingWithloop()
 *
 * description:
 *
 *   These functions generate default schedulings for the three different
 *   scheduling cases. Their implementation is responsible for defining
 *   which kind of scheduling actually is the default and what the default
 *   arguments are.
 *
 ******************************************************************************/

static SCHsched_t
MakeDefaultSchedulingConstSegment ()
{
    SCHsched_t sched;

    DBUG_ENTER ("MakeDefaultSchedulingConstSegment");

    sched = SCHMakeScheduling ("Block");

    DBUG_RETURN (sched);
}

static SCHsched_t
MakeDefaultSchedulingVarSegment ()
{
    SCHsched_t sched;

    DBUG_ENTER ("MakeDefaultSchedulingVarSegment");

    sched = SCHMakeScheduling ("BlockVar");

    DBUG_RETURN (sched);
}

static SCHsched_t
MakeDefaultSchedulingWithloop ()
{
    SCHsched_t sched;

    DBUG_ENTER ("MakeDefaultSchedulingWithloop");

    sched = SCHMakeScheduling ("Static");

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   SCHsched_t InferSchedulingConstSegment(node *wlseg, node *arg_info)
 *
 * description:
 *
 *   This function defines the inference strategy for the scheduling of
 *   constant segments.
 *
 ******************************************************************************/
static SCHsched_t
InferSchedulingConstSegment (node *wlseg, node *arg_info)
{
    SCHsched_t sched;

    DBUG_ENTER ("InferSchedulingConstSegment");

    sched = MakeDefaultSchedulingConstSegment ();

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   SCHsched_t InferSchedulingVarSegment(node *wlsegvar, node *arg_info)
 *
 * description:
 *
 *   This function defines the inference strategy for the scheduling of
 *   variable segments.
 *
 ******************************************************************************/
static SCHsched_t
InferSchedulingVarSegment (node *wlsegvar, node *arg_info)
{
    SCHsched_t sched;

    DBUG_ENTER ("InferSchedulingVarSegment");

    sched = MakeDefaultSchedulingVarSegment ();

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   SCHsched_t InferSchedulingWithloop(node *sync, node *arg_info)
 *
 * description:
 *
 *   This function defines the inference strategy for the scheduling of
 *   with-loops.
 *
 ******************************************************************************/
static SCHsched_t
InferSchedulingWithloop (node *sync, node *arg_info)
{
    SCHsched_t sched;

    DBUG_ENTER ("InferSchedulingWithloop");

    sched = MakeDefaultSchedulingWithloop ();

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   node *SCHEDwlseg(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   sched_tab traversal function for N_WLseg nodes.
 *
 *   This function assures that each segment within an spmd-function has
 *   a scheduling specification while all segments outside of spmd-functions
 *   do not have scheduling specifications.
 *
 ******************************************************************************/
node *
SCHEDwlseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHEDwlseg");

    if (arg_info == NULL) {
        /*
         * Here, we are not within an spmd-function, so schedulings derived from wlcomp
         * pragmas must be removed.
         */
        if (WLSEG_SCHEDULING (arg_node) != NULL) {
            WLSEG_SCHEDULING (arg_node)
              = SCHRemoveScheduling (WLSEG_SCHEDULING (arg_node));
        }
    } else {
        /*
         * Here, we are within an spmd-function, so if no scheduling is already present,
         * the inference strategy is used. Otherwise a scheduling derived froma wlcomp
         * pragma is checked for suitability for constant segments.
         */
        if (WLSEG_SCHEDULING (arg_node) == NULL) {
            WLSEG_SCHEDULING (arg_node)
              = InferSchedulingConstSegment (arg_node, arg_info);
        } else {
            SCHCheckSuitabilityConstSeg (WLSEG_SCHEDULING (arg_node));
        }
    }

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = Trav (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHEDwlsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   sched_tab traversal function for N_WLsegVar nodes.
 *
 *   This function assures that each segment within an spmd-function has
 *   a scheduling specification while all segments outside of spmd-functions
 *   do not have scheduling specifications.
 *
 ******************************************************************************/
node *
SCHEDwlsegVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHEDwlsegVar");

    if (arg_info == NULL) {
        /*
         * Here, we are not within an spmd-function, so schedulings derived from wlcomp
         * pragmas must be removed.
         */
        if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
            WLSEGVAR_SCHEDULING (arg_node)
              = SCHRemoveScheduling (WLSEGVAR_SCHEDULING (arg_node));
        }
    } else {
        /*
         * Here, we are within an spmd-function, so if no scheduling is already present,
         * the inference strategy is used. Otherwise a scheduling derived froma wlcomp
         * pragma is checked for suitability for constant segments.
         */
        if (WLSEGVAR_SCHEDULING (arg_node) == NULL) {
            WLSEGVAR_SCHEDULING (arg_node)
              = InferSchedulingVarSegment (arg_node, arg_info);
        } else {
            SCHCheckSuitabilityVarSeg (WLSEGVAR_SCHEDULING (arg_node));
        }
    }

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = Trav (WLSEGVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHEDsync(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   sched_tab traversal function for N_sync nodes.
 *
 *   This function assures that each synchronisation block has a scheduling
 *   specification. The with-loops within a single synchronisation block are
 *   traversed and their scheduling specifications originating from the wlcomp
 *   pragma are examined, checked for compatibility, and combined. The scheduling
 *   specifications tied to the with-loops are removed. If no scheduling
 *   specifications are present, the inference strategy is used.
 *
 ******************************************************************************/
node *
SCHEDsync (node *arg_node, node *arg_info)
{
    node *block;
    node *assign;
    node *let;
    node *with;

    DBUG_ENTER ("SCHEDsync");

    block = SYNC_REGION (arg_node);
    DBUG_ASSERT ((NODE_TYPE (block) == N_block),
                 "N_block expected, but another kind of node found");

    assign = BLOCK_INSTR (block);
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "N_assign expected, but another kind of node found");

    /* traverse N_assigns */
    if (assign != NULL) {
        let = ASSIGN_INSTR (assign);
        DBUG_ASSERT ((NODE_TYPE (let) == N_let),
                     "N_let expected, but another kind of node found");

        with = LET_EXPR (let);

        if (NODE_TYPE (with) == N_Nwith2) {

            /* this is an old assertion when there was no if ... */
            DBUG_ASSERT ((NODE_TYPE (with) == N_Nwith2),
                         "N_Nwith2 expected, but another kind of node found");

            if (NWITH2_SCHEDULING (with) != NULL) {
                /*
                 *  Somebody has annotated a pragma,
                 *  check if this one is suitable for this with-loop.
                 */
                SCHCheckSuitabilityWithloop (NWITH2_SCHEDULING (arg_node));
            } else {
                NWITH2_SCHEDULING (with) = InferSchedulingWithloop (arg_node, arg_info);
            }
        } else {
            /*
             *  No schedules for other constructs.
             */
        }

        assign = ASSIGN_NEXT (assign);
    }
    /*
     if (SYNC_SCHEDULING( arg_node) == NULL) {
       SYNC_SCHEDULING( arg_node) = InferSchedulingSyncblock( arg_node, arg_info);
     } else {
       SCHCheckSuitabilitySyncblock( SYNC_SCHEDULING( arg_node));
     }
   */

    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHEDnwith2(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   This function is used to remove all scheduling specifications tied to
 *   with-loops originating from wlcomp pragmas. The respective information
 *   has already been moved to N_sync nodes where required.
 *
 ******************************************************************************/
node *
SCHEDnwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHEDnwith2");

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (arg_node) = SCHRemoveScheduling (NWITH2_SCHEDULING (arg_node));
    }

    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), NULL);
    }
    /*
     * Here, arg_info is not propagated because all scheduling specifications must
     * be removed from nested with-loops. The NULL pointer is used to mark this
     * behaviour.
     */

    DBUG_RETURN (arg_node);
}
