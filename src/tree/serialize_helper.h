
/**
 *
 * $Log$
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

extern node *SHLPAllocateNode (nodetype ntype, int lineno, char *sfile);
extern void SHLPFillNode (node *node, ...);

#endif /* _SERIALIZE_HELPER_H */
