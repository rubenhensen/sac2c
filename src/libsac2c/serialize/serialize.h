#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#define SAC_SERIALIZE_VERSION 6 /* 14/12/2009 */

#include "types.h"

extern node *SERdoSerialize (node *module);

extern void SERserializeFundefLink (node *fundef, FILE *file);
extern void SERserializeObjdefLink (node *fundef, FILE *file);
extern char *SERfundefHeadSymbol2BodySymbol (const char *symbol);
extern char *SERgetSerFunName (node *arg_node);

extern serstack_t *SERbuildSerStack (node *arg_node);

extern node *SERfundef (node *arg_node, info *arg_info);
extern node *SERtypedef (node *arg_node, info *arg_info);
extern node *SERobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_SERIALIZE_H_ */
