#ifndef _SAC_CHECK_AND_SIMPLIFY_GENERIC_DEFINITIONS_H_
#define _SAC_CHECK_AND_SIMPLIFY_GENERIC_DEFINITIONS_H_

#include "types.h"

/******************************************************************************
 *
 * Template traversal ( csgd_tab)
 *
 * Prefix: CSGD
 *
 ******************************************************************************/
extern node *CSGDdoCheckAndSimplifyGenericDefinitions (node *syntax_tree);

extern node *CSGDfundef (node *arg_node, info *arg_info);
extern node *CSGDarg (node *arg_node, info *arg_info);
extern node *CSGDret (node *arg_node, info *arg_info);
extern node *CSGDassign (node *arg_node, info *arg_info);
extern node *CSGDcast (node *arg_node, info *arg_info);
extern node *CSGDreturn (node *arg_node, info *arg_info);
extern node *CSGDavis (node *arg_node, info *arg_info);

#endif /* _SAC_CHECK_AND_SIMPLIFY_GENERIC_DEFINITIONS_H_ */
