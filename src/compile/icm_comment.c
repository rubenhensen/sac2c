/*
 *
 * $Log$
 * Revision 3.9  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 3.8  2002/10/10 23:51:08  dkr
 * ICM_STR added
 *
 * Revision 3.7  2002/07/10 19:24:48  dkr
 * several ICM_... types added and renamed
 *
 * Revision 3.6  2002/07/10 16:24:11  dkr
 * ICM_ANY added, ICM_VAR renamed into ICM_VARANY
 *
 * Revision 3.5  2002/07/02 14:03:50  dkr
 * comments are printed in top-level ICMs only now
 *
 * Revision 3.4  2002/06/07 15:44:48  dkr
 * ICM_DEF: one \n removed
 *
 * Revision 3.3  2002/03/07 20:11:52  dkr
 * Support for ICMs arguments of type N_icm (H-ICMs with str-, int-, var- or
 * varint-arguments only) added (ICM_ICM).
 * This feature is not just yet, so it might contain several bugs...
 *
 * Revision 3.1  2000/11/20 18:01:22  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:47  sacbase
 * new release made
 *
 * Revision 1.9  1998/06/23 12:51:18  cg
 * implemented new ICM argument type VARINT for a variable number
 * of integer arguments.
 *
 * Revision 1.8  1998/05/28 16:25:17  dkr
 * ICM-comment is indented now
 *
 * Revision 1.7  1998/04/25 16:25:20  sbs
 *  new icm2c / BEtest mechanism implemented!
 *
 * Revision 1.6  1995/12/18 16:30:17  cg
 * small layout change
 *
 * Revision 1.5  1995/05/04  11:42:34  sbs
 * trf inserted in ICM-macros
 *
 * Revision 1.4  1995/04/13  09:11:01  sbs
 * sep=1 was missing in ICM_VAR
 *
 * Revision 1.3  1995/04/11  15:02:33  sbs
 * \b\b eliminated in output
 *
 * Revision 1.2  1995/04/03  13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.1  1995/03/10  17:26:51  sbs
 * Initial revision
 *
 */

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
