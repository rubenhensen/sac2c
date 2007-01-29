/*
 *
 * $Log$
 * Revision 1.4  2005/04/26 17:11:46  sah
 * errors are now propagated from libmanager to modmanager
 * and handele there. This allows for more precise error
 * messages.
 *
 * Revision 1.3  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.2  2004/09/23 21:15:39  sah
 * interface complete and
 * working implementation for Solaris
 *
 * Revision 1.1  2004/09/21 17:54:05  sah
 * Initial revision
 *
 *
 *
 */

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
extern const char *LIBMgetError ();
extern dynlib_t LIBMloadLibrary (const char *name);
extern dynlib_t LIBMunLoadLibrary (dynlib_t lib);
extern dynfun_t LIBMgetLibraryFunction (const char *name, dynlib_t lib);

#endif /* _SAC_LIBMANAGER_H_ */
