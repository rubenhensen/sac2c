/*
 *
 * $Log$
 * Revision 1.6  2000/01/28 13:51:02  jhs
 * Traversal is finished, pragma-schedulings can be used.
 *
 * Revision 1.5  2000/01/26 17:25:31  dkr
 * type of traverse-function-table changed.
 *
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
#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *ScheduleInit(node *arg_node, node *arg_info)
 *
 * description:
 *   Call this function to run mini-phase schedule-init.
 *   Expects an N_fundef as arg_node!
 *
 ******************************************************************************/
node *
ScheduleInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    node *old_scheduling;
    int old_innerwls;

    DBUG_ENTER ("ScheduleInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "ScheduleInit expects a N_fundef as arg_node");

    DBUG_PRINT ("SCHIN", ("entering ScheduleInit"));

    old_tab = act_tab;
    act_tab = schin_tab;

    /* pushing arg_info information */
    old_scheduling = INFO_SCHIN_SCHEDULING (arg_info);
    old_innerwls = INFO_SCHIN_INNERWLS (arg_info);
    INFO_SCHIN_SCHEDULING (arg_info) = NULL;
    INFO_SCHIN_INNERWLS (arg_info) = NULL;

    DBUG_PRINT ("SCHIN", ("trav into fundef"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("SCHIN", ("trav from fundef"));

    /* poping arg_info_information */
    INFO_SCHIN_SCHEDULING (arg_info) = old_scheduling;
    INFO_SCHIN_INNERWLS (arg_info) = old_innerwls;

    act_tab = old_tab;

    DBUG_PRINT ("SCHIN", ("leaving ScheduleInit"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static int WithLoopIsWorthConcurrentExecution(node *withloop, ids *let_var)
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
 *   static int WithLoopIsAllowedConcurrentExecution(node *withloop)
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
 *   static SCHsched_t MakeDefaultSchedulingConstSegment()
 *   static SCHsched_t MakeDefaultSchedulingVarSegment()
 *   static SCHsched_t MakeDefaultSchedulingWithloop()
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
 *   static SCHsched_t InferSchedulingConstSegment(node *wlseg, node *arg_info)
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
 *   static SCHsched_t InferSchedulingVarSegment(node *wlsegvar, node *arg_info)
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
 *   node *SCHINassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Locks for let-assigments with a rhs with-loop,
 *   Such a with-loop will be checked if allowed and worth for mt and not a
 *   nested with-loop.
 *   In any case the with-loop will be traversed, to find nested pragmas
 *   in nested with-loops.
 *   The flag INFO_SCHIN_ALLOWED says whether a with-loop will be executed
 *   multithreaded.
 *   While traversing the with-loop the Segments will be scheduled.
 *
 ******************************************************************************/
node *
SCHINassign (node *arg_node, node *arg_info)
{
    node *let;
    int old_allowed;

    DBUG_ENTER ("SCHINassign");

    /* DBUG_PRINT( "SCHIN", ("assign reached")); */

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let) {
        /* it is a let */
        let = ASSIGN_INSTR (arg_node);
        /* DBUG_PRINT( "SCHIN", ("let found")); */

        if (NODE_TYPE (LET_EXPR (let)) == N_Nwith2) {
            /* DBUG_PRINT( "SCHIN", ("with found")); */
            old_allowed = INFO_SCHIN_ALLOWED (arg_info);
            if ((!(INFO_SCHIN_INNERWLS (arg_info)))
                && (WithLoopIsAllowedConcurrentExecution (LET_EXPR (let)))
                && (WithLoopIsWorthConcurrentExecution (LET_EXPR (let), LET_IDS (let)))) {
                DBUG_PRINT ("SCHIN", ("wl is allowed and worth mt"));
                INFO_SCHIN_ALLOWED (arg_info) = TRUE;
            } else {
                DBUG_PRINT ("SCHIN", ("wl is (inner or not-allowed-mt or not-worth-mt)"));
                INFO_SCHIN_ALLOWED (arg_info) = FALSE;
            }
            /*
             *  in any case the expr will be traversed to find annotated pragmas
             *  in inner-wls.
             */
            LET_EXPR (let) = Trav (LET_EXPR (let), arg_info);
            INFO_SCHIN_ALLOWED (arg_info) = old_allowed;
        } else {
            /* rhs is not a with: not of interest */
        }
    } else {
        /* assignment is not a let: not of interest */
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHINnwith2( node* arg_node, node *arg_info)
 *
 * description:
 *   Checks if a given pragma-scheduling (if exists) can be propagated, saves
 *   the information  about that in the arg_info, gives warnings if necessary,
 *   because of nested with-loops etc.
 *   Traverses Segments and Code of the with-loop afterwards to do propagation
 *   and Scheduling of the with-loop.
 *
 ******************************************************************************/
node *
SCHINnwith2 (node *arg_node, node *arg_info)
{
    int old_innerwls;

    DBUG_ENTER ("SCHINnwith2");

    /* The next line is used to test propagation */
    /* NWITH2_SCHEDULING( arg_node) = SCHMakeScheduling("Block"); */

    /*
     *  if a scheduling is annotated to a with-loop this scheduling will be
     *  propagated to the segments.
     *  Schedulings of inner with-loops will be ignored
     */
    if (INFO_SCHIN_ALLOWED (arg_info)) {
        if (!INFO_SCHIN_INNERWLS (arg_info)) {
            if (NWITH2_SCHEDULING (arg_node) != NULL) {
                DBUG_PRINT ("SCHIN", ("propagate with-loop scheduling"));
                INFO_SCHIN_SCHEDULING (arg_info) = NWITH2_SCHEDULING (arg_node);
            } else {
                DBUG_PRINT ("SCHIN", ("no pragma scheduling found"));
                INFO_SCHIN_SCHEDULING (arg_info) = NULL;
            }
        } else {
            if (NWITH2_SCHEDULING (arg_node) != NULL) {
                WARN (linenum, ("pragma-scheduling of inner-wl ignored"));
            }
            INFO_SCHIN_SCHEDULING (arg_info) = NULL;
        }
    } else {
        if (!INFO_SCHIN_INNERWLS (arg_info)) {
            WARN (linenum, ("pragma-scheduling of *mt not allowed* wl ignored"));
        } else {
            WARN (linenum, ("pragma-scheduling of *mt not allowed* inner-wl ignored"));
        }
    }

    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    old_innerwls = INFO_SCHIN_INNERWLS (arg_info);
    INFO_SCHIN_INNERWLS (arg_info) = TRUE;

    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    INFO_SCHIN_INNERWLS (arg_info) = old_innerwls;

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (arg_node) = SCHRemoveScheduling (NWITH2_SCHEDULING (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SCHINwlseg(node *arg_node, node *arg_info)
 *
 * description:
 *   schin_tab traversal function for N_WLseg nodes.
 *   Assures each segement of a top-level with-loop has scheduling
 *   specifications infered or propagated.
 *   Segments of nested with-loops and with-loops not allowed multithreaded
 *   execution will not be scheduled.
 *
 * attention:
 *   comment is referenced in SCHINwlsegVar
 *
 ******************************************************************************/
node *
SCHINwlseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHINwlseg");

    /*
     *  Only segments of top-level with-loops are supposed to have schedulings.
     */
    if ((INFO_SCHIN_ALLOWED (arg_info)) && (!INFO_SCHIN_INNERWLS (arg_info))) {
        /*
         *  If no scheduling is already present, the inference strategy is used.
         *  Otherwise a scheduling derived from wlcomp pragma is checked for
         *  suitability for constant segments.
         */
        if (WLSEG_SCHEDULING (arg_node) == NULL) {
            if (INFO_SCHIN_SCHEDULING (arg_info) == NULL) {
                DBUG_PRINT ("SCHIN", ("wlseg infer"));
                WLSEG_SCHEDULING (arg_node)
                  = InferSchedulingConstSegment (arg_node, arg_info);
            } else {
                DBUG_PRINT ("SCHIN", ("wlseg reuse"));
                WLSEG_SCHEDULING (arg_node)
                  = SCHCopyScheduling (INFO_SCHIN_SCHEDULING (arg_info));
                SCHCheckSuitabilityConstSeg (WLSEG_SCHEDULING (arg_node));
            }
        } else {
            if (INFO_SCHIN_SCHEDULING (arg_info) != NULL) {
                WARN (linenum,
                      ("wlseg cannot reuse propagation, already found scheduling"));
            }
            DBUG_PRINT ("SCHIN", ("wlseg suit?"));
            SCHCheckSuitabilityConstSeg (WLSEG_SCHEDULING (arg_node));
        }
    } else {
        /*
         *  further analysis of allowed and innerwls skipped,
         *  due to the impossibilty to annotate anything the segments
         */
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
 *   See comment on SCHINwlseg above.
 *
 ******************************************************************************/
node *
SCHINwlsegVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHINwlsegVar");

    /*
     *  Only segments of top-level with-loops are supposed to have schedulings.
     */
    if ((INFO_SCHIN_ALLOWED (arg_info)) && (!INFO_SCHIN_INNERWLS (arg_info))) {
        /*
         *  If no scheduling is already present, the inference strategy is used.
         *  Otherwise a scheduling derived from wlcomp pragma is checked for
         *  suitability for constant segments.
         */
        if (WLSEGVAR_SCHEDULING (arg_node) == NULL) {
            if (INFO_SCHIN_SCHEDULING (arg_info) == NULL) {
                DBUG_PRINT ("SCHIN", ("wlseg infer"));
                WLSEGVAR_SCHEDULING (arg_node)
                  = InferSchedulingVarSegment (arg_node, arg_info);
            } else {
                DBUG_PRINT ("SCHIN", ("wlseg reuse"));
                WLSEGVAR_SCHEDULING (arg_node)
                  = SCHCopyScheduling (INFO_SCHIN_SCHEDULING (arg_info));
                SCHCheckSuitabilityVarSeg (WLSEGVAR_SCHEDULING (arg_node));
            }
        } else {
            if (INFO_SCHIN_SCHEDULING (arg_info) != NULL) {
                WARN (linenum,
                      ("wlsegvar cannot reuse propagation, already found scheduling"));
            }
            DBUG_PRINT ("SCHIN", ("wlsegvar suit?"));
            SCHCheckSuitabilityVarSeg (WLSEGVAR_SCHEDULING (arg_node));
        }
    } else {
        /*
         *  further analysis of allowed and innerwls skipped,
         *  due to the impossibilty to annotate anything the segments
         */
    }

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = Trav (WLSEGVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
