/*
 *
 * $Log$
 * Revision 3.3  2004/11/24 23:17:31  cg
 * SacDevCamp approved.
 *
 * Revision 3.2  2001/04/24 11:08:19  dkr
 * ProcessErrorMessage():
 * DBUG_ASSERT about TABs removed (Note, that TABs may occur in the
 * source code and therefore also in scan/parse error messages!)
 * TABs are printed as SPACEs now
 *
 * Revision 3.1  2000/11/20 17:59:24  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:39:11  sacbase
 * new release made
 *
 * Revision 1.27  1999/01/19 10:46:06  sbs
 * ret=NULL; inserted in default-case of ItemName
 *
 * Revision 1.26  1998/06/05 15:23:25  cg
 * functions ModName() and ItemName() now use an internal static buffer
 * in order to avoid memory leaks.
 *
 * Revision 1.25  1998/03/04 16:18:12  cg
 * removed functions postmortem(), Error(), and polished cleanup()
 *
 * Revision 1.24  1998/02/25 09:05:55  cg
 * All global variables moved to globals.[ch]
 *
 * Revision 1.23  1997/04/25 12:13:52  sbs
 * malloc replaced by Malloc
 *
 * Revision 1.22  1997/03/19  13:32:55  cg
 * removed compiler phase 'prepare-linking' ("Checking required external module/class
 * implementations")
 * switched to single tmp directory tmp_dirname instead of build_dirname and
 * store_dirname
 *
 * Revision 1.21  1997/03/11  16:33:20  cg
 * added compiler phase 'writing dependencies to stdout'
 *
 * Revision 1.20  1996/09/11  06:11:28  cg
 * New compilation phase "Updating Makefile with dependencies" added.
 *
 * Revision 1.19  1996/01/25  18:37:21  cg
 * renamed compiler phase objinit resoltutio
 *
 * Revision 1.18  1996/01/07  16:51:59  cg
 * renamed some compiler phases
 *
 * Revision 1.17  1996/01/05  12:23:42  cg
 * added function CleanUp(), added some compiler phases
 *
 * Revision 1.16  1996/01/02  15:43:47  cg
 * added new compiler phase name
 *
 * Revision 1.15  1995/12/29  10:21:14  cg
 * added new compiler phase readsib
 *
 * Revision 1.14  1995/12/15  13:38:52  cg
 * ItemName and ModName no longer use Malloc for memory allocation.
 * Now, they can be used in DBUG_PRINTs AND DBUG_PRINT can be used in Malloc !!
 *
 * Revision 1.13  1995/12/01  20:23:23  cg
 * changed compilation sequence: objinit.c now after import.c
 *
 * Revision 1.12  1995/12/01  16:09:58  cg
 * added name of new compilation phase 'precompile'
 *
 * Revision 1.11  1995/11/16  19:32:38  cg
 * added new compilation phase name for 'RemoveVoidFunctions'
 *
 * Revision 1.10  1995/11/16  15:33:20  cg
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
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dbug.h"
#include "Error.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"

/*
 *
 *  functionname  : ERRprocessErrorMessage
 *  arguments     : 1) format string
 *                  2) variable number of arguments similar to printf
 *  description   : evaluates the format string with the given arguments
 *                  and splits it into single lines using @ as delimiter.
 *
 */

void
ERRprocessErrorMessage (char *format, ...)
{
    va_list arg_p;
    int index, line, last_space;

    DBUG_ENTER ("ERRprocessErrorMessage");

    va_start (arg_p, format);
    vsprintf (global.error_message_buffer, format, arg_p);
    va_end (arg_p);

    index = 0;
    last_space = 0;
    line = 0;

    while ((index < MAX_ERROR_MESSAGE_LENGTH)
           && (global.error_message_buffer[index] != '\0')) {
        if (global.error_message_buffer[index] == '\t') {
            global.error_message_buffer[index] = ' ';
        }

        if (global.error_message_buffer[index] == ' ') {
            last_space = index;
        }

        if (global.error_message_buffer[index] == '\n') {
            global.error_message_buffer[index] = '@';
            line = 0;
        } else {
            if (line == global.current_line_length) {
                if (global.error_message_buffer[last_space] == ' ') {
                    global.error_message_buffer[last_space] = '@';
                    line = index - last_space;
                } else {
                    break;
                }
            }
        }

        index++;
        line++;
    }

    DBUG_PRINT ("ERROR", ("processed message: %s", global.error_message_buffer));

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ERRmodName
 *  arguments     : 1) module name, maybe NULL
 *                  2) item name, may not be NULL
 *  description   : constructs a new string combining module and item name
 *                  with a colon.
 *                  ERRmodName is designed to simplify error messages.
 *
 *  remarks       : Since an internal buffer is used for the generation of
 *                  a combined name, this function should exclusively be
 *                  employed for immediately subsequent print operations.
 *
 */

char *
ERRmodName (char *mod, char *name)
{
    char *tmp;
    static char buffer[128];

    DBUG_ENTER ("ERRmodName");

    if (mod == NULL) {
        tmp = name;
    } else {
        strncpy (buffer, mod, 63);
        strcat (buffer, ":");
        strncat (buffer, name, 63);
        tmp = buffer;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : ERRitemName
 *  arguments     : 1) pointer to fundef, objdef or typedef node
 *  description   : generates the full combined name of the item,
 *                  including the module name when present.
 *
 */

char *
ERRitemName (node *item)
{
    char *ret;

    DBUG_ENTER ("ERRitemName");

    switch (NODE_TYPE (item)) {
    case N_fundef:
        ret = ERRmodName (FUNDEF_MOD (item), FUNDEF_NAME (item));
        break;
    case N_typedef:
        ret = ERRmodName (TYPEDEF_MOD (item), TYPEDEF_NAME (item));
        break;
    case N_objdef:
        ret = ERRmodName (OBJDEF_MOD (item), OBJDEF_NAME (item));
        break;
    default:
        DBUG_ASSERT (0, "Wrong item in call of function 'ERRitemName`");
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : ERRcleanUp
 *  arguments     : ---
 *  description   : removes all files and directories generated by sac2c
 *                  except the actual result file. Used to clean up the
 *                  file system in regular as well as irregular program
 *                  termination.
 *
 */

void
ERRcleanUp ()
{
    DBUG_ENTER ("ERRcleanUp");

    if (global.cleanup && (global.tmp_dirname != NULL)) {
        ILIBsystemCall ("%s %s", global.config.rmdir, global.tmp_dirname);
    }

    DBUG_VOID_RETURN;
}
