#include <stdarg.h>
#include <stdio.h>

int verbose = 0;

void
DoPrint (char *format, ...)
{
    va_list arg_p;

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);
}
