/*
 *
 * $Log$
 * Revision 1.8  1995/10/19 10:04:45  cg
 * new function ItemName for convenient output of combined names
 * of types, functions, or global objects.
 *
 * Revision 1.7  1995/10/18  13:32:32  cg
 * new function ModName(char*, char*) allows for easy
 * printing of combined names of types, functions and objects.
 * mainly designed for use with error macros.
 *
 * Revision 1.6  1995/05/04  11:39:51  sbs
 * DoPrint implemented by vfprintf!
 *
 * Revision 1.5  1994/12/13  11:26:34  hw
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
#include <string.h>

#include "Error.h"

#include "tree_basic.h"

#include "dbug.h"
#include "internal_lib.h"

#define PUTC_STDERR(c) putc ((c), stderr)

int errors = 0;   /* counter for number of errors */
int warnings = 0; /* counter for number of warnings */
int silent = 0;
int compiler_phase = 1; /* counter for compilation phases */

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

    DBUG_ENTER ("DoPrint");
    va_start (arg_p, format);

    vfprintf (stderr, format, arg_p);
    va_end (arg_p);
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ModName
 *  arguments     : 1) module name, maybe NULL
 *                  2) item name, may not be NULL
 *  description   : constructs a new string combining module and item name
 *                  with a colon.
 *                  ModName is designed to simplify error messages.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strcpy, strcat, strlen
 *  macros        :
 *
 *  remarks       :
 *
 */

char *
ModName (char *mod, char *name)
{
    char *tmp;

    DBUG_ENTER ("ModName");

    if (mod == NULL) {
        tmp = name;
    } else {
        tmp = Malloc (strlen (mod) + strlen (name) + 2);
        tmp = strcpy (tmp, mod);
        tmp = strcat (tmp, ":");
        tmp = strcat (tmp, name);
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : ItemName
 *  arguments     : 1) pointer to fundef, objdef or typedef node
 *  description   : generates the full combined name of the item,
 *                  including the module name when present.
 *  global vars   : ---
 *  internal funs : ModName
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
ItemName (node *item)
{
    char *ret;

    DBUG_ENTER ("ItemName");

    switch (NODE_TYPE (item)) {
    case N_fundef:
        ret = ModName (FUNDEF_MOD (item), FUNDEF_NAME (item));
        break;
    case N_typedef:
        ret = ModName (TYPEDEF_MOD (item), TYPEDEF_NAME (item));
        break;
    case N_objdef:
        ret = ModName (OBJDEF_MOD (item), OBJDEF_NAME (item));
        break;
    default:
        DBUG_ASSERT (0, "Wrong item in call of function 'ItemName`");
    }

    DBUG_RETURN (ret);
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
