/*
 * $Id$
 */

#ifndef _SAC_RESOLVE_REFERENCE_ARGS_H_
#define _SAC_RESOLVE_REFERENCE_ARGS_H_

#include "types.h"

extern node *RRAmodule (node *arg_node, info *arg_info);
extern node *RRAfundef (node *arg_node, info *arg_info);
extern node *RRAreturn (node *arg_node, info *arg_info);
extern node *RRAlet (node *arg_node, info *arg_info);
extern node *RRAap (node *arg_node, info *arg_info);

extern node *RRAdoResolveReferenceArgs (node *syntax_tree);

#endif /* _SAC_RESOLVE_REFERENCE_ARGS_H_ */
