
/**
 *
 * $Log$
 * Revision 1.4  2004/10/26 09:35:21  sah
 * added  SHLPFixLink, removed SHLPLookupFunction
 *
 * Revision 1.3  2004/09/27 13:18:12  sah
 * implemented new serialization scheme
 *
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/20 19:53:13  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_HELPER_H
#define _SERIALIZE_HELPER_H

#include "attribs.h"
#include "serialize_stack.h"

extern node *SHLPMakeNode (nodetype ntype, int lineno, char *sfile, ...);
extern void SHLPFixLink (serstack_t *stack, int from, int pos, int to);

#endif /* _SERIALIZE_HELPER_H */
