/*****************************************************************************
 *
 * $Id$
 *
 * file:   annotate_scheduling.c
 *
 * prefix: MTAS
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
 *   is checked for suitability and missing schedulings are supplied on the
 *   basis of an inference scheme.
 *
 *****************************************************************************/

#include "annotate_scheduling.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "str.h"
#include "memory.h"

/**
 * INFO structure
 */

struct INFO {
    bool inparwl;
};

/**
 * INFO macros
 */

#define INFO_INPARWL(n) (n->inparwl)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INPARWL (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn MTASdoAnnotateScheduling
 *
 *  @brief
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTASdoAnnotateScheduling (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_mtas);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

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

static sched_t *
MakeDefaultSchedulingConstSegment (void)
{
    sched_t *sched;

    DBUG_ENTER ();

    sched = SCHmakeScheduling ("Block");

    DBUG_RETURN (sched);
}

static sched_t *
MakeDefaultSchedulingVarSegment (void)
{
    sched_t *sched;

    DBUG_ENTER ();

    sched = SCHmakeScheduling ("BlockVar");

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *InferSchedulingConstSegment(node *wlseg, info *arg_info)
 *
 * description:
 *   This function defines the inference strategy for the scheduling of
 *   constant segments.
 *
 ******************************************************************************/

static sched_t *
InferSchedulingConstSegment (node *wlseg, info *arg_info)
{
    sched_t *sched;

    DBUG_ENTER ();

    if (global.backend == BE_cudahybrid)
        sched = SCHmakeScheduling ("Static");
    else
        sched = MakeDefaultSchedulingConstSegment ();

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *InferSchedulingVarSegment(node *wlsegvar, info *arg_info)
 *
 * description:
 *   This function defines the inference strategy for the scheduling of
 *   variable segments.
 *
 ******************************************************************************/

static sched_t *
InferSchedulingVarSegment (node *wlsegvar, info *arg_info)
{
    sched_t *sched;

    DBUG_ENTER ();

    if (global.backend == BE_cudahybrid)
        sched = SCHmakeScheduling ("Static");
    else
        sched = MakeDefaultSchedulingVarSegment ();

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *InferTaskselConstSegment(node *wlseg, info *arg_info)
 *
 * description:
 *   This function defines the inference strategy for the task selection of
 *   constant segments.
 *
 ******************************************************************************/

static tasksel_t *
InferTaskselConstSegment (node *wlseg, info *arg_info)
{
    tasksel_t *tasksel;

    DBUG_ENTER ();

    if (global.backend == BE_cudahybrid)
        tasksel = SCHmakeTasksel ("Even", 0, global.min_parallel_size);
    else
        tasksel = WLSEG_TASKSEL (wlseg);

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *InferTaskselVarSegment(node *wlsegvar, info *arg_info)
 *
 * description:
 *   This function defines the inference strategy for the task selection of
 *   variable segments.
 *
 ******************************************************************************/

static tasksel_t *
InferTaskselVarSegment (node *wlsegvar, info *arg_info)
{
    tasksel_t *tasksel;

    DBUG_ENTER ();

    if (global.backend == BE_cudahybrid)
        tasksel = SCHmakeTasksel ("Even", 0, global.min_parallel_size);
    else
        tasksel = WLSEG_TASKSEL (wlsegvar);

    DBUG_RETURN (tasksel);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTASmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTASmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTASfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTASfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTASwith2(node *arg_node, info *arg_info)
 *
 * description:
 *   This function is used to remove all scheduling specifications tied to
 *   with-loops originating from wlcomp pragmas. The respective information
 *   has already been moved to N_sync nodes where required.
 *
 ******************************************************************************/

node *
MTASwith2 (node *arg_node, info *arg_info)
{
    bool was_in_par_wl;

    DBUG_ENTER ();

    was_in_par_wl = INFO_INPARWL (arg_info);

    INFO_INPARWL (arg_info) = WITH2_PARALLELIZE (arg_node);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    INFO_INPARWL (arg_info) = was_in_par_wl;

    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTASwlseg(node *arg_node, info *arg_info)
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
MTASwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INPARWL (arg_info)) {
        /*
         * Here, we are within a (to be) parallelised with-loop.
         * If no scheduling has been annotated via the wlcomp pragma,
         * an inference strategy is used.
         * Otherwise, the annotated scheduling is checked for suitability for
         * constant segments.
         */
        if (WLSEG_SCHEDULING (arg_node) == NULL) {
            if (WLSEG_ISDYNAMIC (arg_node)) {
                WLSEG_SCHEDULING (arg_node)
                  = InferSchedulingVarSegment (arg_node, arg_info);
                WLSEG_TASKSEL (arg_node) = InferTaskselVarSegment (arg_node, arg_info);
            } else {
                WLSEG_SCHEDULING (arg_node)
                  = InferSchedulingConstSegment (arg_node, arg_info);
                WLSEG_TASKSEL (arg_node) = InferTaskselConstSegment (arg_node, arg_info);
            }
        } else {
            if (WLSEG_ISDYNAMIC (arg_node)) {
                SCHcheckSuitabilityVarSeg (WLSEG_SCHEDULING (arg_node));
            } else {
                SCHcheckSuitabilityConstSeg (WLSEG_SCHEDULING (arg_node));
            }
        }
    } else {
        /*
         * Here, we are not within a (to be) parallelised with-loop.
         * Scheduling annotations from the wlcomp pragma are removed.
         */
        if (WLSEG_SCHEDULING (arg_node) != NULL) {
            WLSEG_SCHEDULING (arg_node)
              = SCHremoveScheduling (WLSEG_SCHEDULING (arg_node));
        }
        if (WLSEG_TASKSEL (arg_node) != NULL) {
            WLSEG_TASKSEL (arg_node) = SCHremoveTasksel (WLSEG_TASKSEL (arg_node));
        }
    }

    WLSEG_NEXT (arg_node) = TRAVopt (WLSEG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
