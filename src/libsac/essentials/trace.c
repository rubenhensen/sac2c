/*****************************************************************************
 *
 * file:   trace.c
 *
 * prefix: SAC_TR
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *   It provides global variables and function definitions for tracing
 *   program execution.
 *
 *   Upon multi-threaded execution access to global variables is protected
 *   by locks and trace information is preceded by the corresponding
 *   thread ID.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef MT
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_PTHREAD 1
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#define SAC_DO_PHM 1

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_PHM

int SAC_TR_array_memcnt = 0;
int SAC_TR_hidden_memcnt = 0;

SAC_MT_DEFINE_LOCK (SAC_TR_array_memcnt_lock)
SAC_MT_DEFINE_LOCK (SAC_TR_hidden_memcnt_lock)

/******************************************************************************
 *
 * function:
 *   void SAC_TR_Print(char *format, ...)
 *
 * description:
 *
 *   This function is used to print trace output to stderr.
 *
 *   The multi-threaded version of the trace print function assures single
 *   threaded execution through a mutex lock;
 *   the usual layout of the trace message is extended by the thread ID
 *   which is determined via thread specific global data ('SAC_MT_threadid_key').
 *
 ******************************************************************************/

#ifdef MT

void
SAC_TR_Print (char *format, ...)
{
    va_list arg_p;
    // unsigned int *thread_id_ptr;
    unsigned int thread_id;

    // thread_id_ptr = (unsigned int *) pthread_getspecific( SAC_MT_threadid_key);
    thread_id = SAC_HM_CurrentThreadId ();
    //   thread_id = SAC_Get_CurrentBee_GlobalID();

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (thread_id == SAC_HM_THREADID_INVALID) {
        fprintf (stderr, "TR:??:-> ");
    } else {
        fprintf (stderr, "TR:%2u:-> ", thread_id);
    }

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

#else /* MT */

void
SAC_TR_Print (char *format, ...)
{
    va_list arg_p;

    fprintf (stderr, "TR-> ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void SAC_TR_IncArrayMemcnt(int size)
 *   void SAC_TR_DecArrayMemcnt(int size)
 *
 * description:
 *
 *   These functions are used to increment and decrement the allocated array
 *   element counter and print the corresponding counter status information.
 *
 *   Under a multi-threaded execution scheme the counter is protected
 *   through a dedicated lock.
 *
 ******************************************************************************/

void
SAC_TR_IncArrayMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_array_memcnt_lock);
    SAC_TR_array_memcnt += size;
    SAC_TR_Print ("%d array elements allocated, total now: %d.", size,
                  SAC_TR_array_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_array_memcnt_lock);
}

void
SAC_TR_DecArrayMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_array_memcnt_lock);
    SAC_TR_array_memcnt -= size;
    SAC_TR_Print ("%d array elements deallocated, total now: %d.", size,
                  SAC_TR_array_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_array_memcnt_lock);
}

/******************************************************************************
 *
 * function:
 *   void SAC_TR_IncHiddenMemcnt(int size)
 *   void SAC_TR_DecHiddenMemcnt(int size)
 *
 * description:
 *
 *   These functions are used to increment and decrement the allocated hidden
 *   objects counter and print the corresponding counter status information.
 *
 *   Under a multi-threaded execution scheme the counter is protected
 *   through a dedicated lock.
 *
 ******************************************************************************/

void
SAC_TR_IncHiddenMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_hidden_memcnt_lock);
    SAC_TR_hidden_memcnt += size;
    SAC_TR_Print ("%d hidden objects allocated, total now: %d.", size,
                  SAC_TR_hidden_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_hidden_memcnt_lock);
}

void
SAC_TR_DecHiddenMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_hidden_memcnt_lock);
    SAC_TR_hidden_memcnt -= size;
    SAC_TR_Print ("%d hidden objects allocated, total now: %d.", size,
                  SAC_TR_hidden_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_hidden_memcnt_lock);
}
