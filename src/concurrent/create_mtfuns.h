/*****************************************************************************
 *
 * $Id:$
 *
 * file: create_mtfuns.h
 *
 * prefix: CMTF
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_MTFUNS_H_
#define _SAC_CREATE_MTFUNS_H_

#include "types.h"

extern node *CMTFdoCreateMtFuns (node *syntax_tree);

extern node *CMTFmodule (node *arg_node, info *arg_info);
extern node *CMTFfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_MTFUNS_H_ */
