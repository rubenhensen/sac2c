#ifndef _HANDLE_SET_EXPRESSION_DOTS_H_
#define _HANDLE_SET_EXPRESSION_DOTS_H_

#include "types.h"

extern node *HSEDdoEliminateSetExpressionDots (node *arg_node);
extern node *HSEDassign (node *arg_node, info *arg_info);
extern node *HSEDwith (node *arg_node, info *arg_info);
extern node *HSEDcode (node *arg_node, info *arg_info);
extern node *HSEDsetwl (node *arg_node, info *arg_info);
extern node *HSEDgenerator (node *arg_node, info *arg_info);

#endif /* _HANDLE_SET_EXPRESSION_DOTS_H_ */
