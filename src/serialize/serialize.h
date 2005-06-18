/*
 * $Log$
 * Revision 1.4  2005/06/18 18:06:00  sah
 * moved entire dependency handling to dependencies.c
 * the dependency table is now created shortly prior
 * to c code generation
 *
 * Revision 1.3  2004/11/27 01:52:55  ktr
 * fixed
 *
 * Revision 1.2  2004/11/23 23:21:51  sah
 * COMPILES!
 *
 * Revision 1.1  2004/11/23 22:40:51  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#include "symboltable.h"
#include "serialize_stack.h"
#include <stdio.h>

extern node *SERdoSerialize (node *module);

extern void SERserializeFundefLink (node *fundef, FILE *file);
extern const char *SERgenerateSerFunName (stentrytype_t type, node *node);

extern serstack_t *SERbuildSerStack (node *arg_node);

extern node *SERfundef (node *arg_node, info *arg_info);
extern node *SERtypedef (node *arg_node, info *arg_info);
extern node *SERobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_SERIALIZE_H_ */
