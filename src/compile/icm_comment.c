/*
 *
 * $Log$
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
 *
 */

#define SEP                                                                              \
    if (sep)                                                                             \
        fprintf (outfile, ", ");

#define ICM_DEF(prf, trf)                                                                \
    {                                                                                    \
        int sep = 0;                                                                     \
        fprintf (outfile, "\n");                                                         \
        INDENT;                                                                          \
        fprintf (outfile, "/*\n");                                                       \
        INDENT;                                                                          \
        fprintf (outfile, " * %s( ", #prf);

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
#define ICM_VARINT(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            SEP;                                                                         \
            fprintf (outfile, "%d", name[i]);                                            \
            sep = 1;                                                                     \
        }                                                                                \
    }

#define ICM_END(prf, args)                                                               \
    fprintf (outfile, ")\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, " */\n");                                                          \
    }

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
