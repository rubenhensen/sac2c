/*
 *
 * $Log$
 * Revision 3.16  2001/06/27 14:33:39  ben
 * modified for cooperation with tasksel-pragma
 *
 * Revision 3.15  2001/06/27 08:56:06  ben
 * 2 compiler warnings for Self-Scheduling fixed
 *
 * Revision 3.14  2001/06/20 12:25:12  ben
 * Self modified for using parameter first_task
 *
 * Revision 3.13  2001/06/19 12:31:37  ben
 *  SCHEDULER_Self modified  with parameter first_task
 *
 * Revision 3.12  2001/06/15 12:32:43  ben
 * Taskselector gets now tasks_per_thread instead of num_tasks
 *
 * Revision 3.11  2001/06/12 11:04:59  ben
 * one minor bug removed
 *
 * Revision 3.10  2001/06/12 11:01:18  ben
 *  SAC_MT_SCHEDULER_Self called without tasks_per_thread now
 *
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
#define TS_EVEN 1
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
InitializeBoundaries (int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **sched_dim = vararg + 3 * dim;
    int i;

    DBUG_ENTER ("InitializeBoundaries");

    for (i = 0; i < dim; i++) {
        INDENT;
        if (!atoi (sched_dim[i])) {
            fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i, lower_bound[i]);
            INDENT;
            fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i, upper_bound[i]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void TaskSelectorInit(int dim, char **vararg,
 *                    int strategy, int tasks_on_dim,
 *                    int tasks_per_thread, char *taskid,int sched_id)
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
TaskSelectorInit (int sched_id, char *ts_name, int ts_dims, int ts_arg_num,
                  char **ts_args, int dim, char **vararg)
{

    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **sched_dims = vararg + 3 * dim;
    int tasks_on_dim, i;

    DBUG_ENTER ("TaskSelectorInit");

    if (strcmp (ts_name, "Factoring") == 0) {
        tasks_on_dim = -1;
        i = 0;
        while ((tasks_on_dim < 0) && (i < dim)) {
            if (atoi (sched_dims[i])) {
                tasks_on_dim = i;
            }
            i++;
        }
        DBUG_ASSERT ((tasks_on_dim >= 0) && (tasks_on_dim < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
        fprintf (outfile, "SAC_MT_SCHEDULER_TS_%s_INIT(%d,%s,%s);\n", ts_name, sched_id,
                 lower_bound[tasks_on_dim], upper_bound[tasks_on_dim]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void TaskSelector( int dim, char **vararg,
 *                    int strategy, int tasks_on_dim,
 *                    int tasks_per_thread, char *taskid,char *worktodo,int sched_id)
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
TaskSelector (int sched_id, char *ts_name, int ts_dims, int ts_arg_num, char **ts_args,
              int dim, char **vararg, char *taskid, char *worktodo)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    char **sched_dims = vararg + 3 * dim;
    int *tasks_on_dim;
    int i, pos;
    DBUG_ENTER ("TaskSelector");

    tasks_on_dim = Malloc (ts_dims * sizeof (int));

    pos = 0;
    for (i = 0; i < dim; i++) {
        if (atoi (sched_dims[i])) {
            DBUG_ASSERT ((pos < ts_dims), " Too many dimensions for Taskselector");
            tasks_on_dim[pos] = i;
            pos++;
        }
    }

    for (i = 0; i < ts_dims; i++) {
        DBUG_ASSERT ((tasks_on_dim[i] >= 0) && (tasks_on_dim[i] < dim),
                     "Task Distribution Dimension should be between 0 and"
                     " the dimension of the withloop");
    }
    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_TS_%s(%d,", ts_name, sched_id);

    for (i = 0; i < ts_dims; i++)
        fprintf (outfile, "%d,", tasks_on_dim[i]);
    for (i = 0; i < ts_dims; i++)
        fprintf (outfile, "%s,", lower_bound[tasks_on_dim[i]]);
    for (i = 0; i < ts_dims; i++)
        fprintf (outfile, "%s,", upper_bound[tasks_on_dim[i]]);
    for (i = 0; i < ts_dims; i++)
        fprintf (outfile, "%s,", unrolling[tasks_on_dim[i]]);
    for (i = 0; i < ts_arg_num; i++)
        fprintf (outfile, "%s,", ts_args[i]);

    fprintf (outfile, "%s, %s);\n", taskid, worktodo);

    tasks_on_dim = Free (tasks_on_dim);

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

    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;

    int i;

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BEGIN");

#define MT_SCHEDULER_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BEGIN

    for (i = 0; i < dim; i++) {
        INDENT;

        fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i, lower_bound[i]);
        INDENT;

        fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i, upper_bound[i]);
    }

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
ICMCompileMT_SCHEDULER_Static_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                     int ts_arg_num, char **ts_args, int dim,
                                     char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_BEGIN");

#define MT_SCHEDULER_Static_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_BEGIN

    INDENT;
    fprintf (outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;

    fprintf (outfile, "SAC_MT_SCHEDULER_Static_FIRST_TASK("
                      "SAC_MT_taskid);\n");

    InitializeBoundaries (dim, vararg);

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo){\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Static_END (int sched_id, char *ts_name, int ts_dims,
                                   int ts_arg_num, char **ts_args, int dim, char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_END");

#define MT_SCHEDULER_Static_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_END

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Static_NEXT_TASK(SAC_MT_taskid);\n");

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (outfile, "}\n");
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Static_INIT (int sched_id, char *ts_name, int ts_dims,
                                    int ts_arg_num, char **ts_args, int dim,
                                    char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Static_INIT");

#define MT_SCHEDULER_Static_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_INIT

    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

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
ICMCompileMT_SCHEDULER_Self_BEGIN (int sched_id, char *first_task, char *ts_name,
                                   int ts_dims, int ts_arg_num, char **ts_args, int dim,
                                   char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_BEGIN");

#define MT_SCHEDULER_Self_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_BEGIN

    DBUG_ASSERT (((strcmp (first_task, "SACl_FirstStatic") == 0)
                  || (strcmp (first_task, "SACl_FirstDynamic") == 0)
                  || (strcmp (first_task, "SACl_FirstAutomatic") == 0)),
                 "Scheduler Self needs one of the following strategies"
                 " for his first task: FirstStatic, FirstDynamic,"
                 " FirstAutomatic");

    INDENT;
    fprintf (outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;
    if (strcmp (first_task, "SACl_FirstAutomatic") == 0) {
        if (sched_id > 0) {
            fprintf (outfile,
                     "SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(%d,SAC_MT_taskid);\n",
                     sched_id);
        } else {
            fprintf (outfile,
                     "SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(%d,SAC_MT_taskid);\n",
                     sched_id);
        }
    }
    if (strcmp (first_task, "SACl_FirstStatic") == 0)
        fprintf (outfile, "SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(%d,SAC_MT_taskid);\n",
                 sched_id);
    if (strcmp (first_task, "SACl_FirstDynamic") == 0)
        fprintf (outfile, "SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(%d,SAC_MT_taskid);\n",
                 sched_id);

    InitializeBoundaries (dim, vararg);

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");
    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo) {\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Self_END (int sched_id, char *first_task, char *ts_name,
                                 int ts_dims, int ts_arg_num, char **ts_args, int dim,
                                 char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_END");

#define MT_SCHEDULER_Self_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_END

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Self_NEXT_TASK(%d,SAC_MT_taskid);\n", sched_id);
    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Self_INIT (int sched_id, char *first_task, char *ts_name,
                                  int ts_dims, int ts_arg_num, char **ts_args, int dim,
                                  char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Self_INIT");

#define MT_SCHEDULER_Self_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_INIT

    DBUG_ASSERT (((strcmp (first_task, "SACl_FirstStatic") == 0)
                  || (strcmp (first_task, "SACl_FirstDynamic") == 0)
                  || (strcmp (first_task, "SACl_FirstAutomatic") == 0)),
                 "Scheduler Self needs one of the following strategies"
                 " for his first task: FirstStatic, FirstDynamic,"
                 " FirstAutomatic");

    INDENT;
    if (strcmp (first_task, "SACl_FirstDynamic") == 0)
        fprintf (outfile, "SAC_MT_SCHEDULER_SET_TASKS(%d,0);\n", sched_id);

    if (strcmp (first_task, "SACl_FirstAutomatic") == 0) {
        if (sched_id == 0) {
            INDENT;
            fprintf (outfile, "SAC_MT_TASK(%d,0)=SAC_MT_THREADS();\n", sched_id);
        } else {
            fprintf (outfile, "SAC_MT_SCHEDULER_SET_TASKS(%d,0);\n", sched_id);
        }
    }
    if (strcmp (first_task, "SACl_FirstStatic") == 0) {
        INDENT;
        fprintf (outfile, "SAC_MT_TASK(%d,0)=SAC_MT_THREADS();\n", sched_id);
    }

    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

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
ICMCompileMT_SCHEDULER_Affinity_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                       int ts_arg_num, char **ts_args, int dim,
                                       char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Affinity_BEGIN");

#define MT_SCHEDULER_Affinity_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_BEGIN

    DBUG_ASSERT ((ts_args != NULL), " Please use Affinity only with Taskselector Even");

    INDENT;
    fprintf (outfile, "int SAC_MT_taskid, SAC_MT_maxloadthread, SAC_MT_mintask, "
                      "SAC_MT_worktodo,SAC_MT_affinitydummy;\n");
    InitializeBoundaries (dim, vararg);
    INDENT;
    fprintf (outfile,
             "SAC_MT_SCHEDULER_Affinity_FIRST_TASK(%d,%d, SAC_MT_taskid, "
             "SAC_MT_worktodo, SAC_MT_maxloadthread, SAC_MT_mintask);\n",
             sched_id, atoi (ts_args[0]));
    INDENT;
    fprintf (outfile, " while (SAC_MT_worktodo){\n");
    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_affinitydummy");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Affinity_END (int sched_id, char *ts_name, int ts_dims,
                                     int ts_arg_num, char **ts_args, int dim,
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
             sched_id, atoi (ts_args[0]));
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Affinity_INIT (int sched_id, char *ts_name, int ts_dims,
                                      int ts_arg_num, char **ts_args, int dim,
                                      char **vararg)
{

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Affinity_INIT");

#define MT_SCHEDULER_Affinity_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_INIT

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Affinity_INIT(%d,%d);\n", sched_id,
             atoi (ts_args[0]));
    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}
