/*
 *
 * $Log$
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

#ifndef _LIBMANAGER_H
#define _LIBMANAGER_H

#include "types.h"

typedef void *dynlib_t;
typedef void *dynfun_t;

extern dynlib_t LoadLibrary (const char *name);
extern dynlib_t UnLoadLibrary (dynlib_t lib);
extern dynfun_t GetLibraryFunction (const char *name, dynlib_t lib);

#endif /* _LIBMANAGER_H */
