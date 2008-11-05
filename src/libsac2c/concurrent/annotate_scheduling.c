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
 *   basis of an inference scheme. All scheduling information of with-loops
 *   within a single synchronisation block is gathered, combined, checked, and
 *   tied to the synchronisation block. Missing scheduling information is again
 *   supplied by applying an inference scheme.
 *
 *****************************************************************************/

#include "annotate_scheduling.h"

#include "dbug.h"
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
    int dummy;
};

/**
 * INFO macros
 */

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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

    DBUG_ENTER ("MTASdoAnnotateScheduling");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_mtas);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTASfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTASfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTASfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node),
                                         FUNDEF_ISSPMDFUN (arg_node) ? arg_info : NULL);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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
MakeDefaultSchedulingConstSegment ()
{
    sched_t *sched;

    DBUG_ENTER ("MakeDefaultSchedulingConstSegment");

    sched = SCHmakeScheduling ("Block");

    DBUG_RETURN (sched);
}

static sched_t *
MakeDefaultSchedulingVarSegment ()
{
    sched_t *sched;

    DBUG_ENTER ("MakeDefaultSchedulingVarSegment");

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

    DBUG_ENTER ("InferSchedulingConstSegment");

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

    DBUG_ENTER ("InferSchedulingVarSegment");

    sched = MakeDefaultSchedulingVarSegment ();

    DBUG_RETURN (sched);
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
    DBUG_ENTER ("MTASwlseg");

    if (arg_info == NULL) {
        /*
         * Here, we are not within an spmd-function, so schedulings derived from
         * wlcomp pragmas must be removed.
         */
        if (WLSEG_SCHEDULING (arg_node) != NULL) {
            WLSEG_SCHEDULING (arg_node)
              = SCHremoveScheduling (WLSEG_SCHEDULING (arg_node));
        }
        if (WLSEGX_TASKSEL (arg_node) != NULL) {
            L_WLSEGX_TASKSEL (arg_node, SCHremoveTasksel (WLSEGX_TASKSEL (arg_node)));
        }

    } else {
        /*
         * Here, we are within an spmd-function, so if no scheduling is already
         * present, the inference strategy is used. Otherwise a scheduling derived
         * froma wlcomp pragma is checked for suitability for constant segments.
         */
        if (WLSEG_SCHEDULING (arg_node) == NULL) {
            WLSEG_SCHEDULING (arg_node)
              = InferSchedulingConstSegment (arg_node, arg_info);
        } else {
            SCHcheckSuitabilityConstSeg (WLSEG_SCHEDULING (arg_node));
        }
    }

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = TRAVdo (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTASwlsegvar(node *arg_node, info *arg_info)
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
MTASwlsegvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTASwlsegvar");

    if (arg_info == NULL) {
        /*
         * Here, we are not within an spmd-function, so schedulings derived from
         * wlcomp pragmas must be removed.
         */
        if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
            WLSEGVAR_SCHEDULING (arg_node)
              = SCHremoveScheduling (WLSEGVAR_SCHEDULING (arg_node));
        }
        if (WLSEGX_TASKSEL (arg_node) != NULL) {
            L_WLSEGX_TASKSEL (arg_node, SCHremoveTasksel (WLSEGX_TASKSEL (arg_node)));
        }

    } else {
        /*
         * Here, we are within an spmd-function, so if no scheduling is already
         * present, the inference strategy is used. Otherwise a scheduling derived
         * from wlcomp pragma is checked for suitability for constant segments.
         */
        if (WLSEGVAR_SCHEDULING (arg_node) == NULL) {
            WLSEGVAR_SCHEDULING (arg_node)
              = InferSchedulingVarSegment (arg_node, arg_info);
        } else {
            SCHcheckSuitabilityVarSeg (WLSEGVAR_SCHEDULING (arg_node));
        }
    }

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = TRAVdo (WLSEGVAR_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("MTASwith2");

    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), NULL);
    }
    /*
     * Here, arg_info is not propagated because all scheduling specifications must
     * be removed from nested with-loops. The NULL pointer is used to mark this
     * behaviour.
     */

    DBUG_RETURN (arg_node);
}
