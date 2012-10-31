/*
 * $Id$
 */
#ifndef _SAC_REORDER_EQUALITYPRF_ARGUMENTS_H_
#define _SAC_REORDER_EQUALITYPRF_ARGUMENTS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Reorder arguments of primitive function _eq_XxX
 *
 * Prefix: REA
 *
 *****************************************************************************/
extern node *REAdoReorderEqualityprfArguments (node *argnode);
extern node *REAblock (node *arg_node, info *arg_info);
extern node *REAassign (node *arg_node, info *arg_info);
extern node *REAlet (node *arg_node, info *arg_info);
extern node *REAprf (node *arg_node, info *arg_info);
extern node *REAfundef (node *arg_node, info *arg_info);
extern node *REAmodule (node *arg_node, info *arg_info);

#endif /* _SAC_REORDER_EQUALITYPRF_ARGUMENTS_H_ */
