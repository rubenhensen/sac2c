/*
 * $Log$
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/21 10:18:51  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_STACK_H
#define _SERIALIZE_STACK_H

#include "types.h"

typedef struct SERSTACK_T serstack_t;

extern serstack_t *SerStackInit ();
extern serstack_t *SerStackDestroy (serstack_t *stack);
extern void SerStackPush (node *val, serstack_t *stack);
extern node *SerStackPop (serstack_t *stack);
extern int SerStackFindPos (node *val, serstack_t *stack);
extern node *SerStackLookup (int pos, serstack_t *stack);

#endif /* _SERIALIZE_STACK_H */
