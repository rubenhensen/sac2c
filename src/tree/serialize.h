/*
 * $Log$
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

extern void SerializeModule (node *module);

extern const char *GenerateSerFunName (symbolentrytype_t type, node *node);
extern node *SerializeLookupFunction (const char *module, const char *name);
extern void SerializeFundefHead (node *fundef);
extern void SerializeFundefBody (node *fundef);

extern node *SERFundef (node *arg_node, info *arg_info);

#endif /* _SERIALIZE_H */
