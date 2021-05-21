/*****************************************************************************
 *
 * file:   schedule.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is a specialised header file of the implementation of the multi-threading
 *   facilities focussing on scheduling and task selection.
 *
 *****************************************************************************/

#ifndef _SAC_SCHEDULE_H_
#define _SAC_SCHEDULE_H_

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

#if SAC_DO_MT_PTHREAD || SAC_DO_MT_LPEL || SAC_DO_MT_BEEHIVE

#include <pthread.h>
#include "runtime/essentials_h/rt_misc.h"

/*****************************************************************************/

/*
 * Macros for defining data structures used by scheduling disciplines
 */

/* The following variables will be materialized as fields in the struct hive. */
#define SAC_MT_SCHEDULER_FIELD_TASKLOCKS() pthread_mutex_t *SAC_MT_Tasklock
#define SAC_MT_SCHEDULER_FIELD_TASKS() volatile int *SAC_MT_Task
#define SAC_MT_SCHEDULER_FIELD_LAST_TASKS() volatile int *SAC_MT_LAST_Task
#define SAC_MT_SCHEDULER_FIELD_REST_ITERATIONS() volatile int *SAC_MT_rest_iterations
#define SAC_MT_SCHEDULER_FIELD_ACT_TASKSIZE() volatile int *SAC_MT_act_tasksize
#define SAC_MT_SCHEDULER_FIELD_LAST_TASKEND() volatile int *SAC_MT_last_taskend
#define SAC_MT_SCHEDULER_FIELD_TS_TASKLOCKS() pthread_mutex_t *SAC_MT_TS_Tasklock
#define SAC_MT_SCHEDULER_FIELD_TASKCOUNT() volatile int *SAC_MT_Taskcount

/*****************************************************************************/

/*
 * Macros for accessing data structures used by scheduling disciplines
 */

/* The index calculation for the 2D to 1D mapping:
 *      sched_id + (num_schedulers * local_thread_id)
 *
 * In case the number of schedulers is statically known:
 *      sched_id + (SAC_SET_NUM_SCHEDULERS * local_thread_id)
 */

#define SAC_MT_TASKLOCK(sched_id, th)                                                    \
    SAC_MT_HIVE ()->SAC_MT_Tasklock[sched_id + SAC_SET_NUM_SCHEDULERS * th]

/* This is used in the libsac C code for initialization. */
#define SAC_MT_TASKLOCK_INIT(hive, sched_id, th, num_sched)                              \
    hive->SAC_MT_Tasklock[sched_id + num_sched * th]

#define SAC_MT_TASK(sched_id, th)                                                        \
    SAC_MT_HIVE ()->SAC_MT_Task[sched_id + SAC_SET_NUM_SCHEDULERS * th]

#define SAC_MT_LAST_TASK(sched_id, th)                                                   \
    SAC_MT_HIVE ()->SAC_MT_LAST_Task[sched_id + SAC_SET_NUM_SCHEDULERS * th]

#define SAC_MT_REST_ITERATIONS(sched_id) SAC_MT_HIVE ()->SAC_MT_rest_iterations[sched_id]

#define SAC_MT_ACT_TASKSIZE(sched_id) SAC_MT_HIVE ()->SAC_MT_act_tasksize[sched_id]

#define SAC_MT_LAST_TASKEND(sched_id) SAC_MT_HIVE ()->SAC_MT_last_taskend[sched_id]

#define SAC_MT_TS_TASKLOCK(sched_id) SAC_MT_HIVE ()->SAC_MT_TS_Tasklock[sched_id]

#define SAC_MT_TASKCOUNT(sched_id) SAC_MT_HIVE ()->SAC_MT_Taskcount[sched_id]

/*****************************************************************************/

/*
 *  Definitions of macro-implemented ICMs for scheduling
 */

