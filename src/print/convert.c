/*
 *
 * $Log$
 * Revision 3.3  2001/03/15 11:59:16  dkr
 * ST_inout replaced by ST_reference
 *
 * Revision 3.2  2001/02/14 10:16:23  dkr
 * Type2String: access macros used
 *
 * Revision 3.1  2000/11/20 17:59:43  sacbase
 * new release made
 *
 * Revision 2.11  2000/10/30 19:24:44  dkr
 * Type2String: in case of ST_inout a '&' is printed now
 *
 * Revision 2.10  2000/10/27 13:24:04  cg
 * Added new functions Basetype2String() and Shpseg2String().
 *
 * Revision 2.9  2000/08/17 10:08:19  dkr
 * comments for Type2String modified
 *
 * Revision 2.8  2000/06/14 12:59:32  nmw
 * Added flag=4 to print only first type in return list
 *
 * Revision 2.7  1999/10/19 12:57:25  sacbase
 * inclusion of type_info.mac adjusted to new .mac mechanism
 *
 * Revision 2.6  1999/04/15 15:00:56  cg
 * Now, enough memory is allocated for string representation
 * of floating point numbers.
 *
 * Revision 2.5  1999/04/14 16:25:41  jhs
 * int[*] removed for second try with empty arrays.
 *
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
 * [ ... ]
 *
 * Revision 1.1  1994/12/05  13:20:47  hw
 * Initial revision
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

#define TYP_IFpr_str(str) str
char *type_string[] = {
#include "type_info.mac"
};

/* strings for primitve types used for renaming of functions*/

#define TYP_IFfunr_str(str) str
static char *rename_type[] = {
#include "type_info.mac"
};

/*
 *
 *  functionname  : Float2String
 *  arguments     :  1) float-val
 *  description   : prints a float into a string so that the string
 *                  1) does not loose any digits
 *                  2) will be recognized as float from any C-Compiler!
 *
 */

char *
Float2String (float val)
{
    char *tmp_string;

    DBUG_ENTER ("Float2String");

    tmp_string = (char *)Malloc (sizeof (char) * 270);
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
 *
 */

char *
Double2String (double val)
{
    char *tmp_string;

    DBUG_ENTER ("Double2String");

    tmp_string = (char *)Malloc (sizeof (char) * 270);
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
 *                  flag == 1: the identifier type->id is also put into
 *                             the resulting string
 *                  flag == 2: used for renaming of functions( lookup type-name
 *                             in rename_type[] instead of type_string[])
 *                  flag == 3: the module name is not included into string
 *                  flag  & 4: force Type2String only print 1st type in list
 *
 */

char *
Type2String (types *type, int flag)
{
    char *tmp_string;

    DBUG_ENTER ("Type2String");

    tmp_string = (char *)MALLOC (sizeof (char) * TYPE_LENGTH);
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

        PrintStatus (TYPES_STATUS (type), FALSE);

        if (TYPES_DIM (type) != 0) {
            if (TYPES_DIM (type) == -1) {
                strcat (tmp_string, "[]");
            } else {
                if (ARRAY_OR_SCALAR == TYPES_DIM (type)) {
                    strcat (tmp_string, "[?]");
                } else {
                    int i, dim;
                    static char int_string[INT_STRING_LENGTH];
                    int known_shape = 1;
                    if (flag == 2) {
                        strcat (tmp_string, "_");
                    } else {
                        strcat (tmp_string, "[");
                    }
                    if (KNOWN_DIM_OFFSET > TYPES_DIM (type)) {
                        dim = KNOWN_DIM_OFFSET - TYPES_DIM (type);
                        known_shape = 0;
                    } else {
                        dim = TYPES_DIM (type);
                    }

                    for (i = 0; i < dim; i++) {
                        if (i != (dim - 1)) {
                            if (flag == 2) {
                                if (known_shape == 1) {
                                    sprintf (int_string, "%d_", TYPES_SHAPE (type, i));
                                } else {
                                    sprintf (int_string, "._");
                                }
                            } else {
                                if (known_shape == 1) {
                                    sprintf (int_string, "%d, ", TYPES_SHAPE (type, i));
                                } else {
                                    sprintf (int_string, "., ");
                                }
                            }
                            strcat (tmp_string, int_string);
                        } else {
                            if (flag == 2) {
                                if (known_shape == 1) {
                                    sprintf (int_string, "%d_", TYPES_SHAPE (type, i));
                                } else {
                                    sprintf (int_string, "._");
                                }
                            } else {
                                if (1 == known_shape) {
                                    sprintf (int_string, "%d]", TYPES_SHAPE (type, i));
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
            || (flag == 1)) {
            strcat (tmp_string, " ");
        }

        if (type->attrib == ST_reference) {
            strcat (tmp_string, "&");
        } else {
            if (type->attrib == ST_readonly_reference) {
                strcat (tmp_string, "(&)");
            }
        }

        if ((type->id != NULL) && (flag == 1)) {
            strcat (tmp_string, type->id);
        }

        type = TYPES_NEXT (type);

        if (flag & 4) { /* break after first type in list*/
            type = NULL;
        }

        if (type != NULL) {
            strcat (tmp_string, ", ");
        }
    } while (type != NULL);

    DBUG_RETURN (tmp_string);
}

/******************************************************************************
 *
 * function:
 *   char *Shpseg2String(int dim, shpseg *shape)
 *
 * description:
 *   This function converts a given shpseg integer vector data structure into
 *   an allocated string. The first parameter provides the actually used length
 *   of the vector.
 *
 ******************************************************************************/

char *
Shpseg2String (int dim, shpseg *shape)
{
    char *buffer;
    char num_buffer[20];
    int i;

    DBUG_ENTER ("Shpseg2String");

    DBUG_ASSERT ((dim <= SHP_SEG_SIZE), " dimension out of range in SetVect()!");

    /*
     * Instead of accurately computing the buffer space to be allocated,
     * we make a generous estimation.
     */
    buffer = (char *)Malloc (dim * 20);

    buffer[0] = '[';
    buffer[1] = '\0';

    for (i = 0; i < dim - 1; i++) {
        sprintf (num_buffer, "%d", SHPSEG_SHAPE (shape, i));
        strcat (buffer, num_buffer);
        strcat (buffer, ", ");
    }

    if (dim > 0) {
        sprintf (num_buffer, "%d", SHPSEG_SHAPE (shape, dim - 1));
        strcat (buffer, num_buffer);
    }
    strcat (buffer, "]");

    DBUG_RETURN (buffer);
}

/******************************************************************************
 *
 * function:
 *   char *Basetype2String(simpletype type)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   the name of basic data type as a string.
 *
 ******************************************************************************/

char *
Basetype2String (simpletype type)
{
    char *res;

#define TYP_IFpr_str(str) str
    static char *ctype_string[] = {
#include "type_info.mac"
    };

    DBUG_ENTER ("Basetype2String");

    res = ctype_string[type];

    DBUG_RETURN (res);
}
