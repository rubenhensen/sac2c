#define SEP                                                                              \
    if (sep) {                                                                           \
        INDENT;                                                                          \
        fprintf (global.outfile, "SAC_Print( \", \");\n");                               \
    }

#define ICM_TRACE_NONE FALSE

#define ICM_TRACE(flag) global.trace.flag

#define ICM_DEF(prf, trf)                                                                \
    if (trf) {                                                                           \
        int sep = 0;                                                                     \
        INDENT;                                                                          \
        fprintf (global.outfile, "SAC_Print( \"%s( \");\n", #prf);

#define ICM_ANY(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_Print( \"%s \");\n", name);                            \
    sep = 1;

#define ICM_ICM(name) ICM_ANY (name)

#define ICM_NT(name) ICM_ANY (name)

#define ICM_ID(name) ICM_ANY (name)

#define ICM_STR(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_Print( \"\\\"%s \"\\\");\n", name);                    \
    sep = 1;

#define ICM_INT(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_Print( \"%d \");\n", name);                            \
    sep = 1;

#define ICM_UINT(name)                                                                    \
    SEP;                                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_Print( \"%u \");\n", name);                            \
    sep = 1;

#define ICM_BOOL(name) ICM_INT (name)

#define ICM_VARANY(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                        \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_ANY (name[i])                                                            \
        }                                                                                \
    }

#define ICM_VARNT(dim, name)                                                             \
    {                                                                                    \
        int i;                                                                        \
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
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_Print( \")\\n\");\n");                                 \
    }

#include "icm.data"

#undef SEP

#undef ICM_DEF
#undef ICM_ANY
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
#undef ICM_TRACE_NONE
#undef ICM_TRACE
