/*
 *
 * $Log$
 * Revision 1.2  1996/01/09 08:52:32  cg
 * first compilable revision
 *
 * Revision 1.1  1996/01/09  08:31:41  cg
 * Initial revision
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#include "dbug.h"

#define TRACE_BUFFER_SIZE 256

int __SAC__Runtime_trace_memcnt = 0;

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

    DBUG_ENTER ("__SAC__Runtime_Error");

    fprintf (stderr, "*** SAC runtime error\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    exit (1);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  :
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

    DBUG_ENTER ("__SAC__Runtime_Print");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  :
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

    DBUG_ENTER ("__SAC__Runtime_PrintTraceHeader");

    va_start (arg_p, format);
    vsprintf (buffer, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "%-40s -> ", buffer);

    trace_layout_flag = 0;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  :
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

    DBUG_ENTER ("__SAC__Runtime_PrintTraveInfo");

    if (trace_layout_flag == 0) {
        trace_layout_flag = 1;
    } else {
        fprintf (stderr, "%-40s    ", " ");
    }

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  :
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

    DBUG_ENTER ("__SAC__Runtime_malloc");

    tmp = malloc (size);

    if (tmp == NULL) {
        __SAC__Runtime_Error ("Unable to allocate %d bytes of memory", size);
    }

    DBUG_RETURN (tmp);
}
