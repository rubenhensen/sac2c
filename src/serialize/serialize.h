/* $Id$ */

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#define SAC_SERIALIZE_VERSION 2

#include "symboltable.h"
#include "serialize_stack.h"
#include <stdio.h>

extern node *SERdoSerialize (node *module);

extern void SERserializeFundefLink (node *fundef, FILE *file);
extern void SERserializeObjdefLink (node *fundef, FILE *file);
extern const char *SERgenerateSerFunName (stentrytype_t type, node *node);

extern serstack_t *SERbuildSerStack (node *arg_node);

extern node *SERfundef (node *arg_node, info *arg_info);
extern node *SERtypedef (node *arg_node, info *arg_info);
extern node *SERobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_SERIALIZE_H_ */
