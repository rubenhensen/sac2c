/*
 *
 * $Log$
 * Revision 3.15  2004/02/05 10:39:30  cg
 * Implementation for MT mode 1 (thread create/join) added.
 *
 * Revision 3.14  2003/09/17 17:22:59  dkr
 * a typo in trace message corrected
 *
 * Revision 3.13  2003/09/15 15:03:48  dkr
 * sac_misc.h included
 *
 * Revision 3.12  2003/04/29 11:55:05  cg
 * If the detach state or the scope of threads cannot be set correctly,
 * a warning is now issued instead of raiing an error and terminating
 * program execution.
 *
 * Revision 3.11  2003/04/14 15:16:51  sbs
 * cast from int into unsigned int added for safe comparisons.
 *
 * Revision 3.10  2003/03/21 13:17:10  sbs
 * emptied iff DISABLE_MT is set.
 *
 * Revision 3.9  2001/06/15 12:34:44  ben
 * SAC_MT_TS_Tasklock[] initialaztion added
 *
 * Revision 3.8  2001/05/21 12:41:17  ben
 * SAC_MT_Setup modified for initilization of SAC_MT_TASK_LOCKS depending on
 * SAC_MT_SET_NUM_SCHEDULERS
 *
 * Revision 3.7  2001/04/19 07:54:56  dkr
 * no changes done
 *
 * Revision 3.6  2001/04/12 12:18:37  sbs
 * two further volatiles added...
 *
 * Revision 3.5  2001/04/12 10:15:03  sbs
 * volatile declarations have to be added to external decls as well!!!!!!
 * otherwise new gcc version on bunasera fails 8-(((((
 *
 * Revision 3.4  2001/03/23 13:34:06  ben
 * in  SAC_MT_Setup SAC_MT_TASKLOCK initialization modified
 *
 * Revision 3.3  2001/03/22 17:37:47  ben
 * Initialisation of the Scheduler Mutexlocks SAC_MT_TASKLOCKS in SAC_MT_Setup
 *
 * Revision 3.2  2001/01/25 12:21:27  cg
 * Used unsigned long int instead of unsigned int for converting
 * pointers into numerical data.
 *
 * Revision 3.1  2000/11/20 18:02:45  sacbase
 * new release made
 *
 * Revision 2.4  2000/02/07 09:51:59  cg
 * Changed setting of semicolons in definitions and declarations of
 * mutex locks in order to avoid nasty warnings from cc.
 *
 * Revision 2.3  2000/01/17 16:25:58  cg
 * Reorganized implementation of the runtime system for
 * multi-threaded execution.
 *
 * Revision 2.2  1999/07/08 12:30:02  cg
 * File moved to new directory src/libsac.
 *
 * Revision 2.1  1999/02/23 12:43:40  sacbase
 * new release made
 *
 * Revision 1.12  1999/02/19 09:28:44  cg
 * bug fixed in creation of worker threads: a dummy threadid variable is
 * provided when calling pthread_create. This is not required under Solaris
 * but Linux does not like the NULL pointer here although the internal thread
 * ID is never to be used.
 * Support for MIT-threads discarded.
 *
 * Revision 1.11  1998/12/10 12:39:05  cg
 * Bug fixed in definition of _MIT_POSIX_THREADS.
 *
 * Revision 1.10  1998/12/07 10:00:11  cg
 * added #define _MIT_POSIX_THREADS to please Linux
 *
 * Revision 1.9  1998/10/23 13:14:56  cg
 * added explanation for some nasty warnings during compilation
 *
 * Revision 1.8  1998/07/10 15:21:15  cg
 * bug fixed in vararg macro usage
 *
 * Revision 1.7  1998/07/10 08:08:25  cg
 * header file stdarg.h used instead of varargs.h which is not
 * available under Linux.
 *
 * Revision 1.6  1998/07/02 09:27:04  cg
 * tracing capabilities improved
 *
 * Revision 1.5  1998/06/29 08:57:13  cg
 * added tracing facilities
 *
 */

