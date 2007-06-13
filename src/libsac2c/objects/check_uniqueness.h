/* $Id$ */

#ifndef _SAC_CHECK_UNIQUENESS_H_
#define _SAC_CHECK_UNIQUENESS_H_

#include "types.h"

extern node *CUavis (node *arg_node, info *arg_info);
extern node *CUblock (node *arg_node, info *arg_info);
extern node *CUcode (node *arg_node, info *arg_info);
extern node *CUcond (node *arg_node, info *arg_info);
extern node *CUfuncond (node *arg_node, info *arg_info);
extern node *CUfundef (node *arg_node, info *arg_info);
extern node *CUid (node *arg_node, info *arg_info);
extern node *CUprf (node *arg_node, info *arg_info);
/*
extern node *CUpropagate( node *arg_node, info *arg_info);
*/
extern node *CUdoCheckUniqueness (node *syntax_tree);

#endif /* _SAC_CHECK_UNIQUENESS_H_ */
