/*
 *
 * $Log$
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

extern dynlib_t LoadLibrary (char *name);
extern dynfun_t GetLibraryFun (char *name);

#endif /* _LIBMANAGER_H */
