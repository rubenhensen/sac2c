/*
 *
 * $Log$
 * Revision 1.2  1998/06/23 12:51:18  cg
 * implemented new ICM argument type VARINT for a variable number
 * of integer arguments.
 *
 * Revision 1.1  1998/04/25 16:22:03  sbs
 * Initial revision
 *
 *
 *
 */

#define ICM_DEF(prf, trf)
#define ICM_STR(name) static char *name;
#define ICM_INT(name) static int name;
#define ICM_VARINT(dim, name) static int *name;
#define ICM_VAR(dim, name) static char **name;
#define ICM_END(prf, args)

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VARINT
#undef ICM_VAR
#undef ICM_END
