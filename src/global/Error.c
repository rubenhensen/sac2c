/*
 *
 * $Log$
 * Revision 1.5  1994/12/13 11:26:34  hw
 * added function: void DoPrint( char *format, ...)
 * added macro : PUTC_STDERR(c)
 *
 * Revision 1.4  1994/12/08  17:56:38  hw
 * new declarations: - int errors;
 *                   - int warnings;
 *                   - int silent;
 *
 * Revision 1.3  1994/12/05  15:08:48  hw
 * changed output of function Error
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */
#include <stdarg.h>
#include <stdio.h>

#include "Error.h"
#include "dbug.h"

#define PUTC_STDERR(c) putc ((c), stderr)

int errors = 0;   /* counter for number of errors */
int warnings = 0; /* counter for number of warnings */
int silent = 0;

/*
 *
 *  functionname  : DoPrint
 *  arguments     : 1) format string like the one of printf
 *                  2) ... arguments of format string
 *  description   : prints a error message on stderr
 *
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : fprintf
 *  macros        : DBUG..., PUTC_STDERR, va_list, va_start, va_end
 *
 *  remarks       : following format information are considered:
 *                  %s %d \n \t
 *
 */
void
DoPrint (char *format, ...)
{
    va_list arg_p;
    char *p, *sval;
    int ival;

    DBUG_ENTER ("DoPrint");
    va_start (arg_p, format);

    for (p = format; *p; p++) {
        if ('\\' == *p)
            switch (*++p) {
            case 'n':
                fprintf (stderr, "\n");
                break;

            case 't':
                fprintf (stderr, "\t");
                break;
            default:
                PUTC_STDERR (*p);
                break;
            }
        else if ('%' == *p)
            switch (*++p) {
            case 'd':
                ival = va_arg (arg_p, int);
                fprintf (stderr, "%d", ival);
                break;
            case 's':
                for (sval = va_arg (arg_p, char *); *sval; sval++)
                    PUTC_STDERR (*sval);
                break;
            default:
                PUTC_STDERR (*p);
                break;
            }
        else
            PUTC_STDERR (*p);
    }
    va_end (arg_p);

    DBUG_VOID_RETURN;
}

void
Error (char *string, int status)
{
    fprintf (stderr, "\n%s\n", string);
    exit (status);
}

/*
void postmortem(char *s)
{
   fprintf(stderr,"\n\nPostMortem: %s \n",s);
   exit(99);
}
*/
