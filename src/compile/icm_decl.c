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

#define ICM_DEF(prf)                                                                     \
    void Print##prf (node *exprs)                                                        \
    {
#define ICM_STR(name) char *name;
#define ICM_INT(name) int name;
#define ICM_VAR(dim, name) char **name;
#define ICM_END(prf) DBUG_ENTER ("Print" #prf);

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
