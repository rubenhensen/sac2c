
/**
 *
 * $Log$
 * Revision 1.1  2004/09/20 19:53:13  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_HELPER_H
#define _SERIALIZE_HELPER_H

#include "types.h"

extern node *AllocateNode (nodetype ntype, int lineno, char *sfile);
extern void FillNode (node *node, ...);

#endif /* _SERIALIZE_HELPER_H */