#define SAC_MT_ADJUST_SCHEDULER_BOUND(diff, bound, lower, upper, unrolling)              \
    {                                                                                    \
        (diff) = 0;                                                                      \
        if (((bound) > (lower)) && ((bound) < (upper))) {                                \
            (diff) = ((bound) - (lower)) % (unrolling);                                  \
                                                                                         \
            if (diff) {                                                                  \
                (diff) = (unrolling) - (diff);                                           \
                                                                                         \
                if ((bound) + (diff) >= (upper)) {                                       \
                    (diff) = (upper) - (bound);                                          \
                    (bound) = (upper);                                                   \
                } else {                                                                 \
                    (bound) += (diff);                                                   \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define SAC_MT_ADJUST_SCHEDULER__BEGIN(to_NT, dims, dim, lower, upper, unrolling)        \
    {                                                                                    \
        int diff_start, diff_stop;                                                       \
                                                                                         \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_start, SAC_WL_MT_SCHEDULE_START (dim),       \
                                       lower, upper, unrolling)                          \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_stop, SAC_WL_MT_SCHEDULE_STOP (dim), lower,  \
                                       upper, unrolling)                                 \
                                                                                         \
        SAC_TR_MT_PRINT (("Scheduler Adjustment: dim %d: %d -> %d", dim,                 \
                          SAC_WL_MT_SCHEDULE_START (dim),                                \
                          SAC_WL_MT_SCHEDULE_STOP (dim)));
/* Here is no closing bracket missing -> SAC_MT_ADJUST_SCHEDULER__END */

#define SAC_MT_ADJUST_SCHEDULER__OFFSET(off_NT, to_NT, dim)                              \
    SAC_ND_WRITE (off_NT, 0)                                                             \
      = SAC_ND_READ (off_NT, 0) + diff_start * SAC_WL_SHAPE_FACTOR (to_NT, dim);

#define SAC_MT_ADJUST_SCHEDULER__END(to_NT, dims, dim, lower, upper, unrolling) }

#define SAC_MT_SCHEDULER_Static_BEGIN()

#define SAC_MT_SCHEDULER_Static_END()

#define SAC_MT_SCHEDULER_Block_DIM0(lower, upper, unrolling)                             \
    {                                                                                    \
        const int iterations = (upper - lower) / unrolling;                              \
        const int iterations_per_thread                                                  \
          = (iterations / SAC_MT_LOCAL_THREADS ()) * unrolling;                          \
        const int iterations_rest = iterations % SAC_MT_LOCAL_THREADS ();                \
                                                                                         \
        if (iterations_rest                                                              \
            && (SAC_MT_SELF_LOCAL_ID () < (unsigned int)iterations_rest)) {              \
            SAC_WL_MT_SCHEDULE_START (0)                                                 \
              = lower + SAC_MT_SELF_LOCAL_ID () * (iterations_per_thread + unrolling);   \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + (iterations_per_thread + unrolling);      \
        } else {                                                                         \
            SAC_WL_MT_SCHEDULE_START (0)                                                 \
              = (lower + iterations_rest * unrolling)                                    \
                + SAC_MT_SELF_LOCAL_ID () * iterations_per_thread;                       \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + iterations_per_thread;                    \
        }                                                                                \
                                                                                         \
        SAC_TR_MT_PRINT (("Scheduler 'Block': dim 0: %d -> %d",                          \
                          SAC_WL_MT_SCHEDULE_START (0), SAC_WL_MT_SCHEDULE_STOP (0)));   \
    }

#define SAC_MT_SCHEDULER_BlockVar_DIM0(lower, upper, unrolling)                          \
    {                                                                                    \
        SAC_MT_SCHEDULER_Block_DIM0 (lower, upper, unrolling)                            \
    }

/*
 * TS_Even is the same as Block_DIM0 with two changes
 * first it gets the dimension for dividing tasks
 * second it divides in num_tasks*SAC_MT_LOCAL_THREADS() tasks
 */

#define SAC_MT_SCHEDULER_TS_Even(sched_id, tasks_on_dim, lower, upper, unrolling,        \
                                 num_tasks, taskid, worktodo)                            \
    {                                                                                    \
        const int number_of_tasks = num_tasks * SAC_MT_LOCAL_THREADS ();                 \
        const int iterations = (upper - lower) / unrolling;                              \
        const int iterations_per_thread = (iterations / number_of_tasks) * unrolling;    \
        const int iterations_rest = iterations % number_of_tasks;                        \
                                                                                         \
        SAC_WL_MT_SCHEDULE_START (tasks_on_dim) = 0;                                     \
        SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim) = 0;                                      \
                                                                                         \
        worktodo = (taskid < number_of_tasks);                                           \
        if (worktodo) {                                                                  \
            if (iterations_rest && (taskid < iterations_rest)) {                         \
                SAC_WL_MT_SCHEDULE_START (tasks_on_dim)                                  \
                  = lower + (iterations_per_thread + unrolling) * (taskid);              \
                SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                   \
                  = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + iterations_per_thread      \
                    + unrolling;                                                         \
            } else {                                                                     \
                SAC_WL_MT_SCHEDULE_START (tasks_on_dim)                                  \
                  = (lower + iterations_rest * unrolling)                                \
                    + taskid * iterations_per_thread;                                    \
                SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                   \
                  = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + iterations_per_thread;     \
            }                                                                            \
            SAC_TR_MT_PRINT (("'TS_Even': dim %d: %d -> %d, Task: %d", tasks_on_dim,     \
                              SAC_WL_MT_SCHEDULE_START (tasks_on_dim),                   \
                              SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim), taskid));          \
        }                                                                                \
    }

