/*
 *
 * $Log$
 * Revision 1.3  1995/04/11 15:02:33  sbs
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

#define ICM_DEF(prf)                                                                     \
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
        }                                                                                \
    }
#define ICM_END(prf)                                                                     \
    fprintf (outfile, ")\n */\n");                                                       \
    }

#include "icm.data"

#undef SEP
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
