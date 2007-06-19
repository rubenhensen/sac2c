/*
 *
 * $Log$
 * Revision 1.3  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.2  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/07/21 12:35:36  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_EXPLICITACCUMULATE_H_
#define _SAC_EXPLICITACCUMULATE_H_

#include "types.h"

/******************************************************************************
 *
 * Explicit accumulate traversal ( ea_tab)
 *
 * Prefix: EA
 *
 *****************************************************************************/
extern node *EAdoExplicitAccumulate (node *arg_node);

extern node *EAmodule (node *arg_node, info *arg_info);
extern node *EAfundef (node *arg_node, info *arg_info);
extern node *EAassign (node *arg_node, info *arg_info);
extern node *EAlet (node *arg_node, info *arg_info);

extern node *EAwith (node *arg_node, info *arg_info);
extern node *EApropagate (node *arg_node, info *arg_info);
extern node *EAfold (node *arg_node, info *arg_info);
extern node *EAcode (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITACCUMULATE_H_ */
