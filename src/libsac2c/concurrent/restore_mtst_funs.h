/*****************************************************************************
 *
 * $Id$
 *
 * file: restore_mtst_funs.h
 *
 * prefix: RMTSTF
 *
 *  TODO: rename to restore_stxt_funs
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_STXT_FUNS_H_
#define _SAC_CREATE_STXT_FUNS_H_

#include "types.h"

extern node *RMTSTFdoCreateMtFuns (node *syntax_tree);

extern node *RMTSTFmodule (node *arg_node, info *arg_info);
extern node *RMTSTFfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_STXT_FUNS_H_ */
