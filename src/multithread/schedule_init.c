/*
 *
 * $Log$
 * Revision 1.4  2000/01/26 15:07:00  jhs
 * Traversal of inner wl's now correct.
 *
 * Revision 1.3  2000/01/24 18:26:41  jhs
 * Comment added ...
 *
 * Revision 1.2  2000/01/24 18:24:21  jhs
 * Added some infrastructure ...
 *
 * Revision 1.1  2000/01/24 10:27:35  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   schedule_init.c
 *
 * prefix: SCHIN
 *
 * description:
 *   This file implements the traversal of function bodies to determine
 *   which withloops shall be executed concurrently and how they should be
 *   scheduled if executed concurrently.
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"

/******************************************************************************
 *
 * function:
 *   node *ScheduleInit(node *arg_node, node *arg_info)
 *
 * description:
 *   Expects an N_fundef as arg_node!
 *
 ******************************************************************************/
node *
ScheduleInit (node *arg_node, node *arg_info)
{
    funptr *old_tab;

    DBUG_ENTER ("ScheduleInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "ScheduleInit expects a N_fundef as arg_node");

    NOTE (("ScheduleInit not implemented yet"));

    old_tab = act_tab;
    act_tab = schin_tab;

    /* push info */
    INFO_SCHIN_SCHEDULING (arg_info) = NULL;
    INFO_SCHIN_INNERWLS (arg_info) = NULL;

    NOTE (("before"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    NOTE (("after"));
    act_tab = old_tab;
    /* pop info */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   int WithLoopIsWorthConcurrentExecution(node *withloop, ids *let_var)
 *
 * description:
 *   This function decides whether a with-loop is actually worth to be executed
 *   concurrenly. This is necessary because for small with-loops the most
 *   efficient way of execution is just sequential.
 *
 * attention:
 *   Each test whether a with-loop is worth to be executed concurrently
 *   has to follow a test, whether the with-loop is allowed to be executed
 *   concurrently (by WithLoopIsAllowedConcurrentExecution, see below).
 *
 ******************************************************************************/

static int
WithLoopIsWorthConcurrentExecution (node *withloop, ids *let_var)
{
    int res, i, size, target_dim;

    DBUG_ENTER ("WithLoopIsWorthConcurrentExecution");

    if ((NWITHOP_TYPE (NWITH2_WITHOP (withloop)) == WO_foldfun)
        || (NWITHOP_TYPE (NWITH2_WITHOP (withloop)) == WO_foldprf)) {
        res = TRUE;
    } else {
        target_dim = VARDEC_DIM (IDS_VARDEC (let_var));
        if (target_dim > 0) {
            size = 1;
            for (i = 0; i < target_dim; i++) {
                size *= VARDEC_SHAPE (IDS_VARDEC (let_var), i);
            }
            if (size < min_parallel_size) {
                res = FALSE;
            } else {
                res = TRUE;
            }
        } else {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int WithLoopIsAllowedConcurrentExecution(node *withloop)
 *
 * description:
 *   This function decides whether a with-loop is actually allowed to be
 *   executed concurrently.
 *
 * attention:
 *   Each test whether a with-loop is allowed to be executed concurrently
 *   should follow a test, whether the with-loop is worth to be executed
 *   concurrently (by WithLoopIsWorthConcurrentExecution, above).
 *
 ******************************************************************************/

static int
WithLoopIsAllowedConcurrentExecution (node *withloop)
{
    int res;
    node *withop;

    DBUG_ENTER ("WithLoopIsAllowedConcurrentExecution");

    withop = NWITH2_WITHOP (withloop);
    if ((NWITHOP_TYPE (withop) == WO_foldfun) || (NWITHOP_TYPE (withop) == WO_foldprf)) {
        if (max_sync_fold == 0) {
            res = FALSE;
        } else {
            res = TRUE;
        }
    } else {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

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
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
SCHINassign (node *arg_node, node *arg_info)
{
    node *let;

    DBUG_ENTER ("SCHINassign");

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let) {
        /* it is a let */
        let = ASSIGN_INSTR (arg_node);

        if (NODE_TYPE (LET_EXPR (let)) == N_Nwith2) {

            if ((WithLoopIsAllowedConcurrentExecution (LET_EXPR (let)))
                && (WithLoopIsWorthConcurrentExecution (LET_EXPR (let), LET_IDS (let)))) {
                NOTE (("wl is allowed and worth mt"));

                LET_EXPR (let) = Trav (LET_EXPR (let), arg_info);

            } else {
                NOTE (("wl is not allowed or not worth mt"));
                /* is a scheduling already annoteted? -> warning #### */
            }
        } else {
        }

    } else {
        /* it is not a let */
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
SCHINnwith2 (node *arg_node, node *arg_info)
{
    int old_innerwls;

    DBUG_ENTER ("SCHINnwith2");

    /*
     *  if a scheduling is annotated to a with-loop this scheduling will be
     *  propagated to the segments.
     *  Schedulings of inner with-loops will be ignoreddd
     */
    if (INFO_SCHIN_INNERWLS (arg_info) == FALSE) {
        if (NWITH2_SCHEDULING (arg_node) != NULL) {
            INFO_SCHIN_SCHEDULING (arg_info) = NWITH2_SCHEDULING (arg_info);
        } else {
            INFO_SCHIN_SCHEDULING (arg_info) = NULL;
        }
    } else {
        if (NWITH2_SCHEDULING (arg_node) != NULL) {
            WARN (linenum, ("pragma Scheduling of inner with-loop will be ignored"));
        }
        INFO_SCHIN_SCHEDULING (arg_info) = NULL;
    }

    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    old_innerwls = INFO_SCHIN_INNERWLS (arg_info);
    INFO_SCHIN_INNERWLS (arg_info) = TRUE;

    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    INFO_SCHIN_INNERWLS (arg_info) = old_innerwls;

    NOTE (("SCHINnwith2 reached, nothing done"));
    /* remove wl scheduling if exists */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHINwlseg(node *arg_node, node *arg_info)
 *
 * description:
 *   scinh_tab traversal function for N_WLseg nodes.
 *   Assures each segement of a top-level with-loop has scheduling
 *   specifications.
 *
 ******************************************************************************/
node *
SCHINwlseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHINwlseg");

    /*
     *  Only segments of top-level with-loops are supposed to have schedulings.
     */
    if (INFO_SCHIN_INNERWLS (arg_info) == FALSE) {
        /*
         *  If no scheduling is already present, the inference strategy is used.
         *  Otherwise a scheduling derived from wlcomp pragma is checked for
         *  suitability for constant segments.
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
 *   node *SCHINwlsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   schin_tab traversal function for N_WLsegVar nodes.
 *   Assures each segement of a top-level with-loop has scheduling
 *   specifications.
 *
 ******************************************************************************/
node *
SCHINwlsegVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHINwlsegVar");

    /*
     *  Only segments of top-level with-loops are supposed to have schedulings.
     */
    if (INFO_SCHIN_INNERWLS (arg_info) == FALSE) {
        /*
         *  If no scheduling is already present, the inference strategy is used.
         *  Otherwise a scheduling derived from wlcomp pragma is checked for
         *  suitability for constant segments.
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
