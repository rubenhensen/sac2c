/*
 *
 * $Log$
 * Revision 1.19  1997/04/24 16:43:16  sbs
 * converted malloc to Malloc
 *
 * Revision 1.18  1996/05/13  14:18:58  hw
 * deleted DBUG_PRINTs in Type2String
 * .
 *
 * Revision 1.17  1996/02/16  09:38:51  sbs
 * floor used for recognizing whole numbers in Double2Str and Float2Str
 *
 * Revision 1.16  1996/02/08  18:04:05  hw
 * changed Type2String ( [.,.] will be printed now ;-)
 *
 * Revision 1.15  1996/02/06  16:10:20  sbs
 * Double2String and Float2String inserted.
 *
 * Revision 1.14  1996/01/16  16:57:00  cg
 * extended macro TYP_IF to 5 entries
 *
 * Revision 1.13  1995/11/06  18:44:45  cg
 * bug fixed in writing reference parameters.
 *
 * Revision 1.12  1995/10/31  08:56:18  cg
 * Parameters with attribute ST_reference will be printed with "&",
 * parameters with attribute ST_readonly_reference will be
 * printed with "(&)".
 *
 * Revision 1.11  1995/10/26  16:03:27  cg
 * Function Type2String now has additional flag to suppress printing
 * of module names.
 *
 * Revision 1.10  1995/07/11  09:01:43  cg
 * reference parameters now considered.
 *
 * Revision 1.9  1995/06/23  12:30:21  hw
 * -changed Type2String to use for renameing of functions
 * - added new "type-string-table" rename_type[]
 *
 * Revision 1.8  1995/01/05  12:44:39  sbs
 * TIF macro inserted
 *
 * Revision 1.7  1995/01/05  11:51:25  sbs
 * MOD_NAME_CON macro inserted for mod-name generation for
 * types and functions.
 *
 * Revision 1.6  1995/01/04  13:32:48  sbs
 * error in Type2String fixed.
 *
 * Revision 1.5  1994/12/31  13:54:17  sbs
 * types->name_mod inserted
 *
 * Revision 1.4  1994/12/20  15:58:18  sbs
 * "void *" as primitive type inserted
 *
 * Revision 1.3  1994/12/19  16:51:48  hw
 * changed TYPE_LENGTH & INT_STRING_LENGTH
 *
 * Revision 1.2  1994/12/14  16:35:39  sbs
 * userdef types integrated
 *
 * Revision 1.1  1994/12/05  13:20:47  hw
 * Initial revision
 *
 *
 *
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "tree.h"
#include "internal_lib.h"
#include "dbug.h"
#include "convert.h"

#define TYPE_LENGTH 256      /* dimension of array of char */
#define INT_STRING_LENGTH 16 /* dimension of array of char */

/* strings for primitve types */
#define TYP_IF(n, d, p, f, sz) p

char *type_string[] = {
#include "type_info.mac"
};
#undef TYP_IF

/* strings for primitve types used for renaming of functions*/
#define TYP_IF(n, d, p, f, sz) f

char *rename_type[] = {
#include "type_info.mac"
};
#undef TYP_IF

/*
 *
 *  functionname  : Float2String
 *  arguments     :  1) float-val
 *  description   : prints a float into a string so that the string
 *                  1) does not loose any digits
 *                  2) will be recognized as float from any C-Compiler!
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, sprintf
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
Float2String (float val)
{
    char *tmp_string;

    DBUG_ENTER ("Float2String");

    tmp_string = (char *)Malloc (sizeof (char) * 256);
    if (val == floor (val))
        sprintf (tmp_string, "%.256g.0f", val);
    else
        sprintf (tmp_string, "%.256gf", val);
    DBUG_RETURN (tmp_string);
}

/*
 *
 *  functionname  : Double2String
 *  arguments     :  1) double-val
 *  description   : prints a double into a string so that the string
 *                  1) does not loose any digits
 *                  2) will be recognized as double from any C-Compiler!
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, sprintf
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
Double2String (double val)
{
    char *tmp_string;

    DBUG_ENTER ("Double2String");

    tmp_string = (char *)Malloc (sizeof (char) * 256);
    if (val == floor (val))
        sprintf (tmp_string, "%.256g.0", val);
    else
        sprintf (tmp_string, "%.256g", val);
    DBUG_RETURN (tmp_string);
}

/*
 *
 *  functionname  : Type2String
 *  arguments     :  1) pointer to type-structure
 *                   2) flag
 *  description   : convertes the infomation in type to a string
 *                  flag ==1: the identifier type->id is also put into
 *                            the resulting string
 *                  flag ==2: used for renaming of functions( lookup type-name
 *                            in rename_type[] instead of type_string[])
 *                  flag ==3: the module name is not included into string
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcat, Malloc, sprintf
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
Type2String (types *type, int flag)
{
    char *tmp_string;

    DBUG_ENTER ("Type2String");

    tmp_string = (char *)Malloc (sizeof (char) * TYPE_LENGTH);
    tmp_string[0] = '\0';

    do {
        if ((type->name_mod != NULL) && (flag != 3)) {
            strcat (tmp_string, type->name_mod);
            strcat (tmp_string, mod_name_con);
        }
        if (2 == flag)
            strcat (tmp_string, SIMPLE4FUN_RENAME (type));
        else
            strcat (tmp_string, SIMPLE2STR (type));

        if (0 != type->dim)
            if (-1 == type->dim)
                strcat (tmp_string, "[]");
            else if (ARRAY_OR_SCALAR == TYPES_DIM (type))
                strcat (tmp_string, "[?]");
            else {
                int i, dim;
                static char int_string[INT_STRING_LENGTH];
                int known_shape = 1;
                if (2 == flag)
                    strcat (tmp_string, "_");
                else
                    strcat (tmp_string, "[");
                if (KNOWN_DIM_OFFSET > TYPES_DIM (type)) {
                    dim = 0 - TYPES_DIM (type) + KNOWN_DIM_OFFSET;
                    known_shape = 0;
                } else
                    dim = TYPES_DIM (type);

                for (i = 0; i < dim; i++)
                    if (i != (dim - 1)) {
                        if (2 == flag)
                            if (1 == known_shape)
                                sprintf (int_string, "%d_", type->shpseg->shp[i]);
                            else
                                sprintf (int_string, "._");
                        else if (1 == known_shape)
                            sprintf (int_string, "%d, ", type->shpseg->shp[i]);
                        else
                            sprintf (int_string, "., ");

                        strcat (tmp_string, int_string);
                    } else {
                        if (2 == flag)
                            if (1 == known_shape)
                                sprintf (int_string, "%d_", type->shpseg->shp[i]);
                            else
                                sprintf (int_string, "._");
                        else if (1 == known_shape)
                            sprintf (int_string, "%d]", type->shpseg->shp[i]);
                        else
                            sprintf (int_string, ".]");

                        strcat (tmp_string, int_string);
                    }
            }

        if ((type->attrib == ST_reference) || (type->attrib == ST_readonly_reference)
            || (1 == flag)) {
            strcat (tmp_string, " ");
        }

        if (type->attrib == ST_reference) {
            strcat (tmp_string, "&");
        } else if (type->attrib == ST_readonly_reference) {
            strcat (tmp_string, "(&)");
        }

        if ((NULL != type->id) && (1 == flag)) {
            strcat (tmp_string, type->id);
        }

        type = type->next;
        if (NULL != type)
            strcat (tmp_string, ", ");
    } while (NULL != type);

    DBUG_RETURN (tmp_string);
}
