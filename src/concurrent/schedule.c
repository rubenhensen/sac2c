/*
 *
 * $Log$
 * Revision 3.6  2001/07/10 09:30:49  ben
 * SCHRemoveTasksel inserted
 *
 * Revision 3.5  2001/03/29 14:46:06  dkr
 * NWITH2_SCHEDULING removed
 *
 * Revision 3.4  2001/03/29 14:03:15  dkr
 * no changes done
 *
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
 *
 * description:
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

/******************************************************************************
 *
 * function:
 *   SCHsched_t InferSchedulingConstSegment(node *wlseg, node *arg_info)
 *
 * description:
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
 *   node *SCHEDwlseg(node *arg_node, node *arg_info)
 *
 * description:
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
        if (WLSEGX_TASKSEL (arg_node) != NULL) {
            WLSEGX_TASKSEL (arg_node) = SCHRemoveTasksel (WLSEGX_TASKSEL (arg_node));
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
        if (WLSEGX_TASKSEL (arg_node) != NULL) {
            WLSEGX_TASKSEL (arg_node) = SCHRemoveTasksel (WLSEGX_TASKSEL (arg_node));
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
 *
 ******************************************************************************/

node *
SCHEDsync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHEDsync");

    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHEDnwith2(node *arg_node, node *arg_info)
 *
 * description:
 *   This function is used to remove all scheduling specifications tied to
 *   with-loops originating from wlcomp pragmas. The respective information
 *   has already been moved to N_sync nodes where required.
 *
 ******************************************************************************/

node *
SCHEDnwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHEDnwith2");

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
