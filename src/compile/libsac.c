/*
 *
 * $Log$
 * Revision 1.7  1998/03/17 12:22:40  cg
 * Now, an alternative way of initializing character arrays derived from
 * strings is implemented. This uses the new ICM ND_CREATE_CONST_ARRAY_C
 * which in turn calls the libsac function String2Array.
 *
 * Revision 1.6  1997/05/28 12:35:25  sbs
 * Profiling integrated
 *
 * Revision 1.5  1996/02/21  16:27:57  cg
 * minor layout change in funtion RuntimeError
 *
 * Revision 1.4  1996/01/25  15:04:57  cg
 * added __SAC__Runtime_hidden_memcnt and __SAC__Runtime_array_memcnt
 *
 * Revision 1.3  1996/01/21  14:16:55  cg
 * minor layout modification
 *
 * Revision 1.2  1996/01/09  08:52:32  cg
 * first compilable revision
 *
 * Revision 1.1  1996/01/09  08:31:41  cg
 * Initial revision
 *
 *
 *
 *
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include "profile.h"

typedef struct timeval __PF_TIMER;
#define __PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0
#define __PF_ADD_TIMERS(timer, timer1, timer2)                                           \
    timer##.tv_sec = timer1##.tv_sec + timer2##.tv_sec;                                  \
    timer##.tv_usec = timer1##.tv_usec + timer2##.tv_usec
#define __PF_TIMER_FORMAT "%8.2f"
#define __PF_TIMER_SPACE "              "
#define __PF_TIMER(timer) timer##.tv_sec + timer##.tv_usec / 1000000.0
#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    (timer1##.tv_sec + timer1##.tv_usec / 1000000.0) * 100                               \
      / (timer2##.tv_sec + timer2##.tv_usec / 1000000.0)

#define TRACE_BUFFER_SIZE 256

int __SAC__Runtime_array_memcnt = 0;
int __SAC__Runtime_hidden_memcnt = 0;

static int trace_layout_flag = 0;

/*
 *
 *  functionname  : __SAC__Runtime_Error
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_Error (char *format, ...)
{
    va_list arg_p;

    fprintf (stderr, "\n\n*** SAC runtime error\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    exit (1);
}

/*
 *
 *  functionname  : __SAC__Runtime_Print
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_Print (char *format, ...)
{
    va_list arg_p;

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintTraceHeader
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintTraceHeader (char *format, ...)
{
    va_list arg_p;
    static char buffer[TRACE_BUFFER_SIZE];

    va_start (arg_p, format);
    vsprintf (buffer, format, arg_p);
    va_end (arg_p);

    if (strlen (buffer) > 40) {
        fprintf (stderr, "%s\n", buffer);
        fprintf (stderr, "%-40s -> ", " ");
    } else {
        fprintf (stderr, "%-40s -> ", buffer);
    }

    trace_layout_flag = 0;
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintTraceInfo
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintTraceInfo (char *format, ...)
{
    va_list arg_p;

    if (trace_layout_flag == 0) {
        trace_layout_flag = 1;
    } else {
        fprintf (stderr, "%-40s    ", " ");
    }

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");
}

/*
 *
 *  functionname  : __SAC__Runtime_malloc
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void *
__SAC__Runtime_malloc (int size)
{
    void *tmp;

    tmp = malloc (size);

    if (tmp == NULL) {
        __SAC__Runtime_Error ("Unable to allocate %d bytes of memory", size);
    }

    return (tmp);
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintHeader
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintHeader (char *title)
{
    __SAC__Runtime_Print ("****************************************"
                          "****************************************\n");
    __SAC__Runtime_Print ("*** %-72s ***\n", title);
    __SAC__Runtime_Print ("****************************************"
                          "****************************************\n");
}

/******************************************************************************
 *
 * function:
 *   void __SAC__Runtime_string2array(char *array, const char *string)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
__SAC__Runtime_String2Array (char *array, const char *string)
{
    int i = 0, j = 0;

    while (string[j] != '\0') {
        if (string[j] == '\\') {
            switch (string[j + 1]) {
            case 'n':
                array[i++] = '\n';
                j += 2;
                break;
            case 't':
                array[i++] = '\t';
                j += 2;
                break;
            case 'v':
                array[i++] = '\v';
                j += 2;
                break;
            case 'b':
                array[i++] = '\b';
                j += 2;
                break;
            case 'r':
                array[i++] = '\r';
                j += 2;
                break;
            case 'f':
                array[i++] = '\f';
                j += 2;
                break;
            case 'a':
                array[i++] = '\a';
                j += 2;
                break;
            case '"':
                array[i++] = '"';
                j += 2;
                break;
            default:
                array[i++] = '\\';
                j += 1;
            }
        } else {
            array[i++] = string[j++];
        }
    }
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintTime
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintTime (char *title, char *space, __PF_TIMER *time)
{
    __SAC__Runtime_Print ("%-20s: %s "__PF_TIMER_FORMAT
                          " sec\n",
                          title, space, __PF_TIMER ((*time)));
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintTimePercentage
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintTimePercentage (char *title, char *space, __PF_TIMER *time1,
                                    __PF_TIMER *time2)
{
    __SAC__Runtime_Print ("%-20s:%s  "__PF_TIMER_FORMAT
                          " sec  %8.2f %%\n",
                          title, space, __PF_TIMER ((*time1)),
                          __PF_TIMER_PERCENTAGE ((*time1), (*time2)));
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintProfile
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

__PF_TIMER *
__SAC__Runtime_PrintProfileOverall (
  int display_flag, int __PF_maxfun, int *__PF_maxfunap,
  __PF_TIMER __PF_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP])
{
    int i, j;
    __PF_TIMER with_total, non_with_total;
    static __PF_TIMER total;

    __PF_INIT_TIMER (with_total);
    __PF_INIT_TIMER (non_with_total);
    __PF_INIT_TIMER (total);
    for (i = 0; i < __PF_maxfun; i++) {
        for (j = 0; j < __PF_maxfunap[i]; j++) {
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_fun_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (non_with_total, non_with_total, __PF_fun_timer[i][j]);
        }
    }
    __PF_ADD_TIMERS (total, with_total, non_with_total);

    __SAC__Runtime_PrintHeader ("Overall Profile");
    __SAC__Runtime_PrintTime ("time used", __PF_TIMER_SPACE, &total);
    if (display_flag != 0) {
        __SAC__Runtime_PrintTimePercentage ("   with-loop", "", &with_total, &total);
        __SAC__Runtime_PrintTimePercentage ("   non-with-loop", "", &non_with_total,
                                            &total);
    }
    return (&total);
}

/*
 *
 *  functionname  : __SAC__Runtime_PrintProfileFuns
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
__SAC__Runtime_PrintProfileFuns (
  __PF_TIMER *total_time, int display_fun, int display_with, int __PF_maxfun,
  int *__PF_maxfunap, __PF_TIMER __PF_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
  __PF_TIMER __PF_fw_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
  char *__PF_fun_name[PF_MAXFUN])
{
    int i, j;
    __PF_TIMER with_total, parent_with_total, non_with_total, total;

    __SAC__Runtime_PrintHeader ("Function Profiles");
    if (display_fun == 0)
        __PF_maxfun = 1;
    for (i = 0; i < __PF_maxfun; i++) {
        __PF_INIT_TIMER (with_total);
        __PF_INIT_TIMER (parent_with_total);
        __PF_INIT_TIMER (non_with_total);
        __PF_INIT_TIMER (total);
        for (j = 0; j < __PF_maxfunap[i]; j++) {
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (parent_with_total, parent_with_total,
                             __PF_fw_fun_timer[i][j]);
            __PF_ADD_TIMERS (non_with_total, non_with_total, __PF_fun_timer[i][j]);
        }
        __PF_ADD_TIMERS (total, with_total, non_with_total);
        __PF_ADD_TIMERS (total, total, parent_with_total);
        if ((total.tv_sec != 0) || (total.tv_usec != 0)) {
            __SAC__Runtime_PrintHeader (__PF_fun_name[i]);
            __SAC__Runtime_PrintTimePercentage ("time used", __PF_TIMER_SPACE, &total,
                                                total_time);
            if (display_with != 0) {
                __SAC__Runtime_PrintTimePercentage ("   with-loop(own)", "", &with_total,
                                                    &total);
                __SAC__Runtime_PrintTimePercentage ("   with-loop(parent)", "",
                                                    &parent_with_total, &total);
                __SAC__Runtime_PrintTimePercentage ("   non-with-loop", "",
                                                    &non_with_total, &total);
            }
        }
    }
}
