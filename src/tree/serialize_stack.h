/*
 * $Log$
 * Revision 1.5  2004/11/22 21:24:02  skt
 * code brushing SACDevCampDK 2K4
 *
 * Revision 1.4  2004/11/01 21:51:43  sah
 * added SerStackDump
 *
 * Revision 1.3  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
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

#ifndef _SAC_SERIALIZE_STACK_H_
#define _SAC_SERIALIZE_STACK_H_

#include "types.h"

#define SERSTACK_NOT_FOUND -1

extern serstack_t *SSinit ();
extern serstack_t *SSdestroy (serstack_t *stack);
extern void SSpush (node *val, serstack_t *stack);
extern node *SSpop (serstack_t *stack);
extern int SSfindPos (node *val, serstack_t *stack);
extern node *SSlookup (int pos, serstack_t *stack);
extern void SSdump (serstack_t *stack);

#endif /* _SAC_SERIALIZE_STACK_H_ */
