/*
 *
 * $Log$
 * Revision 3.3  2002/03/07 20:12:40  dkr
 * Support for ICMs arguments of type N_icm (H-ICMs with str-, int-, var- or
 * varint-arguments only) added (ICM_ICM).
 * This feature is not just yet, so it might contain several bugs...
 *
 * Revision 3.1  2000/11/20 18:01:24  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:50  sacbase
 * new release made
 *
 * Revision 1.2  1998/06/23 12:51:18  cg
 * implemented new ICM argument type VARINT for a variable number
 * of integer arguments.
 *
 * Revision 1.1  1998/04/25 16:22:03  sbs
 * Initial revision
 *
 */

#define ICM_DEF(prf, trf)
#define ICM_ICM(name) static char *name;
#define ICM_STR(name) static char *name;
#define ICM_INT(name) static int name;
#define ICM_VARINT(cnt, name) static int *name;
#define ICM_VAR(cnt, name) static char **name;
#define ICM_END(prf, args)

#include "icm.data"

#undef ICM_DEF
#undef ICM_ICM
#undef ICM_STR
#undef ICM_INT
#undef ICM_VARINT
#undef ICM_VAR
#undef ICM_END