#define SAC_MT_SCHEDULER_TS_Factoring_INIT(sched_id, lower, upper)                       \
    {                                                                                    \
        SAC_MT_REST_ITERATIONS (sched_id) = (upper - lower);                             \
        SAC_MT_LAST_TASKEND (sched_id) = 0;                                              \
        SAC_MT_TASKCOUNT (sched_id) = 0;                                                 \
        SAC_MT_ACT_TASKSIZE (sched_id)                                                   \
          = ((SAC_MT_REST_ITERATIONS (sched_id)) / (2 * SAC_MT_LOCAL_THREADS ())) + 1;   \
    }

/*
 * TS_Factoring divides the With Loop in Blocks with decreasing size
 * every SAC_MT_LOCAL_THREADS tasks have the same size and then a new tasksize
 * will be computed
 */

#define SAC_MT_SCHEDULER_TS_Factoring(sched_id, tasks_on_dim, lower, upper, unrolling,   \
                                      taskid, worktodo)                                  \
    {                                                                                    \
        int tasksize;                                                                    \
        int restiter;                                                                    \
        SAC_WL_MT_SCHEDULE_START (tasks_on_dim) = 0;                                     \
        SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim) = 0;                                      \
                                                                                         \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TS_TASKLOCK (sched_id));                             \
                                                                                         \
        restiter = SAC_MT_REST_ITERATIONS (sched_id);                                    \
        worktodo = (restiter > 0);                                                       \
                                                                                         \
        if (worktodo) {                                                                  \
            if (SAC_MT_TASKCOUNT (sched_id) == SAC_MT_LOCAL_THREADS ()) {                \
                SAC_MT_ACT_TASKSIZE (sched_id)                                           \
                  = ((restiter) / (2 * SAC_MT_LOCAL_THREADS ())) + 1;                    \
                SAC_MT_TASKCOUNT (sched_id) = 0;                                         \
            }                                                                            \
            tasksize = SAC_MT_ACT_TASKSIZE (sched_id);                                   \
            (SAC_MT_TASKCOUNT (sched_id))++;                                             \
                                                                                         \
            (SAC_WL_MT_SCHEDULE_START (tasks_on_dim)) = SAC_MT_LAST_TASKEND (sched_id);  \
                                                                                         \
            if (restiter < tasksize) {                                                   \
                tasksize = restiter;                                                     \
            }                                                                            \
            restiter -= tasksize;                                                        \
                                                                                         \
            SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                       \
              = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + tasksize;                      \
                                                                                         \
            SAC_MT_ACT_TASKSIZE (sched_id) = tasksize;                                   \
            SAC_MT_REST_ITERATIONS (sched_id) = restiter;                                \
            SAC_MT_LAST_TASKEND (sched_id) = SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim);     \
                                                                                         \
            SAC_TR_MT_PRINT (("'TS_Factoring': dim %d: %d -> %d, Task: %d, Size: %d",    \
                              tasks_on_dim, SAC_WL_MT_SCHEDULE_START (tasks_on_dim),     \
                              SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim),                    \
                              SAC_MT_TASKCOUNT (sched_id) - 1,                           \
                              SAC_MT_ACT_TASKSIZE (sched_id)));                          \
        }                                                                                \
        SAC_MT_RELEASE_LOCK (SAC_MT_TS_TASKLOCK (sched_id));                             \
    }

/*
 * SET_TASKS initiziale the first max_task taskqueues
 */

#define SAC_MT_SCHEDULER_SET_TASKS(sched_id, max_task)                                   \
    {                                                                                    \
        int i;                                                                           \
                                                                                         \
        for (i = 0; i <= max_task; i++) {                                                \
            SAC_MT_TASK (sched_id, i) = 0;                                               \
        }                                                                                \
        SAC_TR_MT_PRINT (("SAC_MT_TASK set for sched_id %d", sched_id));                 \
    }

#define SAC_MT_SCHEDULER_Static_FIRST_TASK(taskid)                                       \
    {                                                                                    \
        taskid = SAC_MT_SELF_LOCAL_ID ();                                                \
    }

#define SAC_MT_SCHEDULER_Static_NEXT_TASK(taskid)                                        \
    {                                                                                    \
        taskid += SAC_MT_LOCAL_THREADS ();                                               \
    }

