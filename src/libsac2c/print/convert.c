#define TYPE_LENGTH 256      /* dimension of array of char */
#define INT_STRING_LENGTH 16 /* dimension of array of char */

/* strings for primitve types */

#define TYP_IFpr_str(str) str
static char *type_string[] = {
#include "type_info.mac"
};

/* strings for primitve types used for renaming of functions*/

#define TYP_IFfunr_str(str) str
static char *rename_type[] = {
#include "type_info.mac"
};

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "convert.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "constants_internal.h"
#include "globals.h"
#include "new_types.h"
#include "user_types.h"
#include "type_utils.h"
#include "namespaces.h"
#include "str.h"
#include "memory.h"

/*
 *
 *  functionname  : CVfloat2String
 *  arguments     :  1) float-val
 *  description   : prints a float into a string so that the string
 *                  1) does not loose any digits
 *                  2) will be recognized as float from any C-Compiler!
 *
 */

char *
CVfloat2String (float val)
{
    char *tmp_string;

    DBUG_ENTER ();

    tmp_string = (char *)MEMmalloc (sizeof (char) * 270);
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

char *
CVfloatvec2String (floatvec val)
{
    char *s;
    int len;
    const size_t vec_len = sizeof (floatvec) / sizeof (float);
    const size_t mem
      = 270 * vec_len + vec_len * strlen (", ") + strlen ("(floatvec){}");
    const size_t buf_size = sizeof(char) * mem;

    s = (char *)MEMmalloc (buf_size);
    len = snprintf (s, buf_size, "(floatvec){");

    for (unsigned i = 0; i < vec_len; i++) {
        char *t = CVfloat2String (FLOATVEC_IDX (val, i));
        /* offset by len to append to string s */
        len += snprintf (s+len, buf_size - (size_t)len, "%s%s", t, i == vec_len - 1 ? "}" : ", ");
        MEMfree (t);
    }

    return s;
}

/*
 *
 *  functionname  : CVdouble2String
 *  arguments     :  1) double-val
 *  description   : prints a double into a string so that the string
 *                  1) does not loose any digits
 *                  2) will be recognized as double from any C-Compiler!
 *
 */

char *
CVdouble2String (double val)
{
    char *tmp_string;

    DBUG_ENTER ();

    tmp_string = (char *)MEMmalloc (sizeof (char) * 270);
    /*
     * 256 chars + "." + "e+1000" + ".0" + "\0" = 266
     */

    snprintf (tmp_string, 270, "%.256g", val);

    if (strchr (tmp_string, '.') == NULL) {
        strcat (tmp_string, ".0");
    }

    DBUG_RETURN (tmp_string);
}

/*
 *
 *  functionname  : CVtype2String
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
CVtype2String (ntype *type, int flag, bool all)
{
    char *tmp_string;
    usertype udt;

    DBUG_ENTER ();

    tmp_string = (char *)MEMmalloc (sizeof (char) * TYPE_LENGTH);
    tmp_string[0] = '\0';

    if (type == NULL) {
        strcat (tmp_string, "(null)");
    } else {
        if (TUisArrayOfUser (type)) {
            udt = TYgetUserType (TYgetScalar (type));
            if ((flag != 3) && (UTgetNamespace (udt) != NULL)) {
                strcat (tmp_string, NSgetModule (UTgetNamespace (udt)));
                if (global.compiler_phase >= PH_pc) {
                    strcat (tmp_string, "__");
                } else {
                    strcat (tmp_string, ":");
                }
            }
            strcat (tmp_string, UTgetName (udt));
        } else {
            if (flag == 2) {
                strcat (tmp_string, rename_type[TYgetSimpleType (TYgetScalar (type))]);
            } else {
                strcat (tmp_string, type_string[TYgetSimpleType (TYgetScalar (type))]);
            }
        }

        if (!TUisScalar (type)) {
            if (TYisAUDGZ (type)) {
                if (flag == 2) {
                    strcat (tmp_string, "_P");
                } else {
                    strcat (tmp_string, "[+]");
                }
            } else if (TYisAUD (type)) {
                if (flag == 2) {
                    strcat (tmp_string, "_S");
                } else {
                    strcat (tmp_string, "[*]");
                }
            } else {
                int i, dim;
                static char int_string[INT_STRING_LENGTH];
                if (flag == 2) {
                    strcat (tmp_string, "_");
                } else {
                    strcat (tmp_string, "[");
                }
                dim = TYgetDim (type);

                for (i = 0; i < dim; i++) {
                    if (i != (dim - 1)) {
                        if (flag == 2) {
                            if (TYisAKS (type)) {
                                sprintf (int_string, "%d_",
                                         SHgetExtent (TYgetShape (type), i));
                            } else {
                                sprintf (int_string, "X_");
                            }
                        } else {
                            if (TYisAKS (type)) {
                                sprintf (int_string, "%d,",
                                         SHgetExtent (TYgetShape (type), i));
                            } else {
                                sprintf (int_string, ".,");
                            }
                        }
                        strcat (tmp_string, int_string);
                    } else {
                        if (flag == 2) {
                            if (TYisAKS (type)) {
                                sprintf (int_string, "%d",
                                SHgetExtent (TYgetShape (type), i));
                            } else {
                                sprintf (int_string, "X");
                            }
                        } else {
                            if (TYisAKS (type)) {
                                sprintf (int_string, "%d]",
                                         SHgetExtent (TYgetShape (type), i));
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

    DBUG_RETURN (tmp_string);
}

/******************************************************************************
 *
 * function:
 *   char *CVbasetype2String(simpletype type)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   the name of basic data type as a string.
 *
 ******************************************************************************/

char *
CVbasetype2String (simpletype type)
{
    char *res;

#define TYP_IFpr_str(str) str
    static char *ctype_string[] = {
#include "type_info.mac"
    };

    DBUG_ENTER ();

    res = ctype_string[type];

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char *CVbasetype2ShortString(simpletype type)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   the short name of basic data type as a string.
 *
 ******************************************************************************/

char *
CVbasetype2ShortString (simpletype type)
{
    char *res;

#define TYP_IFfunr_str(str) str
    static char *ctype_string[] = {
#include "type_info.mac"
    };

    DBUG_ENTER ();

    res = ctype_string[type];

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char *CVintBytes2String( unsigned int bytes)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   a "dotted" version of the integer number given. It is primarily used
 *   for printing memory usages.
 *
 ******************************************************************************/

char *
CVintBytes2String (size_t bytes)
{
    static char res[32];
    char *tmp = &res[0];
    size_t factor = 1000000000;
    size_t num;

    DBUG_ENTER ();

    while ((bytes / factor == 0) && (factor >= 1000)) {
        factor /= 1000;
        tmp += sprintf (tmp, "    ");
    }
    tmp += sprintf (tmp, "%3zu", (bytes / factor));
    while (factor >= 1000) {
        bytes = bytes % factor;
        factor /= 1000;
        num = bytes / factor;
        if (num < 10) {
            tmp += sprintf (tmp, ".00%1zu", num);
        } else if (num < 100) {
            tmp += sprintf (tmp, ".0%2zu", num);
        } else {
            tmp += sprintf (tmp, ".%3zu", num);
        }
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
