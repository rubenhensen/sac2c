/*
 *
 * $Log$
 * Revision 3.6  2002/10/10 23:51:08  dkr
 * ICM_STR added
 *
 * Revision 3.5  2002/07/10 19:25:03  dkr
 * several ICM_... types added and renamed
 *
 * Revision 3.4  2002/07/10 16:24:05  dkr
 * ICM_ANY added, ICM_VAR renamed into ICM_VARANY
 *
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
#define ICM_ANY(name) static char *name;
#define ICM_ICM(name) ICM_ANY (name)
#define ICM_NT(name) ICM_ANY (name)
#define ICM_ID(name) ICM_ANY (name)
#define ICM_STR(name) ICM_ANY (name)
#define ICM_INT(name) static int name;
#define ICM_VARANY(cnt, name) static char **name;
#define ICM_VARNT(cnt, name) ICM_VARANY (cnt, name)
#define ICM_VARID(cnt, name) ICM_VARANY (cnt, name)
#define ICM_VARINT(cnt, name) static int *name;
#define ICM_END(prf, args)

#include "icm.data"

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
