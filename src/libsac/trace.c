/*
 *
 * $Log$
 * Revision 2.4  2000/02/07 09:51:59  cg
 * Changed setting of semicolons in definitions and declarations of
 * mutex locks in order to avoid nasty warnings from cc.
 *
 * Revision 2.3  2000/01/17 16:25:58  cg
 * Reorganized implementation of the runtime system for
 * multi-threaded execution.
 *
 * Revision 2.2  1999/07/08 12:30:35  cg
 * File moved to new directory src/libsac.
 *
 *
 */

/*
 *
 * Revision 2.1  1999/02/23 12:43:42  sacbase
 * new release made
 *
 * Revision 1.5  1998/07/10 15:21:50  cg
 * bug fixed in vararg macro usage
 *
 * Revision 1.4  1998/07/10 08:08:25  cg
 * header file stdarg.h used instead of varargs.h which is not
 * available under Linux.
 *
 * Revision 1.3  1998/06/29 08:50:14  cg
 * added '#define _POSIX_C_SOURCE 199506L' for multi-threaded execution.
 *
 * Revision 1.2  1998/05/07 08:13:24  cg
 * SAC runtime library implementation converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:36:27  cg
 * Initial revision
 *
 *
 */

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
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#include "sac_mt.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

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
    unsigned int *thread_id_ptr;

    thread_id_ptr = (unsigned int *)pthread_getspecific (SAC_MT_threadid_key);

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (thread_id_ptr == NULL) {
        fprintf (stderr, "TR:??:-> ");
    } else {
        fprintf (stderr, "TR:%2u:-> ", *thread_id_ptr);
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
    SAC_TR_Print ("%d array elements allocated, total now: %d.", size,
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