/*****************************************************************************
 *
 * file:   mt.c
 *
 * prefix: SAC
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains routines and global identifiers required by the
 *   multi-threaded runtime system.
 *
 * remark:
 *
 *   By means of the make tool, this source file is used to produce two
 *   different object files:
 *   1. mt.o        which contains the normal routines
 *   2. mt_trace.o  which contains variants of the normal routines along
 *                     with some extra routines and identifiers for tracing
 *                     program execution.
 *
 *   The compiler produces different object files by setting or unsetting
 *   the preprocessor flag TRACE.
 *
 *****************************************************************************/

#include "config.h"

/*
 * In case we do not have mt available, we have to make sure this file
 * does not cause any problems (e.g. when running implicit dependency
 * inference system). Therefore, we render this file empty iff MT compilation
 * is disabled!
 */

#ifndef DISABLE_MT

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "sac_misc.h"
#include "sac_message.h"

#ifdef TRACE
#define SAC_DO_TRACE 1
#else
#define SAC_DO_TRACE 0
#endif /* TRACE */

#include "sac_trace.h"

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1
#include "sac_mt.h"
#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

/*
 * External declarations of global variables defined in the
 * compiled SAC program.
 */

extern volatile SAC_MT_barrier_t SAC_MT_barrier_space[];

extern pthread_mutex_t SAC_MT_Tasklock[];

extern pthread_mutex_t SAC_MT_TS_Tasklock[];

extern volatile int SAC_MT_Task[];

/*
 *  Definition of global variables.
 *
 *    These variables define the multi-threaded runtime system.
 */

#ifndef TRACE

/*
 * If we compile for mt_trace.o, we don't need the global variables since
 * these always remain in mt.o.
 */

pthread_key_t SAC_MT_threadid_key;

SAC_MT_DEFINE_LOCK (SAC_MT_output_lock)

SAC_MT_DEFINE_LOCK (SAC_MT_init_lock)

unsigned int SAC_MT_master_id = 0;

pthread_attr_t SAC_MT_thread_attribs;

volatile SAC_MT_barrier_t *SAC_MT_barrier;

volatile unsigned int SAC_MT_master_flag = 0;

volatile unsigned int SAC_MT_not_yet_parallel = 1;

unsigned int SAC_MT_masterclass;

unsigned int SAC_MT_threads;

pthread_t *SAC_MT1_internal_id;

volatile unsigned int (*SAC_MT_spmd_function) (const unsigned int, const unsigned int,
                                               unsigned int);

#endif /* TRACE */

/******************************************************************************
 *
 * function:
 *   void ThreadControl(void *arg)
 *
 * description:
 *
 *   This thread control function is executed by worker threads immediately
 *   after creation. After some thread specific initalizations, further worker
 *   threads are created depending on the worker class.
 *
 *   Afterwards, the worker thread enters a non-terminating loop where it waits
 *   at a barrier for work to be assigned and returns after finishing its task.
 *
 ******************************************************************************/

static void
ThreadControl (void *arg)
{
    volatile unsigned int worker_flag = 0; /* This volatile IS essential!!! */
    unsigned int i;
    unsigned int my_worker_class = ((unsigned long int)arg) >> 17;
    const unsigned int my_thread_id = ((unsigned long int)arg) & 0xFFFF;
    pthread_t tmp;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_init_lock);

    pthread_setspecific (SAC_MT_threadid_key, &my_thread_id);

    while (my_thread_id + my_worker_class >= SAC_MT_threads) {
        my_worker_class >>= 1;
    }

    SAC_TR_PRINT (
      ("This is worker thread #%u with class %u.", my_thread_id, my_worker_class));

    for (i = my_worker_class; i; i >>= 1) {

        SAC_TR_PRINT (
          ("Creating thread #%u with maximum class %u.", my_thread_id + i, i >> 1));

        if (0
            != pthread_create (&tmp, &SAC_MT_thread_attribs,
                               (void *(*)(void *))ThreadControl,
                               (void *)((unsigned long int)((i << 16)
                                                            + (my_thread_id + i))))) {

            SAC_RuntimeError ("Multi Thread Error: worker thread #%u failed to create"
                              "worker thread #%u",
                              my_thread_id, my_thread_id + i);
        }
    }

    SAC_MT_RELEASE_LOCK (SAC_MT_init_lock);

    for (;;) {
        SAC_TR_PRINT (("Worker thread #%u ready.", my_thread_id));

        while (worker_flag == SAC_MT_master_flag)
            ; /* wait here */
        worker_flag = SAC_MT_master_flag;

        worker_flag
          = (*SAC_MT_spmd_function) (my_thread_id, my_worker_class, worker_flag);
    }
}

