/*
 *
 * $Log$
 * Revision 1.10  1995/11/16 15:33:20  cg
 * renamed compiler phase flatten
 *
 * Revision 1.9  1995/11/10  14:59:22  cg
 * new functions ProcessErrorMessage and NumberOfDigit,
 * used by the revised macros in Error.h
 * some further global variables for error handling and layout formatting.
 *
 * Revision 1.8  1995/10/19  10:04:45  cg
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

#include "Error.h"

#define PUTC_STDERR(c) putc ((c), stderr)

int errors = 0;         /* counter for number of errors */
int warnings = 0;       /* counter for number of warnings */
int verbose_level = 3;  /* controls compile time output   */
int compiler_phase = 1; /* counter for compilation phases */

int message_indent = 0;
int last_indent = 0;
int current_line_length;

int silent = 0; /* only needed for asi-compatibility */

char error_message_buffer[MAX_ERROR_MESSAGE_LENGTH];

char *compiler_phase_name[] = {"",
                               "Loading SAC program",
                               "Resolving global object initializations",
                               "Resolving imports from modules and classes",
                               "Simplifying source code",
                               "Running type inference system",
                               "Checking module/class declaration file",
                               "Resolving implicit types",
                               "Analysing functions and global objects",
                               "Generating SAC-Information-Block",
                               "Resolving global objects and reference parameters",
                               "Checking uniqueness property of objects",
                               "Running SAC optimizations",
                               "Running PSI optimizations",
                               "Running reference count inference system",
                               "Generating C-code",
                               "Generating C-compiler call",
                               "Unknown compiler phase"};

/*
 *
 *  functionname  : DoPrint
 *  arguments     : 1) format string like the one of printf
 *                  2) ... arguments of format string
 *  description   : prints an error message on stderr
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
 *  functionname  : ProcessErrorMessage
 *  arguments     : 1) format string
 *                  2) variable number of arguments similar to printf
 *  description   : evaluates the format string with the given arguments
 *                  and splits it into single lines using @ as delimiter.
 *  global vars   : error_message_buffer, current_line_length
 *  internal funs : ---
 *  external funs : vsprintf, va_start, va_end
 *  macros        : DBUG, MAX_ERROR_MESSAGE_LENGTH
 *
 *  remarks       :
 *
 */

void
ProcessErrorMessage (char *format, ...)
{
    va_list arg_p;
    int index, line, last_space;

    DBUG_ENTER ("ProcessErrorMessage");

    va_start (arg_p, format);
    vsprintf (error_message_buffer, format, arg_p);
    va_end (arg_p);

    index = 0;
    last_space = 0;
    line = 0;

    while ((index < MAX_ERROR_MESSAGE_LENGTH) && (error_message_buffer[index] != '\0')) {
        DBUG_ASSERT (error_message_buffer[index] != '\t',
                     "TABs not allowed in error messages, use SPACEs !");

        if (error_message_buffer[index] == ' ') {
            last_space = index;
        }

        if (error_message_buffer[index] == '\n') {
            error_message_buffer[index] = '@';
            line = 0;
        } else {
            if (line == current_line_length) {
                if (error_message_buffer[last_space] == ' ') {
                    error_message_buffer[last_space] = '@';
                    line = index - last_space;
                } else {
                    break;
                }
            }
        }

        index++;
        line++;
    }

    DBUG_PRINT ("ERROR", ("processed message: %s", error_message_buffer));

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : NumberOfDigits
 *  arguments     : 1) integer
 *  description   : returns the number of digits, e.g 4 for 1000
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG
 *
 *  remarks       :
 *
 */

int
NumberOfDigits (int number)
{
    int i = 1;

    DBUG_ENTER ("NumberOfDigits");

    while (number / 10 >= 1) {
        number = number / 10;
        i += 1;
    }

    DBUG_RETURN (i);
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
