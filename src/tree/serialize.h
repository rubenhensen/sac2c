/*
 * $Log$
 * Revision 1.8  2004/11/22 21:24:02  skt
 * code brushing SACDevCampDK 2K4
 *
 * Revision 1.7  2004/11/02 14:59:01  sah
 * extended serialize traversal
 *
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

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#include "symboltable.h"
#include "serialize_stack.h"
#include <stdio.h>

extern void SERdoSerialize (node *module);

extern void SERserializeFundefLink (node *fundef, FILE *file);
extern const char *SERgenerateSerFunName (STentrytype_t type, node *node);

extern serstack_t *SERbuildSerStack (node *arg_node);

extern node *SERfundef (node *arg_node, info *arg_info);
extern node *SERtypedef (node *arg_node, info *arg_info);
extern node *SERobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_SERIALIZE_H_ */
