/*
 *
 * $Log$
 * Revision 1.2  1995/04/03 13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.1  1995/03/10  17:26:51  sbs
 * Initial revision
 *
 *
 */

#define ICM_DEF(prf) fprintf (outfile, "/*\n * %s( ", #prf);
#define ICM_STR(name) fprintf (outfile, "%s, ", name);
#define ICM_INT(name) fprintf (outfile, "%d, ", name);
#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < dim; i++) {                                                      \
            AccessConst (name, i);                                                       \
            fprintf (outfile, ", ");                                                     \
        }                                                                                \
    }
#define ICM_END(prf) fprintf (outfile, "\b\b)\n */\n");

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
