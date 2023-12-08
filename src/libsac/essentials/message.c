/*****************************************************************************
 *
 * file:   message.c
 *
 * prefix: SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains two functions for producing arbitrary output during runtime
 *   or for terminating program execution with a runtime error message,
 *   respectively. The direct usage of respective C functions is not possible
 *   since name/type clashes with functions whose prototypes are derived from
 *   imported external modules may occur.
 *
 *   During multi-threaded execution, the integrity of messages is guaranteed
 *   through a locking mechanism.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#if SAC_MT_MODE > 0
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1
#define SAC_DO_MT_PTHREAD 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#include "libsac/essentials/message.h"
#include "runtime/mt_h/rt_mt.h" // needed for SAC_MT_ACQUIRE_LOCK!
#include "runtime/mt_h/schedule.h" // needed for mt_beehive.h!
#include "libsac/mt/mt_beehive.h" // needed for SAC_MT_output_lock!



#ifdef SAC_BACKEND_MUTC
char *
strtok (char *str, const char *delim)
{
    return (str);
}
#endif

void (*SAC_MessageExtensionCallback) (void) = 0;

/*
 * Function definitions
 */

void
SAC_RuntimeError (const char *format, ...)
{
    va_list arg_p;
    char *line;
    char *tmp;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback)
        SAC_MessageExtensionCallback ();

#if ENABLE_DISTMEM
    if (SAC_DISTMEM_rank != SAC_DISTMEM_RANK_UNDEFINED) {
        fprintf (stderr, "\n\n*** SAC runtime error at Node %zd\n", SAC_DISTMEM_rank);
    } else
#endif
        fprintf (stderr, "\n\n*** SAC runtime error\n");

    va_start (arg_p, format);

    tmp = strdup (format);
    line = strtok (tmp, "\n");
    while (line != NULL) {
        fprintf (stderr, "*** ");
        vfprintf (stderr, line, arg_p);
        fprintf (stderr, "\n");
        line = strtok (NULL, "\n");
    }

    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    /* If the program does not use the distributed memory backend,
     * an empty dummy function in libsacdistmem.nodistmem will be called. */
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

void
SAC_RuntimeError_Mult (int cnt, ...)
{
    va_list arg_p;
    char *line;
    char *next;
    int pos;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback)
        SAC_MessageExtensionCallback ();

    fprintf (stderr, "\n\n*** SAC runtime error\n");

    va_start (arg_p, cnt);

    for (pos = 0; pos < cnt; pos++) {
        next = strdup (va_arg (arg_p, char *));
        line = strtok (next, "@");
        while (line != NULL) {
            fprintf (stderr, "*** ");
            vfprintf (stderr, line, arg_p);
            fprintf (stderr, "\n");
            line = strtok (NULL, "@");
        }
    }

    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

void
SAC_RuntimeErrorLine (int line, const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback)
        SAC_MessageExtensionCallback ();

    fprintf (stderr, "\n\n*** SAC runtime error\n");
    fprintf (stderr, "*** line %d\n*** ", line);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

void
SAC_RuntimeWarning (const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    if (SAC_DISTMEM_rank != SAC_DISTMEM_RANK_UNDEFINED) {
        fprintf (stderr, "\n\n*** SAC runtime warning at Node %zd\n", SAC_DISTMEM_rank);
    } else
#endif
        fprintf (stderr, "\n\n*** SAC runtime warning\n");

    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

/* Prints a runtime warning only at the master node.
 * Useful for warnings that are not node- but program-specific. */
void
SAC_RuntimeWarningMaster (const char *format, ...)
{
    va_list arg_p;

#if ENABLE_DISTMEM
    if (SAC_DISTMEM_rank == SAC_DISTMEM_RANK_UNDEFINED
        || SAC_DISTMEM_rank == SAC_DISTMEM_RANK_MASTER)
#endif
        do {
            SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

            fprintf (stderr, "\n\n*** SAC runtime warning\n");

            fprintf (stderr, "*** ");

            va_start (arg_p, format);
            vfprintf (stderr, format, arg_p);
            va_end (arg_p);

            fprintf (stderr, "\n\n");

            SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
        } while (0);
}

#define MAX_SHAPE_SIZE 255

const char *
SAC_PrintShape (SAC_array_descriptor_t desc)
{
    int pos;
    char buf[MAX_SHAPE_SIZE];
    char *buffer;
    char *res;
    int space, written;

    buf[0] = '[';
    buffer = &buf[1];
    space = MAX_SHAPE_SIZE-1;
    

    for (pos = 0; pos < DESC_DIM (desc); pos++) {
        if (pos < DESC_DIM (desc) - 1) {
            written = snprintf (buffer, space - 5, " %d,",
                                        (int)DESC_SHAPE (desc, pos));
        } else {
            written = snprintf (buffer, space - 5, " %d",
                                        (int)DESC_SHAPE (desc, pos));
        }
        if (written >= space - 5) {
            buffer += (space - 6);
            sprintf (buffer, "...");
            buffer += 3;
            break;
        } else {
            buffer += written;
            space -= written;
        }
    }

    snprintf (buffer, 2, "]");

    res = (char *)malloc (strlen (buf) + 1);
    strcpy (res, buf);

    return (res);
}

void
SAC_Print (const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}