/******************************************************************************
 *
 * function:
 *   void ThreadControlInitialWorker(void *arg)
 *
 * description:
 *
 *   This function is a special thread control function for the first worker
 *   thread. This thread creates additional threads on behalf of the master
 *   thread. As an optimization technique, the master thread does not create
 *   worker threads according to its thread class. Instead, the master thread
 *   only creates the first worker thread which in turn creates additional
 *   worker threads.
 *
 ******************************************************************************/

static void
ThreadControlInitialWorker (void *arg)
{
    volatile unsigned int worker_flag = 0; /* This volatile is essential!! */
    unsigned int i;
    pthread_t tmp;
    const unsigned int my_thread_id = 1;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_init_lock);

    pthread_setspecific (SAC_MT_threadid_key, &my_thread_id);

    SAC_TR_PRINT (("This is worker thread #1 with class 0."));

    for (i = SAC_MT_masterclass; i > 1; i >>= 1) {

        SAC_TR_PRINT (("Creating thread #%u with maximum class %u.", i, i >> 1));

        if (0
            != pthread_create (&tmp, &SAC_MT_thread_attribs,
                               (void *(*)(void *))ThreadControl,
                               (void *)((unsigned long int)((i << 16) + i)))) {

            SAC_RuntimeError ("Multi Thread Error: worker thread #1 failed to create"
                              "worker thread #%u",
                              i);
        }
    }

    SAC_MT_RELEASE_LOCK (SAC_MT_init_lock);

    for (;;) {
        SAC_TR_PRINT (("Worker thread #1 ready."));

        while (worker_flag == SAC_MT_master_flag)
            ; /* wait here */
        worker_flag = SAC_MT_master_flag;

        worker_flag = (*SAC_MT_spmd_function) (1, 0, worker_flag);
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_TR_SetupInitial( int argc, char *argv[],
 *                                unsigned int num_threads,
 *                                unsigned int max_threads)
 *   void SAC_MT_SetupInitial( int argc, char *argv[],
 *                             unsigned int num_threads,
 *                             unsigned int max_threads)
 *
 * description:
 *
 *   This function implements an initial setup of the runtime system for
 *   multi-threaded program execution. Here initializations are made which
 *   may not wait till worker thread creation. Basically, these are
 *   - the creation of the thread speicific data key which holds the thread ID,
 *   - the initialization of the  thread specific data for the master thread,
 *   - the evaluation of the -mt command line option.
 *
 ******************************************************************************/

#ifdef TRACE
void
SAC_MT_TR_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                        unsigned int max_threads)
#else
void
SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                     unsigned int max_threads)
#endif
{
    int i;

    SAC_TR_PRINT (("Creating thread specific data key to hold thread ID."));

    if (0 != pthread_key_create (&SAC_MT_threadid_key, NULL)) {
        SAC_RuntimeError ("Unable to create thread specific data key.");
    }

    SAC_TR_PRINT (("Initializing thread specific data for master thread."));

    if (0 != pthread_setspecific (SAC_MT_threadid_key, &SAC_MT_master_id)) {
        SAC_RuntimeError ("Unable to initialize thread specific data.");
    }

    if (num_threads == 0) {
        for (i = 1; i < argc - 1; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'm') && (argv[i][2] == 't')
                && (argv[i][3] == '\0')) {
                SAC_MT_threads = atoi (argv[i + 1]);
                break;
            }
        }
        if ((SAC_MT_threads <= 0) || (SAC_MT_threads > max_threads)) {
            SAC_RuntimeError ("Number of threads is unspecified or exceeds legal"
                              " range (1 to %d).\n"
                              "    Use option '-mt <num>'.",
                              max_threads);
        }
    } else {
        SAC_MT_threads = num_threads;
    }

    SAC_TR_PRINT (("Number of threads determined as %u.", SAC_MT_threads));
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_Setup( int cache_line_max, int barrier_offset,int num_schedulers)
 *   void SAC_MT_TR_Setup( int cache_line_max, int barrier_offset,int num_schedulers)
 *
 * description:
 *
 *   This function initializes the runtime system for multi-threaded
 *   program execution. The basic steps performed are
 *   - aligning the synchronization barrier data structure so that no two
 *     threads write to the same cache line,
 *   - Initialisation of the Scheduler Mutexlocks SAC_MT_TASKLOCKS
 *   - determining the thread class of the master thread,
 *   - creation and initialization of POSIX thread attributes,
 *   - creation of the initial worker thread.
 *
 ******************************************************************************/

