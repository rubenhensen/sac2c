/*
 *
 * $Log$
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
 *
 */

/*****************************************************************************
 *
 * file:   libsac_mt.c
 *
 * prefix: SAC
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains routines and global identifiers required by the
 *   multi-threaded runtime system.
 *
 * remark
 *
 *   By means of the make tool, this source file is used to produce two
 *   different object files:
 *   1. libsac_mt.o    which contains the normal routines
 *   2. libsac_trmt.o  which contains variants of the normal routines along
 *                     with some extra routines and identifiers for tracing
 *                     program execution.
 *
 *   The compiler produces different object files by setting or unsetting
 *   the preprocessor flag TRACE.
 *
 *****************************************************************************/

#define _POSIX_C_SOURCE 199506L

#ifdef LINUX_X86
#define _MIT_POSIX_THREADS 1
#endif /* LINUX_X86 */

#include <pthread.h>
#include <stdio.h>

/*
 * Definition of trace macros.
 */

#ifdef TRACE

#include <stdarg.h>

#define TRACE_PRINT(msg) SAC_TRMT_Print msg

#else

#define TRACE_PRINT(msg)

#endif

/*
 * Definition of global variables.
 */

#ifdef TRACE

/*
 * Warning:
 *
 * At least on Solaris 2.5.x the static initialization of mutex locks causes
 * warnings upon compilation. The reason is that in the corresponding file
 * pthread.h, the definition of PTHREAD_MUTEX_INITIALIZER does NOT fit the
 * type definition of pthread_mutex_t.
 *
 * However, so far it works  :-)))
 */

pthread_mutex_t SAC_TRMT_array_memcnt_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t SAC_TRMT_hidden_memcnt_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_key_t SAC_TRMT_threadid_key;

const unsigned int SAC_TRMT_master_id = 0;

extern pthread_attr_t SAC_MT_thread_attribs;

extern void *SAC_MT_barrier;

extern volatile unsigned int SAC_MT_master_flag;

extern unsigned int SAC_MT_not_yet_parallel;

extern unsigned int SAC_MT_masterclass;

extern unsigned int SAC_MT_threads;

extern volatile unsigned int (*SAC_MT_spmd_function) (const unsigned int,
                                                      const unsigned int, unsigned int);

#else

pthread_attr_t SAC_MT_thread_attribs;

void *SAC_MT_barrier;

volatile unsigned int SAC_MT_master_flag = 0;

unsigned int SAC_MT_not_yet_parallel = 1;

unsigned int SAC_MT_masterclass;

unsigned int SAC_MT_threads;

volatile unsigned int (*SAC_MT_spmd_function) (const unsigned int, const unsigned int,
                                               unsigned int);

#endif /* TRACE */

/*
 * Definition of trace output function
 */

#ifdef TRACE

void
SAC_TRMT_Print (char *format, ...)
{
    va_list arg_p;
    unsigned int threadid;

    threadid = *((unsigned int *)pthread_getspecific (SAC_TRMT_threadid_key));

    pthread_mutex_lock (&output_lock);

    fprintf (stderr, "TR:%2u:-> ", threadid);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    pthread_mutex_unlock (&output_lock);
}

#endif

/*
 * Definition of thread control functions.
 */

static void
ThreadControl (void *arg)
{
    unsigned int worker_flag = 0;
    unsigned int i;
    unsigned int my_worker_class = ((unsigned int)arg) >> 17;
    const unsigned int my_thread_id = ((unsigned int)arg) & 0xFFFF;

#ifdef TRACE
    pthread_setspecific (SAC_TRMT_threadid_key, &my_thread_id);
#endif

    while (my_thread_id + my_worker_class >= SAC_MT_threads) {
        my_worker_class >>= 1;
    }

    TRACE_PRINT (
      ("This is worker thread #%u with class %u.", my_thread_id, my_worker_class));

    for (i = my_worker_class; i; i >>= 1) {

        TRACE_PRINT (
          ("Creating thread #%u with maximum class %u.", my_thread_id + i, i >> 1));

        pthread_create (NULL, &SAC_MT_thread_attribs, (void *(*)(void *))ThreadControl,
                        (void *)((i << 16) + (my_thread_id + i)));
    }

    for (;;) {
        TRACE_PRINT (("Worker thread #%u ready.", my_thread_id));

        while (worker_flag == SAC_MT_master_flag)
            ;
        worker_flag = SAC_MT_master_flag;

        worker_flag
          = (*SAC_MT_spmd_function) (my_thread_id, my_worker_class, worker_flag);
    }
}

#ifdef TRACE
void
SAC_TRMT_ThreadControl (void *arg)
#else
void
SAC_MT_ThreadControl (void *arg)
#endif
{
    unsigned int worker_flag = 0;
    unsigned int i;

#ifdef TRACE
    const unsigned int my_thread_id = 1;

    pthread_setspecific (SAC_TRMT_threadid_key, &my_thread_id);
#endif

    TRACE_PRINT (("This is worker thread #1 with class 0."));

    for (i = SAC_MT_masterclass; i > 1; i >>= 1) {

        TRACE_PRINT (("Creating thread #%u with maximum class %u.", i, i >> 1));

        pthread_create (NULL, &SAC_MT_thread_attribs, (void *(*)(void *))ThreadControl,
                        (void *)((i << 16) + i));
    }

    for (;;) {
        TRACE_PRINT (("Worker thread #1 ready."));

        while (worker_flag == SAC_MT_master_flag)
            ;
        worker_flag = SAC_MT_master_flag;

        worker_flag = (*SAC_MT_spmd_function) (1, 0, worker_flag);
    }
}
