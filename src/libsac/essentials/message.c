/******************************************************************************
 *
 * This file is part of the implementation of the SAC runtime library. It
 * contains two functions for producing arbitrary output during runtime or for
 * terminating program execution with a runtime error message, respectively.
 * The direct usage of respective C functions is not possible since name/type
 * clashes with functions whose prototypes are derived from imported external
 * modules may occur.
 *
 * During multi-threaded execution, the integrity of messages is guaranteed
 * through a locking mechanism.
 *
 ******************************************************************************/
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
#endif /* SAC_MT_MODE */

#include "libsac/essentials/message.h"
#include "runtime/mt_h/rt_mt.h" // needed for SAC_MT_ACQUIRE_LOCK!
#include "runtime/mt_h/schedule.h" // needed for mt_beehive.h!
#include "libsac/mt/mt_beehive.h" // needed for SAC_MT_output_lock!

#ifdef SAC_BACKEND_MUTC
char *
strtok (char *str, const char *delim)
{
    return str;
}
#endif

void (*SAC_MessageExtensionCallback) (void) = 0;

static void
PrintHeader (const char *loglevel, const char *filename, int line, int column)
{
    #if ENABLE_DISTMEM
    if (SAC_DISTMEM_rank != SAC_DISTMEM_RANK_UNDEFINED) {
        fprintf (stderr, "\n*** SAC runtime %s at node %zd\n", loglevel, SAC_DISTMEM_rank);
    } else
    #endif /* ENABLE_DISTMEM */
        fprintf (stderr, "\n*** SAC runtime %s\n", loglevel);

    if (filename != NULL) {
        fprintf (stderr, "*** In %s", filename);
    }

    if (line >= 0) {
        fprintf (stderr, ", line %d", line);

        if (column >= 0) {
            fprintf (stderr, ", column %d", column);
        }

        fprintf (stderr, "\n");
    }
}

static void
PrintLines (const char *format, va_list arg_p)
{
    char *line, *tmp;

    tmp = strdup (format);
    line = strtok (tmp, "\n");
    while (line != NULL) {
        fprintf (stderr, "*** ");
        vfprintf (stderr, line, arg_p);
        fprintf (stderr, "\n");
        line = strtok (NULL, "\n");
    }

    fprintf (stderr, "\n");
}

/******************************************************************************
 *
 * @fn void SAC_RuntimeWarning (const char *format, ...)
 *
 * @brief Prints a runtime error message to stderr, but does not abort the
 * program. If the given error message contains multiple lines, then each line
 * is printed to a separate line in stderr as well, prepended with "***".
 *
 ******************************************************************************/
void
SAC_RuntimeWarning (const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    PrintHeader ("warning", NULL, -1, -1);
    va_start (arg_p, format);
    PrintLines (format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

/******************************************************************************
 *
 * @fn void SAC_RuntimeWarningLoc (const char *filename, const char *format, int line, int col, ...)
 *
 * @brief Prints a runtime error message to stderr, but does not abort the
 * program. If the given error message contains multiple lines, then each line
 * is printed to a separate line in stderr as well, prepended with "***".
 *
 ******************************************************************************/
void
SAC_RuntimeWarningLoc (const char *filename, int line, int col, const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    PrintHeader ("warning", filename, line, col);
    va_start (arg_p, format);
    PrintLines (format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

/******************************************************************************
 *
 * @fn void SAC_RuntimeError (const char *format, ...)
 *
 * @brief Prints a runtime error message to stderr and then aborts the program.
 * If the given error message contains multiple lines, then each line is printed
 * to a separate line in stderr as well, prepended with "***".
 *
 ******************************************************************************/
void
SAC_RuntimeError (const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback) {
        SAC_MessageExtensionCallback ();
    }

    PrintHeader ("error", NULL, -1, -1);
    va_start (arg_p, format);
    PrintLines (format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    /**
     * If the program does not use the distributed memory backend, an empty
     * dummy function in libsacdistmem.nodistmem will be called.
     */
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

void
SAC_RuntimeErrorLoc (const char *filename, int line, int col, const char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback) {
        SAC_MessageExtensionCallback ();
    }

    PrintHeader ("error", filename, line, col);
    va_start (arg_p, format);
    PrintLines (format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    /**
     * If the program does not use the distributed memory backend, an empty
     * dummy function in libsacdistmem.nodistmem will be called.
     */
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

/******************************************************************************
 *
 * @fn void SAC_RuntimeError_Mult (int cnt, ...)
 *
 * @brief Prints multiple runtime error messages to stderr and then aborts the
 * program.
 *
 ******************************************************************************/
void
SAC_RuntimeError_Mult (int cnt, ...)
{
    va_list arg_p;
    char *line, *next;
    int pos;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    if (SAC_MessageExtensionCallback) {
        SAC_MessageExtensionCallback ();
    }

    fprintf (stderr, "\n*** SAC runtime error\n");

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

    fprintf (stderr, "\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

#if ENABLE_DISTMEM
    /**
     * If the program does not use the distributed memory backend, an empty
     * dummy function in libsacdistmem.nodistmem will be called.
     */
    SAC_DISTMEM_ABORT (1);
#endif

    exit (1);
}

#define MAX_SHAPE_SIZE 255

const char *
SAC_PrintShape (SAC_array_descriptor_t desc)
{
    int pos, val, space, written;
    char buf[MAX_SHAPE_SIZE], *buffer;
    char *res;

    buf[0] = '[';
    buffer = &buf[1];
    space = MAX_SHAPE_SIZE-1;

    for (pos = 0; pos < DESC_DIM (desc); pos++) {
        val = (int)DESC_SHAPE (desc, pos);
        if (pos < DESC_DIM (desc) - 1) {
            written = snprintf (buffer, space - 5, " %d,", val);
        } else {
            written = snprintf (buffer, space - 5, " %d", val);
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
