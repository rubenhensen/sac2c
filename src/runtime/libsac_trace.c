/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:36:27  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_trace.c
 *
 * prefix: _SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define TRACE_BUFFER_SIZE 256

int _SAC_array_memcnt = 0;
int _SAC_hidden_memcnt = 0;

static int trace_layout_flag = 0;

void
_SAC_PrintTraceHeader (char *format, ...)
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

void
_SAC_PrintTraceInfo (char *format, ...)
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
