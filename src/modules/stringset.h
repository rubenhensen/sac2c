/*
 *
 * $Log$
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

#ifndef _STRINGSET_H
#define _STRINGSET_H

#include "types.h"

typedef void *(*SSfoldfun_p) (const char *elem, void *rest);

extern bool SSContains (const char *string, stringset_t *set);
extern stringset_t *SSAdd (const char *string, stringset_t *set);
extern void *SSFold (SSfoldfun_p fun, stringset_t *set, void *init);
extern stringset_t *SSJoin (stringset_t *one, stringset_t *two);
extern stringset_t *SSFree (stringset_t *set);
extern void SSPrint (stringset_t *set);

#endif /* _STRINGSET_H */
