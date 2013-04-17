/*****************************************************************************
 *
 * file: create_mtst_funs.h
 *
 * prefix: MTSTF
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_MTST_FUNS_H_
#define _SAC_CREATE_MTST_FUNS_H_

#include "types.h"

extern node *MTSTFdoCreateMtFuns (node *syntax_tree);

extern node *MTSTFmodule (node *arg_node, info *arg_info);
extern node *MTSTFfundef (node *arg_node, info *arg_info);
extern node *MTSTFassign (node *arg_node, info *arg_info);
extern node *MTSTFap (node *arg_node, info *arg_info);
extern node *MTSTFfold (node *arg_node, info *arg_info);
extern node *MTSTFcond (node *arg_node, info *arg_info);
extern node *MTSTFwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_MTST_FUNS_H_ */