#ifdef TRACE
void
SAC_MT_TR_Setup (int cache_line_max, int barrier_offset, int num_schedulers)
#else
void
SAC_MT_Setup (int cache_line_max, int barrier_offset, int num_schedulers)
#endif
{
    int i, n;

    SAC_TR_PRINT (("Aligning synchronization barrier data structure "
                   "to data cache specification."));

    if (cache_line_max > 0) {
        SAC_MT_barrier = (SAC_MT_barrier_t *)((char *)(SAC_MT_barrier_space + 1)
                                              - ((unsigned long int)SAC_MT_barrier_space
                                                 % barrier_offset));
    } else {
        SAC_MT_barrier = SAC_MT_barrier_space;
    }

    SAC_TR_PRINT (("Barrier base address is %p", SAC_MT_barrier));

    SAC_TR_PRINT (("Initializing Tasklocks."));

    for (n = 0; n < num_schedulers; n++) {
        pthread_mutex_init (&(SAC_MT_TS_TASKLOCK (n)), NULL);
        for (i = 0; (unsigned int)i < SAC_MT_threads; i++) {
            pthread_mutex_init (&(SAC_MT_TASKLOCK_INIT (n, i, num_schedulers)), NULL);
        }
    }

    SAC_TR_PRINT (("Computing thread class of master thread."));

    for (SAC_MT_masterclass = 1; SAC_MT_masterclass < SAC_MT_threads;
         SAC_MT_masterclass <<= 1) {
        SAC_MT_CLEAR_BARRIER (SAC_MT_masterclass);
    }

    SAC_MT_masterclass >>= 1;

    SAC_TR_PRINT (("Thread class of master thread is %d.", (int)SAC_MT_masterclass));

    if (SAC_MT_threads > 1) {
        pthread_t tmp;

        SAC_TR_PRINT (("Setting up POSIX thread attributes"));

        if (0 != pthread_attr_init (&SAC_MT_thread_attribs)) {
            SAC_RuntimeError ("Unable to initialize POSIX thread attributes");
        }

        if (0 != pthread_attr_setscope (&SAC_MT_thread_attribs, PTHREAD_SCOPE_SYSTEM)) {
            SAC_RuntimeWarning ("Unable to set POSIX thread attributes to "
                                "PTHREAD_SCOPE_SYSTEM.\n"
                                "Probably, your PTHREAD implementation does "
                                "not support system \n"
                                "scope threads, i.e. threads are likely not "
                                "to be executed in \n"
                                "parallel, but in time-sharing mode.");
        }

        if (0
            != pthread_attr_setdetachstate (&SAC_MT_thread_attribs,
                                            PTHREAD_CREATE_DETACHED)) {
            SAC_RuntimeWarning ("Unable to set POSIX thread attributes to "
                                "PTHREAD_CREATE_DETACHED."
                                "Probably, your PTHREAD implementation does "
                                "not support detached \n"
                                "threads. This may cause some runtime "
                                "overhead.");
        }

        SAC_TR_PRINT (("Creating worker thread #1 of class 0"));

        if (0
            != pthread_create (&tmp, &SAC_MT_thread_attribs,
                               (void *(*)(void *))ThreadControlInitialWorker, NULL)) {
            SAC_RuntimeError ("Unable to create (initial) worker thread #1");
        }
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT1_Setup( int cache_line_max, int barrier_offset,int num_schedulers)
 *   void SAC_MT1_TR_Setup( int cache_line_max, int barrier_offset,int num_schedulers)
 *
 * description:
 *
 *   This function initializes the runtime system for multi-threaded
 *   program execution. The basic steps performed are
 *   - aligning the synchronization barrier data structure so that no two
 *     threads write to the same cache line,
 *   - Initialisation of the Scheduler Mutexlocks SAC_MT_TASKLOCKS
 *   - determining the thread class of the master thread,
 *   - creation and initialization of POSIX thread attributes,
 *
 ******************************************************************************/

#ifdef TRACE
void
SAC_MT1_TR_Setup (int cache_line_max, int barrier_offset, int num_schedulers)
#else
void
SAC_MT1_Setup (int cache_line_max, int barrier_offset, int num_schedulers)
#endif
{
    int i, n;

    SAC_TR_PRINT (("Aligning synchronization barrier data structure "
                   "to data cache specification."));

    if (cache_line_max > 0) {
        SAC_MT_barrier = (SAC_MT_barrier_t *)((char *)(SAC_MT_barrier_space + 1)
                                              - ((unsigned long int)SAC_MT_barrier_space
                                                 % barrier_offset));
    } else {
        SAC_MT_barrier = SAC_MT_barrier_space;
    }

    SAC_TR_PRINT (("Barrier base address is %p", SAC_MT_barrier));

    SAC_TR_PRINT (("Initializing Tasklocks."));

    for (n = 0; n < num_schedulers; n++) {
        pthread_mutex_init (&(SAC_MT_TS_TASKLOCK (n)), NULL);
        for (i = 0; (unsigned int)i < SAC_MT_threads; i++) {
            pthread_mutex_init (&(SAC_MT_TASKLOCK_INIT (n, i, num_schedulers)), NULL);
        }
    }

    SAC_TR_PRINT (("Computing thread class of master thread."));

    for (SAC_MT_masterclass = 1; SAC_MT_masterclass < SAC_MT_threads;
         SAC_MT_masterclass <<= 1) {
        SAC_MT_CLEAR_BARRIER (SAC_MT_masterclass);
    }

    SAC_MT_masterclass >>= 1;

    SAC_TR_PRINT (("Thread class of master thread is %d.", (int)SAC_MT_masterclass));

    if (SAC_MT_threads > 1) {

        SAC_MT1_internal_id = (pthread_t *)malloc (SAC_MT_threads * sizeof (pthread_t));

        if (SAC_MT1_internal_id == NULL) {
            SAC_RuntimeError (
              "Unable to allocate memory for internal thread identifiers");
        }

        SAC_TR_PRINT (("Setting up POSIX thread attributes"));

        if (0 != pthread_attr_init (&SAC_MT_thread_attribs)) {
            SAC_RuntimeError ("Unable to initialize POSIX thread attributes");
        }

        if (0 != pthread_attr_setscope (&SAC_MT_thread_attribs, PTHREAD_SCOPE_SYSTEM)) {
            SAC_RuntimeWarning ("Unable to set POSIX thread attributes to "
                                "PTHREAD_SCOPE_SYSTEM.\n"
                                "Probably, your PTHREAD implementation does "
                                "not support system \n"
                                "scope threads, i.e. threads are likely not "
                                "to be executed in \n"
                                "parallel, but in time-sharing mode.");
        }
    }
}

static void
ThreadControl_mt0 (void *arg)
{
    const unsigned int my_thread_id = (unsigned int)arg;
    unsigned int worker_flag = 0;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_init_lock);

    pthread_setspecific (SAC_MT_threadid_key, &my_thread_id);

    SAC_TR_PRINT (("This is worker thread #%u.", my_thread_id));

    SAC_MT_RELEASE_LOCK (SAC_MT_init_lock);

    worker_flag = (*SAC_MT_spmd_function) (my_thread_id, 0, worker_flag);
}

#ifndef TRACE

void
SAC_MT1_StartWorkers ()
{
    unsigned int i;

    for (i = 1; i < SAC_MT_threads; i++) {
        if (0
            != pthread_create (&SAC_MT1_internal_id[i], &SAC_MT_thread_attribs,
                               (void *(*)(void *))ThreadControl_mt0, (void *)(i))) {

            SAC_RuntimeError ("Master thread failed to create worker thread #%u", i);
        }
    }
}

#endif

#endif /* DISABLE_MT */
