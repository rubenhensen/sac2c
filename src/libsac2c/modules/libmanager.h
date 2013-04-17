#ifndef _SAC_LIBMANAGER_H_
#define _SAC_LIBMANAGER_H_

#include "types.h"

/******************************************************************************
 *
 * Libmanager
 *
 * Prefix: LIBM
 *
 *****************************************************************************/
extern const char *LIBMgetError (void);
extern dynlib_t LIBMloadLibrary (const char *name);
extern dynlib_t LIBMunLoadLibrary (dynlib_t lib);
extern dynfun_t LIBMgetLibraryFunction (const char *name, dynlib_t lib);

#endif /* _SAC_LIBMANAGER_H_ */
