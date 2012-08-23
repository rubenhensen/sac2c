/** <!--********************************************************************-->
 *
 * @defgroup Prepare Distributed Scheduler
 *
 *
 *   This module rearranges the schedulers around the MT with-loops, to take
 *   into account the memory transfers. Additionally, the scheduler related ICMs
 *   are copied to the CUDA version of the with-loop.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file prepare_dist_scheduler.c
 *
 * Prefix: PDS
 *
 *****************************************************************************/
#include "prepare_dist_scheduler.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"

#define DBUG_PREFIX "PDS"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"

#include "str.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *schedule_begin;
    node *schedule_end;
    node *scheduler_begin;
    node *scheduler_end;
    node *mem_assigns;
    node *host2dist;
};

/*
 * INFO_SCHEDULEBEGIN   The ICM assignment for WL_SCHEDULE__BEGIN.
 *
 * INFO_SCHEDULEEND     The ICM assignment for WL_SCHEDULE__END.
 *
 * INFO_SCHEDULERBEGIN  The ICM assignment for MT_SCHEDULER_xxx_BEGIN.
 *
 * INFO_SCHEDULEREND    The ICM assignment for MT_SCHEDULER_xxx_END.
 *
 * INFO_MEMASSIGNS      N_assign chain of <dist2conc_rel> transfers on the MT
 *                      branch.
 *
 * INFO_HOST2DIST       N_assign chain of <host2dist> transfers on the MT
 *                      branch.
 *
 */

#define INFO_SCHEDULEBEGIN(n) (n->schedule_begin)
#define INFO_SCHEDULEEND(n) (n->schedule_end)
#define INFO_SCHEDULERBEGIN(n) (n->scheduler_begin)
#define INFO_SCHEDULEREND(n) (n->scheduler_end)
#define INFO_MEMASSIGNS(n) (n->mem_assigns)
#define INFO_HOST2DIST(n) (n->host2dist)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SCHEDULEBEGIN (result) = NULL;
    INFO_SCHEDULEEND (result) = NULL;
    INFO_SCHEDULERBEGIN (result) = NULL;
    INFO_SCHEDULEREND (result) = NULL;
    INFO_MEMASSIGNS (result) = NULL;
    INFO_HOST2DIST (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *PDSdoPrepareDistributedScheduler( node *syntax_tree)
 *
 *****************************************************************************/
node *
PDSdoPrepareDistributedScheduler (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_pds);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *PDSfundef( node *arg_node, info *arg_info)
 *
 * @brief Skip non-SPMD functions
 *
 *****************************************************************************/
node *
PDSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_MEMASSIGNS (arg_info) = NULL;
        INFO_SCHEDULERBEGIN (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PDSassign( node *arg_node, info *arg_info)
 *
 * @brief  Identifies scheduler statements from the MT branch and copies them
 *         to the CUDA branch. Additionally, moves memory transfers on the MT
 *         branch inside the scheduler.
 *
 *****************************************************************************/
node *
PDSassign (node *arg_node, info *arg_info)
{
    node *scheduler, *res, *stmt, *next;
    const char *icm_name;
    nodetype stmt_type;

    DBUG_ENTER ();

    stmt = ASSIGN_STMT (arg_node);
    stmt_type = NODE_TYPE (stmt);
    switch (stmt_type) {
    case N_icm:
        /* depending on the ICM being observed we do different things. Note that
         * this depends heavily on the distributed SPMDs having specific
         * instructions in a specific order.
         */
        icm_name = ICM_NAME (stmt);
        if (STReq (icm_name, "SCHED_START")) {
            /* This is the first statement on the CUDA branch that needs the scheduler.
             * We insert the scheduler_begin assignments before it. */
            scheduler = DUPdoDupNode (INFO_SCHEDULERBEGIN (arg_info));
            ASSIGN_NEXT (scheduler) = arg_node;
            res = DUPdoDupNode (INFO_SCHEDULEBEGIN (arg_info));
            ASSIGN_NEXT (res) = scheduler;
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        } else if (STReq (icm_name, "WL_SCHEDULE__BEGIN")) {
            /* This is the first scheduler statement on the MT branch. We save it.*/
            INFO_SCHEDULEBEGIN (arg_info) = arg_node;
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        } else if (STReq (icm_name, "WL_SCHEDULE__END")) {
            /* This is the last scheduler statement on the MT branch. We save it. */
            INFO_SCHEDULEEND (arg_info) = arg_node;
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        } else if (STReq (icm_name, "DIST_HOST2DIST_SPMD")) {
            /* This is the first of the concrete to distributed memory transfers on
             * the MT branch. We save the assignment chain in info, and stop
             * traversing deeper.*/
            INFO_HOST2DIST (arg_info) = arg_node;
            res = NULL;
        } else if (STRprefix ("MT_SCHEDULER_", icm_name)) {
            /* The MT scheduler ICMs vary according to the scheduler used, so we check
             * for them through the suffix and prefix. We save both the begin and end
             * ICMs in info.*/
            if (STRsuffix ("_BEGIN", icm_name)) {
                /* we insert any memory transfers after this statement, as they need the
                 * scheduler to operate properly. */
                INFO_SCHEDULERBEGIN (arg_info) = arg_node;
                next = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
                ASSIGN_NEXT (arg_node)
                  = TCappendAssign (INFO_MEMASSIGNS (arg_info), next);
                res = arg_node;
            } else if (STRsuffix ("_END", icm_name)) {
                INFO_SCHEDULEREND (arg_info) = arg_node;
                ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
                /* The updates to the distributed variables should be inside the
                 * scheduler loop. We know where these start from info, and since these
                 * are the last assignments in a conditional branch, we can use them
                 * directly here.
                 */
                res = TCappendAssign (INFO_HOST2DIST (arg_info), arg_node);
            } else {
                /* This is another scheduler statment, such as INIT. We just traverse.*/
                ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
                res = arg_node;
            }
        } else if (STReq ("DIST_DIST2HOST_REL", icm_name)
                   && INFO_SCHEDULERBEGIN (arg_info) == NULL) {
            /* Since we have not yet come across a scheduler_begin statement, as it is
             * NULL, we are still collecting memory transfers to place after the
             * scheduler starts on the MT branch. Append this transfer to the chain in
             * info. */
            next = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (arg_node) = NULL;
            INFO_MEMASSIGNS (arg_info)
              = TCappendAssign (INFO_MEMASSIGNS (arg_info), arg_node);
            res = TRAVopt (next, arg_info);
        } else {
            /* for any other ICM, we just traverse to the next. */
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        }
        break;
    case N_cond:
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        res = arg_node;
        break;
    case N_do:
        /* This is the availability loop of the CUDA SPMD branch. We insert the
         * end of scheduling ICMs after this assignment */
        scheduler = DUPdoDupNode (INFO_SCHEDULEEND (arg_info));
        ASSIGN_NEXT (scheduler) = ASSIGN_NEXT (arg_node);
        res = DUPdoDupNode (INFO_SCHEDULEREND (arg_info));
        ASSIGN_NEXT (res) = scheduler;
        ASSIGN_NEXT (arg_node) = res;
        res = arg_node;
        break;

    default:
        /* For anything else, just traverse the next assignment. */
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        res = arg_node;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *PDScond( node *arg_node, info *arg_info)
 *
 * @brief Traverse bottom-up, so we can identify the scheduler statements in the
 *        MT branch (the last one).
 *
 *****************************************************************************/
node *
PDScond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
