/*
 *
 * $Log$
 * Revision 1.3  1996/01/21 14:15:39  cg
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
        fprintf (outfile, "__SAC__Runtime_Print( \", \");\n");                           \
    }

#define ICM_DEF(prf, trf)                                                                \
    if (trf & traceflag) {                                                               \
        int sep = 0;                                                                     \
        INDENT;                                                                          \
        fprintf (outfile, "__SAC__Runtime_Print( \"%s( \");\n", #prf);
#define ICM_STR(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "__SAC__Runtime_Print( \"%s \");\n", name);                        \
    sep = 1;
#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "__SAC__Runtime_Print( \"%d \");\n", name);                        \
    sep = 1;
#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            INDENT;                                                                      \
            fprintf (outfile, "__SAC__Runtime_Print( \"%s \");\n", name[i]);             \
            sep = 1;                                                                     \
        }                                                                                \
    }
#define ICM_END(prf)                                                                     \
    INDENT;                                                                              \
    fprintf (outfile, "__SAC__Runtime_Print( \")\\n\");\n");                             \
    }                                                                                    \
    ;

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
