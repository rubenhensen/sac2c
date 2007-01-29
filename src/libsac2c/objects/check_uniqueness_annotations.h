/* $Id$ */

#ifndef _SAC_CHECK_UNIQUENESS_ANNOTATIONS_H_
#define _SAC_CHECK_UNIQUENESS_ANNOTATIONS_H_

#include "types.h"

extern node *CUAobjdef (node *arg_node, info *arg_info);
extern node *CUAarg (node *arg_node, info *arg_info);
extern node *CUAret (node *arg_node, info *arg_info);

extern node *CUAdoCheckUniquenessAnnotations (node *syntax_tree);

#endif /* _SAC_CHECK_UNIQUENESS_ANNOTATIONS_H_ */