#define SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(sched_id, taskid)                        \
    {                                                                                    \
        taskid = SAC_MT_SELF_LOCAL_ID ();                                                \
        SAC_TR_MT_PRINT (("SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC"));                   \
    }

#define SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(sched_id, taskid)                       \
    {                                                                                    \
        SAC_MT_SCHEDULER_Self_NEXT_TASK (sched_id, taskid);                              \
        SAC_TR_MT_PRINT (                                                                \
          ("SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC task = %d", taskid));               \
    }

#define SAC_MT_SCHEDULER_Self_NEXT_TASK(sched_id, taskid)                                \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, 0));                             \
                                                                                         \
        taskid = SAC_MT_TASK (sched_id, 0);                                              \
        (SAC_MT_TASK (sched_id, 0))++;                                                   \
                                                                                         \
        SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, 0));                             \
    }

/*
 * Affinity_INIT initialize the taskqueues for each thread for the
 * affinity scheduling
 */
#define SAC_MT_SCHEDULER_Affinity_INIT(sched_id, tasks_per_thread)                       \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < SAC_MT_LOCAL_THREADS (); i++) {                                  \
            SAC_MT_TASK (sched_id, i) = i * tasks_per_thread;                            \
            SAC_MT_LAST_TASK (sched_id, i) = (i + 1) * tasks_per_thread;                 \
        }                                                                                \
    }

#define SAC_MT_SCHEDULER_Affinity_FIRST_TASK(sched_id, tasks_per_thread, taskid,         \
                                             worktodo)                                   \
    {                                                                                    \
        SAC_MT_SCHEDULER_Affinity_NEXT_TASK (sched_id, tasks_per_thread, taskid,         \
                                             worktodo)                                   \
    }

#define SAC_MT_SCHEDULER_Affinity_NEXT_TASK(sched_id, tasks_per_thread, taskid,          \
                                            worktodo)                                    \
    {                                                                                    \
        int queueid, maxloadthread, mintask;                                             \
        worktodo = 0;                                                                    \
        taskid = 0;                                                                      \
                                                                                         \
        /* first look if MYTHREAD has work to do */                                      \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_SELF_LOCAL_ID ()));       \
                                                                                         \
        if (SAC_MT_TASK (sched_id, SAC_MT_SELF_LOCAL_ID ())                              \
            < SAC_MT_LAST_TASK (sched_id, SAC_MT_SELF_LOCAL_ID ())) {                    \
            taskid = SAC_MT_TASK (sched_id, SAC_MT_SELF_LOCAL_ID ());                    \
            (SAC_MT_TASK (sched_id, SAC_MT_SELF_LOCAL_ID ()))++;                         \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_SELF_LOCAL_ID ()));   \
            worktodo = 1;                                                                \
        } else {                                                                         \
            /*                                                                           \
             * if there was no work to do, find the task, which has done, the            \
             * smallest work till now                                                    \
             */                                                                          \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_SELF_LOCAL_ID ()));   \
            maxloadthread = 0;                                                           \
            mintask = SAC_MT_LAST_TASK (sched_id, 0) - SAC_MT_TASK (sched_id, 0);        \
            for (queueid = 1; queueid < SAC_MT_LOCAL_THREADS (); queueid++)              \
                if (SAC_MT_LAST_TASK (sched_id, queueid)                                 \
                      - SAC_MT_TASK (sched_id, queueid)                                  \
                    > mintask) {                                                         \
                    mintask = SAC_MT_TASK (sched_id, queueid)                            \
                              - SAC_MT_LAST_TASK (sched_id, queueid);                    \
                    maxloadthread = queueid;                                             \
                }                                                                        \
                                                                                         \
            /* if there was a thread with work to do,get his next task */                \
            SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, maxloadthread));             \
            if (SAC_MT_TASK (sched_id, maxloadthread)                                    \
                < SAC_MT_LAST_TASK (sched_id, maxloadthread)) {                          \
                taskid = SAC_MT_LAST_TASK (sched_id, maxloadthread) - 1;                 \
                (SAC_MT_LAST_TASK (sched_id, maxloadthread))--;                          \
                worktodo = 1;                                                            \
            }                                                                            \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, maxloadthread));             \
        }                                                                                \
    }

/******************************************************************************/

#endif /* SAC_DO_MT_PTHREAD || SAC_DO_MT_LPEL || SAC_DO_MT_BEEHIVE */

#endif /* SAC_DO_MULTITHREAD */

#endif /* _SAC_SCHEDULE_H_ */
