/*
 * $Log$
 * Revision 1.6  2004/10/26 09:35:55  sah
 * added serialization of links
 *
 * Revision 1.5  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.4  2004/10/11 17:00:28  sah
 * added SerializeBuildStack
 *
 * Revision 1.3  2004/09/24 20:21:53  sah
 * intermediate version
 *
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/20 19:55:28  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "symboltable.h"
#include "serialize_stack.h"
#include <stdio.h>

extern void SerializeModule (node *module);
extern void SerializeFundefLink (node *fundef, FILE *file);
extern const char *GenerateSerFunName (STentrytype_t type, node *node);

extern serstack_t *SerializeBuildSerStack (node *arg_node);

extern node *SERFundef (node *arg_node, info *arg_info);

#endif /* _SERIALIZE_H */
