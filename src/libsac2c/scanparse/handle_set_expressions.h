#ifndef _HANDLE_SET_EXPRESSIONS_H_
#define _HANDLE_SET_EXPRESSIONS_H_

#include "types.h"

extern node *HSEdoEliminateSetExpressions (node *arg_node);
extern node *HSEassign (node *arg_node, info *arg_info);
extern node *HSEcode (node *arg_node, info *arg_info);
extern node *HSEgenerator (node *arg_node, info *arg_info);
extern node *HSEsetwl (node *arg_node, info *arg_info);

#endif /* _HANDLE_SET_EXPRESSIONS_H_ */
