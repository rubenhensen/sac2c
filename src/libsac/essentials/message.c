/*
 *
 * $Id$
 *
 */

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

#ifdef MT
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#include "sac.h"

/*
 * Function definitions
 */

void
SAC_RuntimeError (char *format, ...)
{
    va_list arg_p;
    char *line;
    char *tmp;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    SAC_HM_ShowDiagnostics ();
    /*
     * If the program is not linked with the diagnostic version of the private
     * heap manager, a dummy function will be called here defined either in
     * heapmgr/setup.c or in libsac/nophm.c.
     */

    fprintf (stderr, "\n\n*** SAC runtime error\n");

    va_start (arg_p, format);

    tmp = strdup (format);
    line = strtok (tmp, "@");
    while (line != NULL) {
        fprintf (stderr, "*** ");
        vfprintf (stderr, line, arg_p);
        fprintf (stderr, "\n");
        line = strtok (NULL, "@");
    }

    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

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

    SAC_HM_ShowDiagnostics ();
    /*
     * If the program is not linked with the diagnostic version of the private
     * heap manager, a dummy function will be called here defined either in
     * heapmgr/setup.c or in libsac/nophm.c.
     */

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

    exit (1);
}

void
SAC_RuntimeErrorLine (int line, char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    SAC_HM_ShowDiagnostics ();
    /*
     * If the program is not linked with the diagnostic version of the private
     * heap manager, a dummy function will be called here defined either in
     * heapmgr/setup.c or in libsac/nophm.c.
     */

    fprintf (stderr, "\n\n*** SAC runtime error\n");
    fprintf (stderr, "*** line %d\n*** ", line);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

    exit (1);
}

void
SAC_RuntimeWarning (char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    fprintf (stderr, "\n\n*** SAC runtime warning\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

#define MAX_SHAPE_SIZE 255

const char *
SAC_PrintShape (SAC_array_descriptor_t desc)
{
    int pos;
    char bufA[MAX_SHAPE_SIZE];
    char bufB[MAX_SHAPE_SIZE];
    char *from = bufA;
    char *to = bufB;
    char *res;
    int written;

    from[0] = '[';
    from[1] = '\0';

    for (pos = 0; pos < DESC_DIM (desc); pos++) {
        if (pos < DESC_DIM (desc) - 1) {
            written
              = snprintf (to, MAX_SHAPE_SIZE - 5, "%s %d,", from, DESC_SHAPE (desc, pos));
        } else {
            written
              = snprintf (to, MAX_SHAPE_SIZE - 5, "%s %d", from, DESC_SHAPE (desc, pos));
        }
        if (written == MAX_SHAPE_SIZE - 5) {
            snprintf (from, MAX_SHAPE_SIZE, "%s...", to);
            break;
        } else {
            char *tmp = to;
            to = from;
            from = tmp;
        }
    }

    snprintf (to, MAX_SHAPE_SIZE, "%s]", from);

    res = malloc (strlen (to) + 1);
    strcpy (res, to);

    return (res);
}

void
SAC_Print (char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}