/*
 * $Id$
 */

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#define SAC_SERIALIZE_VERSION 5 /* 27/11/2009 */

#include "types.h"

extern node *SERdoSerialize (node *module);

extern void SERserializeFundefLink (node *fundef, FILE *file);
extern void SERserializeObjdefLink (node *fundef, FILE *file);
extern char *SERgenerateSerFunName (stentrytype_t type, node *node);

extern serstack_t *SERbuildSerStack (node *arg_node);

extern node *SERfundef (node *arg_node, info *arg_info);
extern node *SERtypedef (node *arg_node, info *arg_info);
extern node *SERobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_SERIALIZE_H_ */
