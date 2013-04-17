/******************************************************************************
 *
 * Dead code inference traversal (dci_tab)
 *
 * prefix: DCI
 *
 *****************************************************************************/

#ifndef _SAC_DEADCODEINFERENCE_H_
#define _SAC_DEADCODEINFERENCE_H_

#include "types.h"

extern node *DCIdoDeadCodeInferenceOneFundef (node *arg_node);
extern node *DCIdoDeadCodeInferenceOneFunction (node *arg_node);

extern node *DCIfundef (node *arg_node, info *arg_info);
extern node *DCIarg (node *arg_node, info *arg_info);
extern node *DCIvardec (node *arg_node, info *arg_info);
extern node *DCIblock (node *arg_node, info *arg_info);
extern node *DCIassign (node *arg_node, info *arg_info);
extern node *DCIreturn (node *arg_node, info *arg_info);
extern node *DCIcond (node *arg_node, info *arg_info);
extern node *DCIlet (node *arg_node, info *arg_info);
extern node *DCIap (node *arg_node, info *arg_info);
extern node *DCIid (node *arg_node, info *arg_info);
extern node *DCIids (node *arg_node, info *arg_info);
extern node *DCIcode (node *arg_node, info *arg_info);
extern node *DCIwithid (node *arg_node, info *arg_info);
extern node *DCIrange (node *arg_node, info *arg_info);

#endif /* _SAC_DEADCODEINFERENCE_H_ */
