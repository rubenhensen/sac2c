/*
 *
 * $Log$
 * Revision 3.7  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 3.6  2004/11/24 18:02:39  jhb
 * compile! only outfile changed to global
 *
 * Revision 3.5  2002/10/10 23:51:08  dkr
 * ICM_STR added
 *
 * Revision 3.4  2002/07/10 19:24:53  dkr
 * several ICM_... types added and renamed
 *
 * Revision 3.3  2002/07/10 16:23:53  dkr
 * ICM_ANY added, ICM_VAR renamed into ICM_VARANY
 *
 * Revision 3.2  2002/03/07 20:12:14  dkr
 * Support for ICMs arguments of type N_icm (H-ICMs with str-, int-, var- or
 * varint-arguments only) added (ICM_ICM).
 * This feature is not just yet, so it might contain several bugs...
 *
 * Revision 3.1  2000/11/20 18:01:24  sacbase
 * new release made
 *
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
 */

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
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
#undef ICM_TRACE_NONE
#undef ICM_TRACE
