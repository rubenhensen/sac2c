/*
 *
 * $Log$
 * Revision 3.9  2001/06/05 09:53:22  ben
 * TaskSelector and Affinity_INIT modified
 *
 * Revision 3.8  2001/05/30 12:25:01  ben
 * TaskSelectorInit implemented
 * TaskSelector modified for using of Factoring
 *
 * Revision 3.7  2001/05/23 09:43:51  ben
 * some minor bugs in Self_INIT fixed
 *
 * Revision 3.6  2001/05/21 12:44:57  ben
 * SAC_MT_SCHEDULER_Self_INIT modified for discriminate between
 * FIRST_TASK_STATIC and FIRST_TASK_DYNAMIC
 *
 * Revision 3.5  2001/05/18 09:58:03  cg
 * #include <malloc.h> removed.
 *
 * Revision 3.4  2001/05/17 13:22:54  dkr
 * static strings instead of Malloc/Free used
 * (this is needed for BEtest!!!)
 *
 * Revision 3.3  2001/05/17 12:08:48  dkr
 * FREE, MALLOC eliminated
 *
 * Revision 3.2  2001/05/16 10:16:58  ben
 * SCHEDULER_Self modified, for dynamic or static choosen first task
 * depending on sched_id
 * Self_INIT and Affinity_INIT modified with the call of SET_TASKS
 *
 * Revision 3.1  2001/05/11 14:35:09  cg
 * Initial revision.
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_sched.c
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the definitions of C implemented ICMs which are
 *   concerned with loop scheduling in multithreaded execution.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

/*
 * task selection strategies
 */
#define TS_BLOCK 1
#define TS_FACTORING 2

/******************************************************************************
 *
 * function:
 *   void InitializeBoundaries( int dim, char **vararg)
 *
 * description:
 *   this funtion sets the boundaries of the withloop
 *   for all dimensions sparing sched_dim
 *
 *   sche_dim==-1 means there is no scheduling dimension
 *
 ******************************************************************************/

