/*
 *
 * $Log$
 * Revision 1.2  1995/05/23 09:59:39  sbs
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
        fprintf (outfile, "fprintf( stderr, \", \");\n");                                \
    }

#define ICM_DEF(prf, trf)                                                                \
    if (trf & traceflag) {                                                               \
        int sep = 0;                                                                     \
        INDENT;                                                                          \
        fprintf (outfile, "fprintf( stderr, \"%s( \");\n", #prf);
#define ICM_STR(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "fprintf( stderr, \"%s\");\n", name);                              \
    sep = 1;
#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "fprintf( stderr, \"%d\");\n", name);                              \
    sep = 1;
#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            INDENT;                                                                      \
            fprintf (outfile, "fprintf( stderr, \"%s\");\n", name[i]);                   \
            sep = 1;                                                                     \
        }                                                                                \
    }
#define ICM_END(prf)                                                                     \
    INDENT;                                                                              \
    fprintf (outfile, "fprintf( stderr, \")\\n\");\n");                                  \
    }                                                                                    \
    ;

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
