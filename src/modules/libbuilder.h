/*
 *
 * $Log$
 * Revision 1.2  2004/11/07 18:05:01  sah
 * improved dependency handling
 * for external function added
 *
 * Revision 1.1  2004/10/17 17:47:40  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _LIBBUILDER_H
#define _LIBBUILDER_H

#include "stringset.h"

extern void CreateLibrary (stringset_t *deps);

#endif /* _LIBBUILDER_H */
