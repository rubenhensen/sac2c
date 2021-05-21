/**
 * @file
 * @brief Provides a set of functions to manipulate the SAC profiler.
 *
 * This file is part of the implementation of the SAC runtime library.
 *
 * It contains function and global variable definitions needed for
 * profiling.
 *
 */

#include "config.h"

#ifndef SAC_BACKEND_MUTC

#include <stdio.h>
#include <stdarg.h>

#include "libsac/profile/profile_print.h"

/*
 *  Internal type definitions
 */

typedef struct timeval __PF_TIMER;

/*
 *  Internal macro definitions
 */

#define __PF_TIMER_FORMAT "%8.2f"

/** Gets the time in seconds from a time struct */
#define __PF_TIMER(timer) timer.tv_sec + timer.tv_usec / 1000000.0

/** Computes percentage time of timer1 in timer2 */
#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    ((timer2.tv_sec + timer2.tv_usec / 1000000.0) == 0                                   \
       ? 0                                                                               \
       : (timer1.tv_sec + timer1.tv_usec / 1000000.0) * 100                              \
           / (timer2.tv_sec + timer2.tv_usec / 1000000.0))

/**
 * @brief This function prints some header lines for presenting profiling
 *        information.
 *
 * @param title Title of header
 */
void
SAC_PF_PrintHeader (const char *title)
{
    fprintf (stderr, "\n****************************************"
                     "****************************************\n");
    fprintf (stderr, "*** %-72.72s ***\n", title);
    fprintf (stderr, "****************************************"
                     "****************************************\n");
}

/**
 * @brief This function prints some header lines for presenting profiling
 *        information for a specific node when the distributed memory
 *        backend is used.
 *
 * @param title Title or name of the node
 * @param rank Distmem rank
 */
void
SAC_PF_PrintHeaderNode (const char *title, size_t rank)
{
    fprintf (stderr, "\n****************************************"
                     "****************************************\n");
    fprintf (stderr, "*** %-60s for node %2zd ***\n", title, rank);
    fprintf (stderr, "****************************************"
                     "****************************************\n");
}

/**
 * @brief This function prints some header lines for presenting profiling
 *        information.
 *
 * @param title Title of the sub-header
 * @param lineno Line number
 */
void
SAC_PF_PrintSubHeader (const char *title, size_t lineno)
{
    fprintf (stderr, "call to `%-56.56s' in line #%zu:\n", title, lineno);
}

/**
 * @brief This function prints out a section heading using the
 *        given title information.
 *
 * @param title Title of the section
 */
void
SAC_PF_PrintSection (const char *title)
{
    fprintf (stderr, " ## %-72s\n", title);
}

/**
 * @brief Function for printing timing information in a formatted manner.
 *
 * @param title Title or label of field
 * @param space Filler to modify the spacing of the string
 * @param time Time structure
 */
void
SAC_PF_PrintTime (const char *title, const char *space, const __PF_TIMER *time)
{
    fprintf (stderr,
             "%-40s: %s "__PF_TIMER_FORMAT
             " sec\n",
             title, space, __PF_TIMER ((*time)));
}

/**
 * @brief Function for printing a counter in a formatted manner.
 *
 * @param title Title or label of the field
 * @param space Filler to modify the spacing of the string
 * @param count Integer value
 */
void
SAC_PF_PrintCount (const char *title, const char *space, unsigned long count)
{
    fprintf (stderr, "%-40s: %s %lu\n", title, space, count);
}

/**
 * @brief Prints out a formatted string containing a title and associated size
 *        value.
 *
 * @param title Field title or name
 * @param space Add further space, otherwise just give `""`
 * @param size
 * @param unit Name of unit, which is appended to the string
 */
void
SAC_PF_PrintSize (const char *title, const char *space, unsigned long size, const char *unit)
{
    fprintf (stderr, "%-40s: %s %8lu %s\n", title, space, size, unit);
}

/**
 * @brief Function for printing timing information with a percentage of
 *        total time.
 *
 * @param title Title or label for field
 * @param space Filler to modify the spacing of the string
 * @param time1 Measured time as a time-structure
 * @param time2 Global total measured time, used to compute percentage
 */
void
SAC_PF_PrintTimePercentage (const char *title, const char *space, const __PF_TIMER *time1,
                            const __PF_TIMER *time2)
{
    fprintf (stderr,
             "%-40s:%s  "__PF_TIMER_FORMAT
             " sec  %8.2f %%\n",
             title, space, __PF_TIMER ((*time1)),
             __PF_TIMER_PERCENTAGE ((*time1), (*time2)));
}

#endif /* !defined(SAC_BACKEND_MUTC) */
