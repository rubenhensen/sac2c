/*
 *
 * $Log$
 * Revision 1.5  1996/02/21 16:27:57  cg
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

#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

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
