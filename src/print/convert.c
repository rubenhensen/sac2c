/*
 *
 * $Log$
 * Revision 2.4  1999/04/09 13:54:07  jhs
 * No changes made, error not in this file.
 *
 * Revision 2.3  1999/04/08 17:17:45  jhs
 * Handling for empty arrays added.
 *
 * Revision 2.2  1999/04/01 13:35:54  cg
 * added lots of '(' and ')' to avoid ambiguity warnings.
 * bug fixed in conversion of float and double constants into
 * strings.
 *
 * Revision 2.1  1999/02/23 12:40:20  sacbase
 * new release made
 *
 * Revision 1.24  1999/01/25 10:21:17  cg
 * Bug fixed in Double2String and Float2String: .0 not added after
 * number in exponential representation.
 *
 * Revision 1.23  1998/11/09 18:44:41  sbs
 * ...size changed to 266 humble humble....
 *
 * Revision 1.22  1998/11/09 17:44:14  sbs
 * Float2String and Double2String patched:
 * Now 260 byte are allocated rather than 256;
 * => prevents SEFAULTS for very long constants ....
 *
 * Revision 1.21  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.20  1998/06/03 14:32:41  cg
 * implementation streamlined
 *
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

#include "internal_lib.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"

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

static char *rename_type[] = {
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

    tmp_string = (char *)Malloc (sizeof (char) * 267);
    /*
     * 256 chars + "." + "e+1000" + ".0f" + "\0" = 267
     */

    sprintf (tmp_string, "%.256g", val);

    if (strchr (tmp_string, '.') == NULL) {
        strcat (tmp_string, ".0");
    }

    strcat (tmp_string, "f");

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

    tmp_string = (char *)Malloc (sizeof (char) * 266);
    /*
     * 256 chars + "." + "e+1000" + ".0" + "\0" = 266
     */

    sprintf (tmp_string, "%.256g", val);

    if (strchr (tmp_string, '.') == NULL) {
        strcat (tmp_string, ".0");
    }

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
        if (TYPES_BASETYPE (type) == T_user) {
            if ((flag != 3) && (TYPES_MOD (type) != NULL)) {
                strcat (tmp_string, TYPES_MOD (type));
                if (compiler_phase >= PH_precompile) {
                    strcat (tmp_string, "__");
                } else {
                    strcat (tmp_string, ":");
                }
            }
            strcat (tmp_string, TYPES_NAME (type));
        } else {
            if (flag == 2) {
                strcat (tmp_string, rename_type[TYPES_BASETYPE (type)]);
            } else {
                strcat (tmp_string, type_string[TYPES_BASETYPE (type)]);
            }
        }

        if (0 != type->dim) {
            if (-1 == type->dim) {
                strcat (tmp_string, "[]");
            } else {
                if (ARRAY_OR_SCALAR == TYPES_DIM (type)) {
                    strcat (tmp_string, "[?]");
                } else if (TYPES_DIM (type) == EMPTY_ARRAY) {
                    strcat (tmp_string, "[*]");
                } else {
                    int i, dim;
                    static char int_string[INT_STRING_LENGTH];
                    int known_shape = 1;
                    if (2 == flag) {
                        strcat (tmp_string, "_");
                    } else {
                        strcat (tmp_string, "[");
                    }
                    if (KNOWN_DIM_OFFSET > TYPES_DIM (type)) {
                        dim = 0 - TYPES_DIM (type) + KNOWN_DIM_OFFSET;
                        known_shape = 0;
                    } else {
                        dim = TYPES_DIM (type);
                    }

                    for (i = 0; i < dim; i++) {
                        if (i != (dim - 1)) {
                            if (2 == flag) {
                                if (1 == known_shape) {
                                    sprintf (int_string, "%d_", type->shpseg->shp[i]);
                                } else {
                                    sprintf (int_string, "._");
                                }
                            } else {
                                if (1 == known_shape) {
                                    sprintf (int_string, "%d, ", type->shpseg->shp[i]);
                                } else {
                                    sprintf (int_string, "., ");
                                }
                            }
                            strcat (tmp_string, int_string);
                        } else {
                            if (2 == flag) {
                                if (1 == known_shape) {
                                    sprintf (int_string, "%d_", type->shpseg->shp[i]);
                                } else {
                                    sprintf (int_string, "._");
                                }
                            } else {
                                if (1 == known_shape) {
                                    sprintf (int_string, "%d]", type->shpseg->shp[i]);
                                } else {
                                    sprintf (int_string, ".]");
                                }
                            }

                            strcat (tmp_string, int_string);
                        }
                    }
                }
            }
        }

        if ((type->attrib == ST_reference) || (type->attrib == ST_readonly_reference)
            || (1 == flag)) {
            strcat (tmp_string, " ");
        }

        if (type->attrib == ST_reference) {
            strcat (tmp_string, "&");
        } else {
            if (type->attrib == ST_readonly_reference) {
                strcat (tmp_string, "(&)");
            }
        }

        if ((NULL != type->id) && (1 == flag)) {
            strcat (tmp_string, type->id);
        }

        type = type->next;
        if (NULL != type) {
            strcat (tmp_string, ", ");
        }
    } while (NULL != type);

    DBUG_RETURN (tmp_string);
}
