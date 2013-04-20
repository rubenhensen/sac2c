/*****************************************************************************
 *
 * file: create_mtst_funs_module.h
 *
 * prefix: MTSTFMOD
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_MTST_FUNS_MODULE_H_
#define _SAC_CREATE_MTST_FUNS_MODULE_H_

#include "types.h"

extern node *MTSTFMODdoCreateMtStFunsModule (node *syntax_tree);

extern node *MTSTFMODmodule (node *arg_node, info *arg_info);
extern node *MTSTFMODfundef (node *arg_node, info *arg_info);
extern node *MTSTFMODassign (node *arg_node, info *arg_info);
extern node *MTSTFMODap (node *arg_node, info *arg_info);
extern node *MTSTFMODfold (node *arg_node, info *arg_info);
extern node *MTSTFMODcond (node *arg_node, info *arg_info);
extern node *MTSTFMODwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_MTST_FUNS_MODULE_H_ */
