/*
 *
 * $Log$
 * Revision 1.2  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/07/21 12:35:36  khf
 * Initial revision
 *
 *
 *
 */

#include "types.h"

#ifndef _SAC_EXPLICITACCUMULATE_H_
#define _SAC_EXPLICITACCUMULATE_H_

extern node *EAdoExplicitAccumulate (node *arg_node);

extern node *EAmodule (node *arg_node, info *arg_info);
extern node *EAfundef (node *arg_node, info *arg_info);
extern node *EAlet (node *arg_node, info *arg_info);

extern node *EAwith (node *arg_node, info *arg_info);
extern node *EAcode (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITACCUMULATE_H_ */
