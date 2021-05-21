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
#include <stdbool.h>

#include "trace.h"

#if SAC_MT_MODE > 0
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_PTHREAD 1
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#define SAC_DO_PHM 1

#include "runtime/mt_h/rt_mt.h"    // needed for SAC_MT_DEFINE_LOCK, SAC_MT_STATIC
#include "libsac/mt/mt.h"          // needed for SAC_MT_Internal_CurrentThreadId
#include "runtime/mt_h/schedule.h" // needed for SAC_MT_ACQUIRE_LOCK
#include "libsac/mt/mt_beehive.h"  // needed for SAC_MT_output_lock
#include "runtime/phm_h/phm.h"     // needed for SAC_HM_THREADID_INVALID

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_PHM

/* globals used for collecting memory operation statistics */
static unsigned long SAC_TR_array_memcnt = 0; /* size of memory allocated */
static unsigned long SAC_TR_hidden_memcnt = 0; /* size of memory allocated */

SAC_MT_STATIC SAC_MT_DEFINE_LOCK (SAC_TR_array_memcnt_lock)
SAC_MT_STATIC SAC_MT_DEFINE_LOCK (SAC_TR_hidden_memcnt_lock)

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

#if SAC_MT_MODE > 0

void
SAC_TR_Print (const char *format, ...)
{
    va_list arg_p;
    unsigned int thread_id;

    thread_id = SAC_MT_Internal_CurrentThreadId ();

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
SAC_TR_Print (const char *format, ...)
{
    va_list arg_p;

#if ENABLE_DISTMEM
    /* Check if tracing is active for this node. */
    if ((SAC_DISTMEM_rank != SAC_DISTMEM_RANK_UNDEFINED
         && SAC_DISTMEM_trace_profile_rank == (int)SAC_DISTMEM_rank)
        || SAC_DISTMEM_trace_profile_rank == SAC_DISTMEM_TRACE_PROFILE_RANK_ANY) {
        fprintf (stderr, "TR:n%zd-> ", SAC_DISTMEM_rank);
    } else
#endif
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
    SAC_TR_Print ("%d array elements allocated, total now: %lu.", size,
                  SAC_TR_array_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_array_memcnt_lock);
}

void
SAC_TR_DecArrayMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_array_memcnt_lock);
    SAC_TR_array_memcnt -= size;
    SAC_TR_Print ("%d array elements deallocated, total now: %lu.", size,
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
    SAC_TR_Print ("%d hidden objects allocated, total now: %lu.", size,
                  SAC_TR_hidden_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_hidden_memcnt_lock);
}

void
SAC_TR_DecHiddenMemcnt (int size)
{
    SAC_MT_ACQUIRE_LOCK (SAC_TR_hidden_memcnt_lock);
    SAC_TR_hidden_memcnt -= size;
    SAC_TR_Print ("%d hidden objects allocated, total now: %lu.", size,
                  SAC_TR_hidden_memcnt);
    SAC_MT_RELEASE_LOCK (SAC_TR_hidden_memcnt_lock);
}
