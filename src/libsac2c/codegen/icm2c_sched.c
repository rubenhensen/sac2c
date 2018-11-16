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

#include "icm2c_basic.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "free.h"
#include "str.h"
#include "memory.h"

#ifdef BEtest
#define LIBIfree(x)                                                                      \
    x;                                                                                   \
    free (x)
#define MEMmalloc(x) malloc (x)
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void InitializeBoundaries( int dim, char **vararg)
 *
 * description:
 *   this funtion sets the boundaries of the withloop
 *   for all dimensions sparing sched_dim
 *   sched_dim[i]==1 : Taskselector sets value for this dimension
 *
 ******************************************************************************/

static void
InitializeBoundaries (int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **sched_dim = vararg + 3 * dim;
    int i;

    DBUG_ENTER ();

    for (i = 0; i < dim; i++) {
        INDENT;
        if (!atoi (sched_dim[i])) {
            fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                     lower_bound[i]);
            INDENT;
            fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                     upper_bound[i]);
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *    void TaskSelectorInit( int sched_id, char *ts_name, int ts_dims,
 *                           unsigned int ts_arg_num, char **ts_args,int dim,
 *                           char **vararg)
 *
 * description:
 *
 *   till now factoring is the only Taskselector who needs this function
 *   Arguments:
 *   ts_name    : taskselctor name
 *   ts_dims    : number of used dimensions by the taskselector
 *   ts_arg_num : number of arguments of the taskselector
 *   ts_args    : parameters of the taskselector
 *
 ******************************************************************************/
static void
TaskSelectorInit (int sched_id, char *ts_name, int ts_dims, unsigned int ts_arg_num,
                  char **ts_args, int dim, char **vararg)
{

    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **sched_dims = vararg + 3 * dim;
    int tasks_on_dim, i;

    DBUG_ENTER ();

    if (STReq (ts_name, "Factoring")) {
        tasks_on_dim = (-1);
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
        fprintf (global.outfile, "SAC_MT_SCHEDULER_TS_%s_INIT(%d,%s,%s);\n", ts_name,
                 sched_id, lower_bound[tasks_on_dim], upper_bound[tasks_on_dim]);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void TaskSelector( int sched_id, char *ts_name, int ts_dims,
 *                      unsigned int ts_arg_num, char **ts_args,int dim, char **vararg,
 *                      char *taskid, char *worktodo)
 *
 * description:
 *   this function divides the given withloop (dim,vararg) with thetaskselector
 *   ts_name into tasks for multithreaded computation.
 *   next_taskid is the number of the
 *   next task, which should be computated. Please call InitializeBoundaries()
 *   before using this function.
 *
 *   other arguments:
 *   ts_name    : taskselctor name
 *   ts_dims    : number of used dimensions by the taskselector
 *   ts_arg_num : number of arguments of the taskselector
 *   ts_args    : parameters of the taskselector
 *
 ******************************************************************************/

static void
TaskSelector (int sched_id, char *ts_name, int ts_dims, unsigned int ts_arg_num, char **ts_args,
              int dim, char **vararg, char *taskid, char *worktodo)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    char **sched_dims = vararg + 3 * dim;
    int *tasks_on_dim;
    int i, pos;
    unsigned int j;

    DBUG_ENTER ();

    tasks_on_dim = (int *)MEMmalloc (ts_dims * sizeof (int));

    pos = 0;
    for (i = 0; i < dim; i++) {
        if (atoi (sched_dims[i])) {
            DBUG_ASSERT (pos < ts_dims, "Too many dimensions for Taskselector");
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
    fprintf (global.outfile, "SAC_MT_SCHEDULER_TS_%s( %d,", ts_name, sched_id);

    for (i = 0; i < ts_dims; i++) {
        fprintf (global.outfile, "%d,", tasks_on_dim[i]);
    }
    for (i = 0; i < ts_dims; i++) {
        fprintf (global.outfile, "%s,", lower_bound[tasks_on_dim[i]]);
    }
    for (i = 0; i < ts_dims; i++) {
        fprintf (global.outfile, "%s,", upper_bound[tasks_on_dim[i]]);
    }
    for (i = 0; i < ts_dims; i++) {
        fprintf (global.outfile, "%s,", unrolling[tasks_on_dim[i]]);
    }
    for (j = 0; j < ts_arg_num; j++) {
        fprintf (global.outfile, "%s,", ts_args[j]);
    }

    fprintf (global.outfile, "%s, %s);\n", taskid, worktodo);

    tasks_on_dim = MEMfree (tasks_on_dim);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define MT_SCHEDULER_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BEGIN

    for (i = 0; i < dim; i++) {
        INDENT;

        if (global.backend == BE_distmem && i == 0) {
            fprintf (global.outfile, "if (SAC_WL_IS_DISTRIBUTED) {\n");

            global.indent++;
            INDENT;

            fprintf (global.outfile,
                     "SAC_WL_MT_SCHEDULE_START( %d) = SAC_DISTEM_DET_DIM0_START( "
                     "SAC_WL_DIST_DIM0_SIZE, %s, %s);\n",
                     i, lower_bound[i], upper_bound[i]);
            INDENT;

            fprintf (global.outfile,
                     "SAC_WL_MT_SCHEDULE_STOP( %d) = SAC_DISTEM_DET_DIM0_STOP( "
                     "SAC_WL_DIST_DIM0_SIZE, %s, %s);\n",
                     i, lower_bound[i], upper_bound[i]);

            global.indent--;
            INDENT;

            fprintf (global.outfile, "} else {\n");

            global.indent++;
            INDENT;
        }

        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                 lower_bound[i]);
        INDENT;

        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                 upper_bound[i]);

        if (global.backend == BE_distmem && i == 0) {
            global.indent--;
            INDENT;

            fprintf (global.outfile, "}\n");
        }
    }

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_END

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_INIT

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define MT_SCHEDULER_Block_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_BEGIN

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_Block_DIM0( %s, %s, %s);\n",
             lower_bound[0], upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                 lower_bound[i]);
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                 upper_bound[i]);
    }

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Block_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Block_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_END

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Block_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Block_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_INIT

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVar_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_BEGIN

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_BlockVar_DIM0( %s, %s, %s);\n",
             lower_bound[0], upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                 lower_bound[i]);
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                 upper_bound[i]);
    }

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockVar_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVar_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_END

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockVar_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVar_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVar_INIT

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_BlockDist_BEGIN( int sched_id, int dim,
 *                                                char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockDist_END( int sched_id, int dim,
 *                                              char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockDist_INIT( int sched_id, int dim,
 *                                               char **vararg)
 *
 * description:
 *   These two ICMs implement the scheduling for constant segments
 *   called "BlockDist".
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space along the outermost dimension upon the available processors.
 *   Blocking is not considered!
 *   Unrolling is not considered!
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_BlockDist_BEGIN (int sched_id, int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    int i;

    DBUG_ENTER ();

#define MT_SCHEDULER_BlockDist_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockDist_BEGIN

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_Block_DIM0( %s, %s, %s);\n",
             lower_bound[0], upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                 lower_bound[i]);
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                 upper_bound[i]);
    }

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockDist_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockDist_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockDist_END

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockDist_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockDist_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockDist_INIT

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_BlockVarDist_BEGIN( int sched_id, int dim,
 *                                                   char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockVarDist_END( int sched_id, int dim,
 *                                                 char **vararg)
 *   void ICMCompileMT_SCHEDULER_BlockVarDist_INIT( int sched_id, int dim,
 *                                                  char **vararg)
 *
 * description:
 *   These two ICMs implement the scheduling for variable segments
 *   called "BlockVarDist".
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space along the outermost dimension upon the available processors.
 *   Blocking is not considered!
 *   Unrolling is not considered!
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_BlockVarDist_BEGIN (int sched_id, int dim, char **vararg)
{
    char **lower_bound = vararg;
    char **upper_bound = vararg + dim;
    char **unrolling = vararg + 2 * dim;
    int i;

    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVarDist_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVarDist_BEGIN

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_BlockVar_DIM0( %s, %s, %s);\n",
             lower_bound[0], upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_START( %d) = %s;\n", i,
                 lower_bound[i]);
        INDENT;
        fprintf (global.outfile, "SAC_WL_MT_SCHEDULE_STOP( %d) = %s;\n", i,
                 upper_bound[i]);
    }

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockVarDist_END (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVarDist_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVarDist_END

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_BlockVarDist_INIT (int sched_id, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_BlockVarDist_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BlockVarDist_INIT

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Static_BEGIN( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Static_END( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Static_INIT( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *
 * description:
 *   These three ICMs implement one scheduling for withloops
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space with the strategy specified for the taskselector.
 *   These tasks will be computed in cyclic order
 *
 *   Arguments:
 *   ts_name    : taskselctor name
 *   ts_dims    : number of used dimensions by the taskselector
 *   ts_arg_num : number of arguments of the taskselector
 *   ts_args    : parameters of the taskselector
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Static_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                     unsigned int ts_arg_num, char **ts_args, int dim,
                                     char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Static_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_BEGIN

    INDENT;
    fprintf (global.outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;

    fprintf (global.outfile, "SAC_MT_SCHEDULER_Static_FIRST_TASK("
                             "SAC_MT_taskid);\n");

    InitializeBoundaries (dim, vararg);

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (global.outfile, " while (SAC_MT_worktodo) {\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Static_END (int sched_id, char *ts_name, int ts_dims,
                                   unsigned int ts_arg_num, char **ts_args, int dim, char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Static_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_END

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_Static_NEXT_TASK(SAC_MT_taskid);\n");

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (global.outfile, "}\n");
    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Static_INIT (int sched_id, char *ts_name, int ts_dims,
                                    unsigned int ts_arg_num, char **ts_args, int dim,
                                    char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Static_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Static_INIT

    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Self_BEGIN( int sched_id, char *first_task,
 *                 char *ts_name, int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                 int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Self_END( int sched_id, char *first_task,
 *                 char *ts_name, int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                 int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Self_INIT( int sched_id, char *first_task,
 *                 char *ts_name, int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                 int dim, char **vararg)
 *
 *
 * description:
 *   These three ICMs implement one scheduling for withloops
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space with the strategy specified for the taskselector and gives each
 *   Thread after the computation of one task one new to computate
 *   (Selfscheduling).
 *
 *   Arguments:
 *   ts_name    : taskselctor name
 *   ts_dims    : number of used dimensions by the taskselector
 *   ts_arg_num : number of arguments of the taskselector
 *   ts_args    : parameters of the taskselector
 *
 *   first_task : FirstStatic    : first task will be choosen static
 *                                 without using the taskqueue
 *                FirstDynamic   : first task will be taken out of
 *                                 taskqueue
 *                FirstAutomatic : if sched_id==0 then FirstStatic
 *                                 else FirstDynamic
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Self_BEGIN (int sched_id, char *first_task, char *ts_name,
                                   int ts_dims, unsigned int ts_arg_num, char **ts_args, int dim,
                                   char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Self_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_BEGIN

    DBUG_ASSERT ((STReq (first_task, "SACl_FirstStatic")
                  || STReq (first_task, "SACl_FirstDynamic")
                  || STReq (first_task, "SACl_FirstAutomatic")),
                 "Scheduler Self needs one of the following strategies"
                 " for his first task: FirstStatic, FirstDynamic,"
                 " FirstAutomatic");

    INDENT;
    fprintf (global.outfile, "int SAC_MT_taskid,SAC_MT_worktodo;\n");
    INDENT;
    if (STReq (first_task, "SACl_FirstAutomatic")) {
        if (sched_id > 0) {
            fprintf (global.outfile,
                     "SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(%d,SAC_MT_taskid);\n",
                     sched_id);
        } else {
            fprintf (global.outfile,
                     "SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(%d,SAC_MT_taskid);\n",
                     sched_id);
        }
    }
    if (STReq (first_task, "SACl_FirstStatic"))
        fprintf (global.outfile,
                 "SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(%d,SAC_MT_taskid);\n",
                 sched_id);
    if (STReq (first_task, "SACl_FirstDynamic"))
        fprintf (global.outfile,
                 "SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(%d,SAC_MT_taskid);\n",
                 sched_id);

    InitializeBoundaries (dim, vararg);

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");
    INDENT;

    fprintf (global.outfile, " while (SAC_MT_worktodo) {\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Self_END (int sched_id, char *first_task, char *ts_name,
                                 int ts_dims, unsigned int ts_arg_num, char **ts_args, int dim,
                                 char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Self_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_END

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_Self_NEXT_TASK(%d,SAC_MT_taskid);\n",
             sched_id);
    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_worktodo");

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Self_INIT (int sched_id, char *first_task, char *ts_name,
                                  int ts_dims, unsigned int ts_arg_num, char **ts_args, int dim,
                                  char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Self_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Self_INIT

    DBUG_ASSERT ((STReq (first_task, "SACl_FirstStatic")
                  || STReq (first_task, "SACl_FirstDynamic")
                  || STReq (first_task, "SACl_FirstAutomatic")),
                 "Scheduler Self needs one of the following strategies"
                 " for his first task: FirstStatic, FirstDynamic,"
                 " FirstAutomatic");

    INDENT;
    if (STReq (first_task, "SACl_FirstDynamic"))
        fprintf (global.outfile, "SAC_MT_SCHEDULER_SET_TASKS(%d,0);\n", sched_id);

    if (STReq (first_task, "SACl_FirstAutomatic")) {
        if (sched_id == 0) {
            INDENT;
            fprintf (global.outfile, "SAC_MT_TASK(%d,0)=SAC_MT_LOCAL_THREADS();\n",
                     sched_id);
        } else {
            fprintf (global.outfile, "SAC_MT_SCHEDULER_SET_TASKS(%d,0);\n", sched_id);
        }
    }
    if (STReq (first_task, "SACl_FirstStatic") == 0) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_TASK(%d,0)=SAC_MT_LOCAL_THREADS();\n", sched_id);
    }

    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

    INDENT;
    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Affinity_BEGIN( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Affinity_END( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *   void ICMCompileMT_SCHEDULER_Affinity_INIT( int sched_id, char *ts_name,
 *                               int ts_dims, unsigned int ts_arg_num, char **ts_args,
 *                               int dim, char **vararg)
 *
 *
 * description:
 *   These three ICMs implement one scheduling for withloops
 *
 *   This scheduling is based on affinity scheduling, which
 *   realizes a form of loadbalancing by stealing tasks from the thread,
 *   which has computated the smallest number of tasks.
 *
 * Caution:
 *   this scheduling can only work together with the taskselector Even
 *   till now
 *
 *   Arguments:
 *   ts_name    : taskselctor name
 *   ts_dims    : number of used dimensions by the taskselector
 *   ts_arg_num : number of arguments of the taskselector
 *   ts_args    : parameters of the taskselector
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Affinity_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                       unsigned int ts_arg_num, char **ts_args, int dim,
                                       char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Affinity_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_BEGIN

    DBUG_ASSERT (STReq (ts_name, "Even"),
                 "Please use Affinity only with Taskselector Even");

    INDENT;
    fprintf (global.outfile, "int SAC_MT_taskid, "
                             "SAC_MT_worktodo,SAC_MT_affinitydummy;\n");

    InitializeBoundaries (dim, vararg);

    INDENT;
    fprintf (global.outfile,
             "SAC_MT_SCHEDULER_Affinity_FIRST_TASK(%d,%d, SAC_MT_taskid, "
             "SAC_MT_worktodo);\n",
             sched_id, atoi (ts_args[0]));

    INDENT;
    fprintf (global.outfile, " while (SAC_MT_worktodo) {\n");

    TaskSelector (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg,
                  "SAC_MT_taskid", "SAC_MT_affinitydummy");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Affinity_END (int sched_id, char *ts_name, int ts_dims,
                                     unsigned int ts_arg_num, char **ts_args, int dim,
                                     char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Affinity_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_END

    INDENT;
    fprintf (global.outfile,
             "SAC_MT_SCHEDULER_Affinity_NEXT_TASK( "
             "%d ,%d, SAC_MT_taskid, SAC_MT_worktodo);\n",
             sched_id, atoi (ts_args[0]));
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

void
ICMCompileMT_SCHEDULER_Affinity_INIT (int sched_id, char *ts_name, int ts_dims,
                                      unsigned int ts_arg_num, char **ts_args, int dim,
                                      char **vararg)
{
    DBUG_ENTER ();

#define MT_SCHEDULER_Affinity_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Affinity_INIT

    DBUG_ASSERT (STReq (ts_name, "Even"),
                 "Please use Affinity only with Taskselector Even");

    INDENT;
    fprintf (global.outfile, "SAC_MT_SCHEDULER_Affinity_INIT(%d,%d);\n", sched_id,
             atoi (ts_args[0]));

    TaskSelectorInit (sched_id, ts_name, ts_dims, ts_arg_num, ts_args, dim, vararg);

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
