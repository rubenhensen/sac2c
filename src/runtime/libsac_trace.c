/*
 *
 * $Log$
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
 * file:   libsac_trace.c
 *
 * prefix: SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *****************************************************************************/

#define _POSIX_C_SOURCE 199506L

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int SAC_TR_array_memcnt = 0;
int SAC_TR_hidden_memcnt = 0;

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

#if 0

#define TRACE_BUFFER_SIZE 256

static int trace_layout_flag=0;



void SAC_TR_PrintTraceHeader(char *format, ...)
{
  va_list arg_p;
  static char buffer[TRACE_BUFFER_SIZE];

  va_start(arg_p, format);
  vsprintf(buffer, format, arg_p);
  va_end(arg_p);

  if (strlen(buffer)>40)
  {
    fprintf(stderr, "%s\n", buffer);
    fprintf(stderr, "%-40s -> ", " ");
  }
  else
  {
    fprintf(stderr, "%-40s -> ", buffer);
  }
  
  trace_layout_flag=0;
}



void SAC_TR_PrintTraceInfo(char *format, ...)
{
  va_list arg_p;

  if (trace_layout_flag==0)
  {
    trace_layout_flag=1;
  }
  else
  {
    fprintf(stderr, "%-40s    ", " ");
  }

  va_start(arg_p, format);
  vfprintf(stderr, format, arg_p);
  va_end(arg_p);

  fprintf(stderr, "\n");
}

#endif
