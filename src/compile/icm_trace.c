/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:42:49  sacbase
 * new release made
 *
 * Revision 1.8  1998/06/23 12:51:18  cg
 * implemented new ICM argument type VARINT for a variable number
 * of integer arguments.
 *
 * Revision 1.7  1998/05/07 08:10:02  cg
 * C implemented ICMs converted to new naming conventions.
 *
 * Revision 1.6  1998/04/25 16:25:20  sbs
 *  new icm2c / BEtest mechanism implemented!
 *
 * Revision 1.5  1998/03/24 13:49:23  cg
 * New prefix for libsac symbols:
 * _SAC_ instead of __SAC__Runtime_
 *
 * Revision 1.4  1996/01/21 18:06:51  cg
 * Now, string arguments of icms are printed correctly when traced
 *
 * Revision 1.3  1996/01/21  14:15:39  cg
 * Now, routines from the SAC runtime library are used instead
 * of those from stdio
 *
 * Revision 1.2  1995/05/23  09:59:39  sbs
 * fprintf's of arguments to stderr inserted.
 *
 * Revision 1.1  1995/05/04  11:48:35  sbs
 * Initial revision
 *
 *
 *
 */

#define SEP                                                                              \
    if (sep) {                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "SAC_Print( \", \");\n");                                      \
    }

#define ICM_DEF(prf, trf)                                                                \
    if (trf & traceflag) {                                                               \
        int sep = 0;                                                                     \
        INDENT;                                                                          \
        fprintf (outfile, "SAC_Print( \"%s( \");\n", #prf);

#define ICM_STR(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    if (name[0] == '"') {                                                                \
        fprintf (outfile, "SAC_Print( %s );\n", name);                                   \
    } else {                                                                             \
        fprintf (outfile, "SAC_Print( \"%s \");\n", name);                               \
    }                                                                                    \
    sep = 1;

#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_Print( \"%d \");\n", name);                                   \
    sep = 1;

#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            INDENT;                                                                      \
            if (name[i][0] == '"') {                                                     \
                fprintf (outfile, "SAC_Print( \"\\%s\"\\\"\" );\n", name[i]);            \
            } else {                                                                     \
                fprintf (outfile, "SAC_Print( \"%s \");\n", name[i]);                    \
            }                                                                            \
            sep = 1;                                                                     \
        }                                                                                \
    }

#define ICM_VARINT(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_Print( \"%d \");\n", name[i]);                        \
            sep = 1;                                                                     \
        }                                                                                \
    }

#define ICM_END(prf, args)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_Print( \")\\n\");\n");                                        \
    }                                                                                    \
    ;

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
