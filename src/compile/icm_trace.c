/*
 *
 * $Log$
 * Revision 1.1  1995/05/04 11:48:35  sbs
 * Initial revision
 *
 *
 *
 */

#if 0
#define SEP                                                                              \
    if (sep)                                                                             \
        fprintf (outfile, ", ");

#define ICM_DEF(prf, trf)                                                                \
    {                                                                                    \
        int sep = 0;                                                                     \
        fprintf (outfile, "/*\n * %s( ", #prf);
#define ICM_STR(name)                                                                    \
    SEP;                                                                                 \
    fprintf (outfile, "%s", name);                                                       \
    sep = 1;
#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    fprintf (outfile, "%d", name);                                                       \
    sep = 1;
#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            AccessConst (name, i);                                                       \
            sep = 1;                                                                     \
        }                                                                                \
    }
#define ICM_END(prf)                                                                     \
    fprintf (outfile, ")\n */\n");                                                       \
    }
#endif

#define ICM_DEF(prf, trf)                                                                \
    if (trf & traceflag) {                                                               \
        INDENT;                                                                          \
        fprintf (outfile, "fprintf( stderr, \"%%s\\n\", \"%s\");\n", #prf);
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)                                                                     \
    fprintf (outfile, "\n");                                                             \
    }                                                                                    \
    ;

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
