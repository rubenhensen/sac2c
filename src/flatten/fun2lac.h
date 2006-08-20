/*
 * $Id$
 */

#ifndef _SAC_FUN2LAC_H_
#define _SAC_FUN2LAC_H_

#include "types.h"

/*****************************************************************************
 *
 * Fun2lac traversal ( fun2lac_tab)
 *
 * prefix: F2L
 *
 * description:
 *
 *   header file for fun2lac.c.
 *
 *****************************************************************************/

extern node *F2LdoFun2Lac (node *syntax_tree);

extern node *F2Lmodule (node *arg_node, info *arg_info);
extern node *F2Lfundef (node *arg_node, info *arg_info);

extern node *F2Lassign (node *arg_node, info *arg_info);
extern node *F2Lcond (node *arg_node, info *arg_info);
extern node *F2Larg (node *arg_node, info *arg_info);

#endif /* _SAC_FUN2LAC_H_ */
