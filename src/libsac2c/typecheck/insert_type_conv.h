#ifndef _SAC_INSERT_TYPE_CONV_H_
#define _SAC_INSERT_TYPE_CONV_H_

#include "types.h"

/******************************************************************************
 *
 * Insert Vardec traversal ( insta_tab)
 *
 * Prefix: INSTC
 *
 *****************************************************************************/

extern node *INSTCdoInsertTypeConv (node *syntaxtree);

extern node *INSTCfundef (node *arg_node, info *arg_info);
extern node *INSTCblock (node *arg_node, info *arg_info);
extern node *INSTCavis (node *arg_node, info *arg_info);
extern node *INSTCassign (node *arg_node, info *arg_info);
extern node *INSTCids (node *arg_node, info *arg_info);
extern node *INSTCid (node *arg_node, info *arg_info);
extern node *INSTCwith (node *arg_node, info *arg_info);
extern node *INSTCreturn (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_TYPE_CONV_H_ */
