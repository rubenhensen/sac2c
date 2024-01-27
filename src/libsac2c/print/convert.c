#define TYPE_LENGTH 256      /* dimension of array of char */
#define INT_STRING_LENGTH 16 /* dimension of array of char */

/******************************************************************************
 *
 * @brief strings for primitive types.
 *
 ******************************************************************************/
static char *type_string[] = {
#define TYP_IFpr_str(str) str
#include "type_info.mac"
};

/******************************************************************************
 *
 * @brief strings for primitive types, used for renaming of functions.
 *
 ******************************************************************************/
static char *rename_type[] = {
#define TYP_IFfunr_str(str) str
#include "type_info.mac"
};

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "constants_internal.h"
#include "free.h"
#include "globals.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "str.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"
#include "user_types.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"

/******************************************************************************
 *
 * @fn char *CVfloat2String (float val)
 *
 * @brief converts a float to a string so that the string: does not loose any
 * digits, and will be recognized as float from any C-Compiler!
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn char *CVfloatvec2String (floatvec val)
 *
 * @brief converts a floatvec to a string.
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn char *CVdouble2String (double val)
 *
 * @brief converts a double to a string so that the string: does not loose any
 * digits, and will be recognized as double from any C-Compiler!
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn char *CVshape2String (int dim, int *shp)
 *
 * @brief Converts a dimensionality and shape to a shape string.
 *
 ******************************************************************************/
char *
CVshape2String (int dim, int *shp)
{
    int pos, space, written;
    char buf[255], *buffer;
    char *res;

    DBUG_ENTER ();

    buf[0] = '[';
    buffer = &buf[1];
    space = 254;

    for (pos = 0; pos < dim; pos++) {
        if (pos < dim - 1) {
            written = snprintf (buffer, space - 5, " %d,", shp[pos]);
        } else {
            written = snprintf (buffer, space - 5, " %d", shp[pos]);
        }

        if (written > space - 6) {
            buffer += (space - 6);
            sprintf (buffer, "...");
            buffer += 3;
            break;
        } else {
            buffer += written;
            space -= written;
        }
    }

    snprintf (buffer, 2, "]");
    res = (char *)malloc (strlen (buf) + 1);
    strcpy (res, buf);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVtype2String (ntype *type, int flag, bool all)
 *
 * @brief convertes the infomation in a type into a string.
 *   flag == 2: used for renaming of functions (lookup type-name
 *              in rename_type[] instead of type_string[])
 *   flag == 3: the module name is not included into string
 *   !all:      force Type2String only print 1st type in list
 *
 ******************************************************************************/
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
 * @fn char *CVbasetype2String (simpletype type)
 *
 * @brief yields a pointer to a static memory area that contains the name of
 * basic data type as a string.
 *
 ******************************************************************************/
char *
CVbasetype2String (simpletype type)
{
    char *res;

    static char *ctype_string[] = {
#define TYP_IFpr_str(str) str
#include "type_info.mac"
    };

    DBUG_ENTER ();

    res = ctype_string[type];

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVbasetype2ShortString (simpletype type)
 *
 * @brief yields a pointer to a static memory area that contains the short name
 * of basic data type as a string.
 *
 ******************************************************************************/
char *
CVbasetype2ShortString (simpletype type)
{
    char *res;

    static char *ctype_string[] = {
#define TYP_IFfunr_str(str) str
#include "type_info.mac"
    };

    DBUG_ENTER ();

    res = ctype_string[type];

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVspids2String (node *spids)
 *
 * @brief converts an N_spids chain to a comma-separated string.
 *
 ******************************************************************************/
char *
CVspids2String (node *spids)
{
    char *res;

    DBUG_ENTER ();

    res = "[";

    if (spids != NULL) {
        res = STRcat (res, SPIDS_NAME (spids));
        spids = SPIDS_NEXT (spids);

        while (spids != NULL) {
            res = STRcatn (3, res, ", ", SPIDS_NAME (spids));
            spids = SPIDS_NEXT (spids);
        }
    }

    res = STRcat (res, "]");

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVspid2String (node *spid)
 *
 * @brief converts an N_spid, which can be a type pattern feature, to a string.
 *
 ******************************************************************************/
char *
CVspid2String (node *spid)
{
    node *tdim;
    char *res;

    DBUG_ENTER ();

    tdim = SPID_TDIM (spid);

    if (tdim != NULL) {
        if (NODE_TYPE (tdim) == N_num) {
            static char num[6];
            sprintf (num, "%d", NUM_VAL (tdim));
            res = num;
        } else {
            DBUG_ASSERT (NODE_TYPE (tdim) == N_spid,
                         "expected an N_num or N_spid node");
            res = CVspid2String (tdim);
        }

        res = STRcatn (3, res, ":", SPID_NAME (spid));
    } else {
        res = STRcpy (SPID_NAME (spid));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVtypePatternShape2String (node *shape)
 *
 * @brief converts a single type pattern feature to a string.
 *
 ******************************************************************************/
char *
CVtypePatternShape2String (node *shape)
{
    char *res;

    DBUG_ENTER ();

    if (NODE_TYPE (shape) == N_num) {
        static char num[6];
        sprintf (num, "%d", NUM_VAL (shape));
        res = STRcpy (num);
    } else if (NODE_TYPE (shape) == N_dot) {
        res = STRcpy (".");
    } else {
        DBUG_ASSERT (NODE_TYPE (shape) == N_spid,
                     "expected an N_num, N_dot, or N_spid node");
        res = CVspid2String (shape);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVtypePattern2String (node *pattern)
 *
 * @brief converts a type pattern to a string.
 *
 ******************************************************************************/
char *
CVtypePattern2String (node *pattern)
{
    node *shp;
    char *shp_str, *res;

    DBUG_ENTER ();

    res = STRcat (TYScalarType2String (TYPEPATTERN_ELEMENTTYPE (pattern)), "[");

    shp = TYPEPATTERN_SHAPE (pattern);

    if (shp != NULL) {
        shp_str = CVtypePatternShape2String (EXPRS_EXPR (shp));
        res = STRcat (res, shp_str);
        shp = EXPRS_NEXT (shp);

        while (shp != NULL) {
            shp_str = CVtypePatternShape2String (EXPRS_EXPR (shp));
            res = STRcatn (3, res, ",", shp_str);
            shp = EXPRS_NEXT (shp);
        }
    }

    res = STRcat (res, "]");

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn char *CVintBytes2String ( unsigned int bytes)
 *
 * @brief yields a pointer to a static memory area that contains a "dotted"
 * version of the integer number given. It is primarily used for printing memory
 * usages.
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
