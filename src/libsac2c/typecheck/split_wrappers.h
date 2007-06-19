/*
 * $Log$
 * Revision 1.1  2005/07/17 20:10:25  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_SPLIT_WRAPPERS_H_
#define _SAC_SPLIT_WRAPPERS_H_

#include "types.h"

extern node *SWRdoSplitWrappers (node *ast);

extern node *SWRmodule (node *arg_node, info *arg_info);
extern node *SWRfundef (node *arg_node, info *arg_info);
extern node *SWRap (node *arg_node, info *arg_info);
extern node *SWRwith (node *arg_node, info *arg_info);
extern node *SWRgenarray (node *arg_node, info *arg_info);
extern node *SWRmodarray (node *arg_node, info *arg_info);
extern node *SWRfold (node *arg_node, info *arg_info);
extern node *SWRpropagate (node *arg_node, info *arg_info);

#endif /* _SAC_SPLIT_WRAPPERS_H_ */
