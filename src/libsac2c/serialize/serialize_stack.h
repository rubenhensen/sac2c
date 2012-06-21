/*
 * $Log$
 * Revision 1.1  2004/11/23 22:41:12  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SERIALIZE_STACK_H_
#define _SAC_SERIALIZE_STACK_H_

#include "types.h"

#define SERSTACK_NOT_FOUND -1

extern serstack_t *SSinit (void);
extern serstack_t *SSdestroy (serstack_t *stack);
extern void SSpush (node *val, serstack_t *stack);
extern node *SSpop (serstack_t *stack);
extern int SSfindPos (node *val, serstack_t *stack);
extern node *SSlookup (int pos, serstack_t *stack);
extern void SSdump (serstack_t *stack);

#endif /* _SAC_SERIALIZE_STACK_H_ */
