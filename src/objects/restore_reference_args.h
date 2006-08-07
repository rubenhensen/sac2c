/* $Id$ */

#ifndef _SAC_RESTORE_REFERENCE_ARGS_H_
#define _SAC_RESTORE_REFERENCE_ARGS_H_

#include "types.h"

extern node *RERAassign (node *arg_node, info *arg_info);
extern node *RERAlet (node *arg_node, info *arg_info);
extern node *RERAap (node *arg_node, info *arg_info);
extern node *RERAid (node *arg_node, info *arg_info);
extern node *RERAids (node *arg_node, info *arg_info);
extern node *RERAreturn (node *arg_node, info *arg_info);
extern node *RERAfundef (node *arg_node, info *arg_info);
extern node *RERAprf (node *arg_node, info *arg_info);
extern node *RERAmodule (node *arg_node, info *arg_info);
extern node *RERAwith (node *arg_node, info *arg_info);
extern node *RERAwith2 (node *arg_node, info *arg_info);
extern node *RERAblock (node *arg_node, info *arg_info);

extern node *RERAdoRestoreReferenceArgs (node *syntax_tree);

#endif /* _SAC_RESTORE_REFERENCE_ARGS_H_ */
