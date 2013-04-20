#ifndef _SAC_LAC2FUN_H_
#define _SAC_LAC2FUN_H_

#include "types.h"

/******************************************************************************
 *
 * Lac2fun traversal ( l2f_tab)
 *
 * Prefix: L2F
 *
 *****************************************************************************/
extern node *L2FdoLac2Fun (node *syntaxtree);

extern node *L2Ffundef (node *arg_node, info *arg_info);
extern node *L2Fcond (node *arg_node, info *arg_info);
extern node *L2Fdo (node *arg_node, info *arg_info);
extern node *L2Fassign (node *arg_node, info *arg_info);

#endif /* _SAC_LAC2FUN_H_ */
