/*
 *
 * $Log$
 * Revision 1.8  2004/11/24 17:57:18  sah
 * COMPILES.
 *
 * Revision 1.7  2004/11/23 21:18:06  ktr
 * fixed some type names.
 *
 * Revision 1.6  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.5  2004/11/07 18:06:28  sah
 * added support for different stringkinds
 *
 * Revision 1.4  2004/11/04 14:53:43  sah
 * implemented dependencies between modules
 *
 * Revision 1.3  2004/11/01 21:49:59  sah
 * added SSPrint
 *
 * Revision 1.2  2004/10/28 17:22:01  sah
 * moved the typedef to types.h
 *
 * Revision 1.1  2004/10/27 13:10:51  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_STRINGSET_H_
#define _SAC_STRINGSET_H_

#include "types.h"

/******************************************************************************
 *
 * String set
 *
 * Prefix: STRS
 *
 *****************************************************************************/
extern bool STRScontains (const char *string, stringset_t *set);
extern stringset_t *STRSadd (const char *string, strstype_t kind, stringset_t *set);
extern void *STRSfold (strsfoldfun_p fun, stringset_t *set, void *init);
extern stringset_t *STRSjoin (stringset_t *one, stringset_t *two);
extern stringset_t *STRSfree (stringset_t *set);
extern void STRSprint (stringset_t *set);

#endif /* _SAC_STRINGSET_H_ */
