/*
 *
 * $Log$
 * Revision 1.3  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
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

#ifndef _SAC_LIBBUILDER_H_
#define _SAC_LIBBUILDER_H_

#include "stringset.h"
#include "types.h"

/******************************************************************************
 *
 * Libbuilder
 *
 * Prefix: LIBB
 *
 *****************************************************************************/
extern void LIBBcreateLibrary (stringset_t *deps);

#endif /* _SAC_LIBBUILDER_H_ */
