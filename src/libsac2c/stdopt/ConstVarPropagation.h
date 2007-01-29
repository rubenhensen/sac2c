/*
 * $Id$
 */

#ifndef _SAC_CONSTVARPROPAGATION_H_
#define _SAC_CONSTVARPROPAGATION_H_

#include "types.h"

/******************************************************************************
 *
 * Constant variable propagation traversal ( cvp_tab)
 *
 * Prefix: CVP
 *
 *****************************************************************************/
extern node *CVPdoConstVarPropagation (node *arg_node);
extern node *CVPdoConstVarPropagationOneFundef (node *arg_node);

extern node *CVPfundef (node *arg_node, info *arg_info);
extern node *CVPassign (node *arg_node, info *arg_info);
extern node *CVPlet (node *arg_node, info *arg_info);
extern node *CVPcode (node *arg_node, info *arg_info);
extern node *CVPcond (node *arg_node, info *arg_info);
extern node *CVPgenerator (node *arg_node, info *arg_info);
extern node *CVPgenarray (node *arg_node, info *arg_info);
extern node *CVPmodarray (node *arg_node, info *arg_info);
extern node *CVPfold (node *arg_node, info *arg_info);
extern node *CVPap (node *arg_node, info *arg_info);
extern node *CVPprf (node *arg_node, info *arg_info);
extern node *CVPid (node *arg_node, info *arg_info);
extern node *CVPreturn (node *arg_node, info *arg_info);
extern node *CVParray (node *arg_node, info *arg_info);
extern node *CVPfuncond (node *arg_node, info *arg_info);
extern node *CVPavis (node *arg_node, info *arg_info);

#endif /* _SAC_CONSTVARPROPAGATION_H_ */
