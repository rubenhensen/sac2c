/*
 *
 * $Log$
 * Revision 3.15  2002/10/08 01:51:46  dkr
 * Type2String() corrected
 *
 * Revision 3.14  2002/09/16 14:24:58  dkr
 * Type2String() modified: some spaces in output removed
 *
 * Revision 3.13  2002/09/09 20:36:27  dkr
 * Type2String() corrected
 *
 * Revision 3.12  2002/08/13 17:22:05  dkr
 * IntBytes2String: argument is unsigned now
 *
 * Revision 3.11  2002/08/05 17:03:01  sbs
 * OldTypeSignature2String added
 *
 * Revision 3.10  2002/03/05 17:49:22  sbs
 * Type2String now handles NULL pointers as well 8-))
 *
 * Revision 3.9  2001/06/28 09:26:06  cg
 * Syntax of array types changed:
 * int[] -> int[+]  and  int[?] -> int[*]
 * int[] is still supported as input.
 *
 * Revision 3.8  2001/05/17 10:03:24  nmw
 * type mistake in IntByte2String corrected, warning eliminated
 *
 * Revision 3.7  2001/05/17 07:35:11  sbs
 * IntBytes2String added
 * Malloc / Free checked
 *
 * Revision 3.6  2001/03/15 16:53:52  dkr
 * Type2String: types->id and the '&' for reference objects are no longer
 * printed here!
 * Note, that types->id and types->attrib is *not* part of the virtual
 * TYPES 'types'!!
 *
 * Revision 3.5  2001/03/15 15:48:28  dkr
 * Type2String streamlined
 *
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
 *  description   : convertes the infomation in type into a string
 *
 *                  flag == 2: used for renaming of functions( lookup type-name
 *                             in rename_type[] instead of type_string[])
 *                  flag == 3: the module name is not included into string
 *
 *                  ! all: force Type2String only print 1st type in list
 *
 */

char *
Type2String (types *type, int flag, bool all)
{
    char *tmp_string;

    DBUG_ENTER ("Type2String");

    tmp_string = (char *)Malloc (sizeof (char) * TYPE_LENGTH);
    tmp_string[0] = '\0';

    if (type == NULL) {
        strcat (tmp_string, "(null)");
    } else {
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

            if (TYPES_DIM (type) != 0) {
                if (TYPES_DIM (type) == UNKNOWN_SHAPE) {
                    if (flag == 2) {
                        strcat (tmp_string, "_P");
                    } else {
                        strcat (tmp_string, "[+]");
                    }
                } else {
                    if (ARRAY_OR_SCALAR == TYPES_DIM (type)) {
                        if (flag == 2) {
                            strcat (tmp_string, "_S");
                        } else {
                            strcat (tmp_string, "[*]");
                        }
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
                                        sprintf (int_string, "%d_",
                                                 TYPES_SHAPE (type, i));
                                    } else {
                                        sprintf (int_string, "x_");
                                    }
                                } else {
                                    if (known_shape == 1) {
                                        sprintf (int_string, "%d,",
                                                 TYPES_SHAPE (type, i));
                                    } else {
                                        sprintf (int_string, ".,");
                                    }
                                }
                                strcat (tmp_string, int_string);
                            } else {
                                if (flag == 2) {
                                    if (known_shape == 1) {
                                        sprintf (int_string, "%d", TYPES_SHAPE (type, i));
                                    } else {
                                        sprintf (int_string, "x");
                                    }
                                } else {
                                    if (1 == known_shape) {
                                        sprintf (int_string, "%d]",
                                                 TYPES_SHAPE (type, i));
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

            type = TYPES_NEXT (type);

            if (!all) { /* break after first type in list */
                type = NULL;
            }

            if (type != NULL) {
                strcat (tmp_string, ", ");
            }
        } while (type != NULL);
    }

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

/******************************************************************************
 *
 * function:
 *   char *IntBytes2String( unsigned int bytes)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   a "dotted" version of the integer number given. It is primarily used
 *   for printing memory usages.
 *
 ******************************************************************************/

char *
IntBytes2String (unsigned int bytes)
{
    static char res[32];
    char *tmp = &res[0];
    int factor = 1000000000;
    int num;

    DBUG_ENTER ("IntBytes2String");

    while ((bytes / factor == 0) && (factor >= 1000)) {
        factor /= 1000;
        tmp += sprintf (tmp, "    ");
    }
    tmp += sprintf (tmp, "%3u", (bytes / factor));
    while (factor >= 1000) {
        bytes = bytes % factor;
        factor /= 1000;
        num = bytes / factor;
        if (num < 10) {
            tmp += sprintf (tmp, ".00%1u", num);
        } else if (num < 100) {
            tmp += sprintf (tmp, ".0%2u", num);
        } else {
            tmp += sprintf (tmp, ".%3u", num);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char *OldTypeSignature2String( node *fundef)
 *
 * description:
 *   constructs a string that represents the signature of a fundef
 *   specified by the user
 *
 ******************************************************************************/

char *
OldTypeSignature2String (node *fundef)
{
    static char buf[4096];
    char *tmp = &buf[0];

    char *tmp_str;
    node *arg;

    DBUG_ENTER ("OldTypeSignature2String");

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        tmp_str = Type2String (ARG_TYPE (arg), 0, FALSE);
        tmp += sprintf (tmp, "%s ", tmp_str);
        tmp_str = Free (tmp_str);
        arg = ARG_NEXT (arg);
    }

    tmp_str = Type2String (FUNDEF_TYPES (fundef), 0, TRUE);
    tmp += sprintf (tmp, "-> %s", tmp_str);
    tmp_str = Free (tmp_str);

    DBUG_RETURN (StringCopy (buf));
}
