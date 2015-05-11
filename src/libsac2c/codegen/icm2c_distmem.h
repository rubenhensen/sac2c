
#ifndef _SAC_ICM2C_DISTMEM_H_
#define _SAC_ICM2C_DISTMEM_H_

#include "types.h"

extern void ICMCompileDISTMEM_DECL (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileDISTMEM_DECL__MIRROR (char *var_NT, int sdim, int *shp);

#endif /* defined(_SAC_ICM2C_DISTMEM_H_) */
