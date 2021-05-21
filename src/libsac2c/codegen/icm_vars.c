#define ICM_DEF(prf, trf)
#define ICM_ANY(name) static char *name;
#define ICM_ICM(name) ICM_ANY (name)
#define ICM_NT(name) ICM_ANY (name)
#define ICM_ID(name) ICM_ANY (name)
#define ICM_STR(name) ICM_ANY (name)
#define ICM_INT(name) static int name;
#define ICM_UINT(name) static unsigned int name;
#define ICM_BOOL(name) static bool name;
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
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
