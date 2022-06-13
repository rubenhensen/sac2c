#define SEP                                                                              \
    if (sep) {                                                                           \
        fprintf (global.outfile, ", ");                                                  \
    }

#define ICM_DEF(prf, trf)                                                                \
    if (print_comment) {                                                                 \
        int sep = 0;                                                                     \
        print_comment = 0;                                                               \
        fprintf (global.outfile, "/*\n");                                                \
        INDENT;                                                                          \
        fprintf (global.outfile, " * %s( ", #prf);

#define ICM_PRAGMA_FUNS(name)                                                            \
    SEP;                                                                                 \
    fprintf (global.outfile, "%p", name);                                                \
    sep = 1;

#define ICM_ANY(name)                                                                    \
    SEP;                                                                                 \
    fprintf (global.outfile, "%s", name);                                                \
    sep = 1;

#define ICM_ICM(name) ICM_ANY (name)

#define ICM_NT(name) ICM_ANY (name)

#define ICM_ID(name) ICM_ANY (name)

#define ICM_STR(name) ICM_ANY (name)

#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    fprintf (global.outfile, "%d", name);                                                \
    sep = 1;

#define ICM_UINT(name)                                                                    \
    SEP;                                                                                 \
    fprintf (global.outfile, "%u", name);                                                \
    sep = 1;

#define ICM_BOOL(name) ICM_INT (name)

/* dim and i needs to be signed due to function in tree_compound.c
   called TUgetFullDimEncoding as it uses negatives to encode other
   shape information.
 */
#define ICM_VARANY(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_ANY (name[i])                                                            \
        }                                                                                \
    }

#define ICM_VARNT(dim, name)                                                             \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_NT (name[i])                                                             \
        }                                                                                \
    }

#define ICM_VARID(dim, name)                                                             \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_ID (name[i])                                                             \
        }                                                                                \
    }

#define ICM_VARINT(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_INT (name[i])                                                            \
        }                                                                                \
    }

#define ICM_END(prf, args)                                                               \
    fprintf (global.outfile, ")\n");                                                     \
    INDENT;                                                                              \
    fprintf (global.outfile, " */\n");                                                   \
    }

#include "icm.data"

#undef SEP

#undef ICM_DEF
#undef ICM_ANY
#undef ICM_PRAGMA_FUNS
#undef ICM_ICM
#undef ICM_NT
#undef ICM_ID
#undef ICM_STR
#undef ICM_INT
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