static void
InitializeBoundaries (int dim, char **vararg, int sched_dim)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    int i;

    DBUG_ENTER ("InitializeBoundaries");

    for (i = 0; i < dim; i++) {
        INDENT;
        if (i != sched_dim)
            fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i, lower_bound[i]);
        INDENT;
        if (i != sched_dim)
            fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i, upper_bound[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void TaskSelectorInit(int dim, char **vararg,
 *                    int strategy, int tasks_on_dim,
 *                    char *num_tasks, char *taskid,int sched_id)
 *
 * description:
 *
 *   implemented strategies:  (name / meaning of 'tasks_on_dim')
 *
 *     Even:    dimension on which the withloop will be divided into
 *               'num_tasks' tasks
 *
 ******************************************************************************/
static void
TaskSelectorInit (int dim, char **vararg, int strategy, int tasks_on_dim, char *num_tasks,
                  int sched_id)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;

    DBUG_ENTER ("TaskSelectorInit");

    switch (strategy) {
    case TS_BLOCK:
        DBUG_ASSERT ((tasks_on_dim >= 0) && (tasks_on_dim < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
        break;

    case TS_FACTORING:
        DBUG_ASSERT ((tasks_on_dim >= 0) && (tasks_on_dim < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
        INDENT;
        fprintf (outfile, "SAC_MT_SCHEDULER_TS_Factoring_INIT(%d,%s,%s);\n", sched_id,
                 lower_bound[tasks_on_dim], upper_bound[tasks_on_dim]);
        break;
    default:
        DBUG_ASSERT ((0), "unknown task selection strategy");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void TaskSelector( int dim, char **vararg,
 *                    int strategy, int tasks_on_dim,
 *                    char *num_tasks, char *taskid,char *worktodo,int sched_id)
 *
 * description:
 *   this function divides the given withloop (dim,vararg) with the strategy
 *   (strategy,strategy_param) into tasks for multithreaded computation.
 *   The number of tasks is num_tasks and next_taskid is the number of the
 *   next task, which should be computated. Please call InitializeBoundaries()
 *   before using this function.
 *
 *   implemented strategies:  (name / meaning of 'tasks_on_dim')
 *
 *     Even:    dimension on which the withloop will be divided into
 *               'num_tasks' tasks
 *
 ******************************************************************************/

static void
TaskSelector (int dim, char **vararg, int strategy, int tasks_on_dim, char *num_tasks,
              char *taskid, char *worktodo, int sched_id)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;

    DBUG_ENTER ("TaskSelector");

    switch (strategy) {
    case TS_BLOCK:
        DBUG_ASSERT ((tasks_on_dim >= 0) && (tasks_on_dim < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
        INDENT;
        fprintf (outfile, "SAC_MT_SCHEDULER_TS_Even(%d, %s, %s, %s, %s, %s, %s);\n",
                 tasks_on_dim, lower_bound[tasks_on_dim], upper_bound[tasks_on_dim],
                 unrolling[tasks_on_dim], num_tasks, taskid, worktodo);
        break;
    case TS_FACTORING:
        DBUG_ASSERT ((tasks_on_dim >= 0) && (tasks_on_dim < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
        INDENT;
        fprintf (outfile, "SAC_MT_SCHEDULER_TS_Factoring(%d,%d, %s, %s, %s, %s, %s);\n",
                 sched_id, tasks_on_dim, lower_bound[tasks_on_dim],
                 upper_bound[tasks_on_dim], num_tasks, taskid, worktodo);
        break;

    default:
        DBUG_ASSERT ((0), "unknown task selection strategy");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_ADJUST_SCHEDULER__OFFSET( char *array, int array_dim,
 *                                               int current_dim,
 *                                               char *lower, char *upper,
 *                                               char *unrolling)
 *
 * description:
 *
 *
 ******************************************************************************/

void
ICMCompileMT_ADJUST_SCHEDULER__OFFSET (char *array, int array_dim, int current_dim,
                                       char *lower, char *upper, char *unrolling)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_ADJUST_SCHEDULER__OFFSET");

#define MT_ADJUST_SCHEDULER
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_ADJUST_SCHEDULER

    INDENT;
    fprintf (outfile,
             "SAC_MT_ADJUST_SCHEDULER__OFFSET( %s, %d, %d, %s, %s, %s"
             ", (",
             array, array_dim, current_dim, lower, upper, unrolling);

    if (current_dim == (array_dim - 1)) {
        fprintf (outfile, "1");
    } else {
        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", array, current_dim + 1);

        for (i = (current_dim + 2); i < array_dim; i++) {
            fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", array, current_dim + i);
        }
    }

    fprintf (outfile, "));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_BEGIN( int sched_id, int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_END( int sched_id, int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_INIT( int sched_id, int dim, char **vararg)
 *
 * description:
 *   These two ICMs implement the default scheduling.
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_BEGIN (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BEGIN");

#define MT_SCHEDULER_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BEGIN

    /* -1 means there is no scheduling dimension */
    InitializeBoundaries (dim, vararg, -1);

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_END");

#define MT_SCHEDULER_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_END

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_INIT");

#define MT_SCHEDULER_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_INIT

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Block_BEGIN( int sched_id, int dim,
 *                                            char **vararg)
 *   void ICMCompileMT_SCHEDULER_Block_END( int sched_id, int dim,
 *                                          char **vararg)
 *   void ICMCompileMT_SCHEDULER_Block_INIT( int sched_id, int dim,
 *                                           char **vararg)
 *
 * description:
 *   These two ICMs implement the scheduling for constant segments
 *   called "Block".
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space along the outermost dimension upon the available processors.
 *   Blocking is not considered!
 *   Unrolling is not considered!
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Block_BEGIN (int sched_id, int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    int i;

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Block_BEGIN");

#define MT_SCHEDULER_Block_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_BEGIN

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Block_DIM0( %s, %s, %s);\n", lower_bound[0],
             upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i, lower_bound[i]);
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i, upper_bound[i]);
    }

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Block_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Block_END");

#define MT_SCHEDULER_Block_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_END

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Block_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Block_INIT");

#define MT_SCHEDULER_Block_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_INIT

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_BlockVar_BEGIN( int sched_id, int dim,
 *                                               char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockVar_END( int sched_id, int dim,
 *                                             char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockVar_INIT( int sched_id, int dim,
 *                                              char **vararg)
 *
 * description:
 *   These two ICMs implement the scheduling for variable segments
 *   called "BlockVar".
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space along the outermost dimension upon the available processors.
 *   Blocking is not considered!
 *   Unrolling is not considered!
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_BlockVar_BEGIN (int sched_id, int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    int i;

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BlockVar_BEGIN");

#define MT_SCHEDULER_BlockVar_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_BEGIN

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_BlockVar_DIM0( %s, %s, %s);\n", lower_bound[0],
             upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i, lower_bound[i]);
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i, upper_bound[i]);
    }

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_BlockVar_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BlockVar_END");

#define MT_SCHEDULER_BlockVar_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_END

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_BlockVar_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BlockVar_INIT");

#define MT_SCHEDULER_BlockVar_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_INIT

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Static_BEGIN( int sched_id, int tasks_per_thread,
 *                                             int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Static_END(int sched_id, int tasks_per_thread,
 *                                          int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Static_INIT(int sched_id, int tasks_per_thread,
 *                                           int dim, char **vararg)
 *
 * description:
 *   These two ICMs implement one scheduling for withloops
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space with the strategy specified for SelectTask (at the moment
 *   tasks_per_thread*Number of Threads Blocks on dimension 0.
 *   These Blcoks will be computated in cyclic order).
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Static_BEGIN (int sched_id, int tasks_per_thread, int dim,
                                     char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_BEGIN");

#define MT_SCHEDULER_Static_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_BEGIN

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    INDENT;
    fprintf (outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;

    fprintf (outfile,
             "SAC_MT_SCHEDULER_Static_FIRST_TASK("
             " %d, SAC_MT_taskid);\n",
             tasks_per_thread);

    InitializeBoundaries (dim, vararg, 0);

    TaskSelector (dim, vararg, TS_FACTORING, 0, numtasks, "SAC_MT_taskid",
                  "SAC_MT_worktodo", sched_id);

    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo){\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Static_END (int sched_id, int tasks_per_thread, int dim,
                                   char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_END");

#define MT_SCHEDULER_Static_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_END

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Static_NEXT_TASK(%d,SAC_MT_taskid);\n",
             tasks_per_thread);
    TaskSelector (dim, vararg, TS_FACTORING, 0, numtasks, "SAC_MT_taskid",
                  "SAC_MT_worktodo", sched_id);

    INDENT;
    fprintf (outfile, "}\n");
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Static_INIT (int sched_id, int tasks_per_thread, int dim,
                                    char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_INIT");

#define MT_SCHEDULER_Static_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_INIT

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    TaskSelectorInit (dim, vararg, TS_FACTORING, 0, numtasks, sched_id);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Self_BEGIN(int sched_id, int tasks_per_thread,
 *                                          int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Self_END(int sched_id, int tasks_per_thread,
 *                                        int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Self_INIT(int sched_id, int tasks_per_thread,
 *                                         int dim, char **vararg)
 *
 * description:
 *   These two ICMs implement one scheduling for withloops
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space with the strategy specified for SelectTask (at the moment
 *   tasks_per_thread* number of Threads Blocks on dimension 0) and gives each
 *   Thread after the computation of one Block one new to computate
 *   (Selfscheduling).
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Self_BEGIN (int sched_id, int tasks_per_thread, int dim,
                                   char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_BEGIN");

#define MT_SCHEDULER_Self_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_BEGIN

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    INDENT;
    fprintf (outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;
    if (sched_id > 0) {
        fprintf (outfile,
                 "SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(%d,%d,SAC_MT_taskid);\n",
                 sched_id, tasks_per_thread);
    } else {
        fprintf (outfile,
                 "SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(%d,%d,SAC_MT_taskid);\n",
                 sched_id, tasks_per_thread);
    }
    InitializeBoundaries (dim, vararg, 0);

    TaskSelector (dim, vararg, TS_FACTORING, 0, numtasks, "SAC_MT_taskid",
                  "SAC_MT_worktodo", sched_id);
    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo) {\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Self_END (int sched_id, int tasks_per_thread, int dim,
                                 char **vararg)
{
    char numtasks[100];
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_END");

#define MT_SCHEDULER_Self_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_END

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Self_NEXT_TASK(%d,%d,SAC_MT_taskid);\n", sched_id,
             tasks_per_thread);
    TaskSelector (dim, vararg, TS_FACTORING, 0, numtasks, "SAC_MT_taskid",
                  "SAC_MT_worktodo", sched_id);

    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Self_INIT (int sched_id, int tasks_per_thread, int dim,
                                  char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_INIT");

#define MT_SCHEDULER_Self_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_INIT

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_SET_TASKS(%d);\n", sched_id);

    if (sched_id == 0) {
        INDENT;
        fprintf (outfile, "SAC_MT_TASK(%d,0,SAC_SET_NUM_SCHEDULERS)=SAC_MT_THREADS();\n",
                 sched_id);
    }

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    TaskSelectorInit (dim, vararg, TS_FACTORING, 0, numtasks, sched_id);

    INDENT;
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Affinity_BEGIN(int sched_id, int tasks_per_thread,
 *                                              int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Affinity_END(int sched_id, int tasks_per_thread,
 *                                            int dim, char **vararg)
 *   void ICMCompileMT_SCHEDULER_Affinity_INIT(int sched_id, int tasks_per_thread,
 *                                             int dim, char **vararg)
 *
 * description:
 *   These two ICMs implement one scheduling for withloops
 *
 *   This scheduling is based on affinity scheduling, which
 *   realizes a form of loadbalancing by stealing tasks from the thread,
 *   which has computated the smallest number of tasks. Each Thread has
 *   param own tasks.
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Affinity_BEGIN (int sched_id, int tasks_per_thread, int dim,
                                       char **vararg)
{
    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Affinity_BEGIN");

#define MT_SCHEDULER_Affinity_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_BEGIN

    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    INDENT;
    fprintf (outfile, "int SAC_MT_taskid, SAC_MT_maxloadthread, SAC_MT_mintask, "
                      "SAC_MT_worktodo,SAC_MT_affinityDummy;\n");
    InitializeBoundaries (dim, vararg, 0);
    INDENT;
    fprintf (outfile,
             "SAC_MT_SCHEDULER_Affinity_FIRST_TASK(%d,%d, SAC_MT_taskid, "
             "SAC_MT_worktodo, SAC_MT_maxloadthread, SAC_MT_mintask);\n",
             sched_id, tasks_per_thread);
    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo){\n");
    TaskSelector (dim, vararg, TS_BLOCK, 0, numtasks, "SAC_MT_taskid",
                  "SAC_MT_affinityDummy", sched_id);

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Affinity_END (int sched_id, int tasks_per_thread, int dim,
                                     char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Affinity_END");

#define MT_SCHEDULER_Affinity_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_END

    INDENT;
    fprintf (outfile,
             "SAC_MT_SCHEDULER_Affinity_NEXT_TASK(%d ,%d, SAC_MT_taskid, "
             "SAC_MT_worktodo, SAC_MT_maxloadthread, SAC_MT_mintask);\n",
             sched_id, tasks_per_thread);
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Affinity_INIT (int sched_id, int tasks_per_thread, int dim,
                                      char **vararg)
{

    char numtasks[100];

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Affinity_INIT");

#define MT_SCHEDULER_Affinity_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_INIT

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Affinity_INIT(%d,%d);\n", sched_id,
             tasks_per_thread);
    sprintf (numtasks, "SAC_MT_THREADS()*%d", tasks_per_thread);
    TaskSelectorInit (dim, vararg, TS_BLOCK, 0, numtasks, sched_id);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}
