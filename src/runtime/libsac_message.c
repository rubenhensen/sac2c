/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:34:14  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_message.c
 *
 * prefix: _SAC_
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
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void
_SAC_RuntimeError (char *format, ...)
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

void
_SAC_Print (char *format, ...)
{
    va_list arg_p;

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);
}

void
_SAC_PrintHeader (char *title)
{
    _SAC_Print ("****************************************"
                "****************************************\n");
    _SAC_Print ("*** %-72s ***\n", title);
    _SAC_Print ("****************************************"
                "****************************************\n");
}
