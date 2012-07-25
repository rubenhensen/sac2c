/*
 * $Id$
 */
#ifndef _SAC_CUDACOSTMODEL_H_
#define _SAC_CUDACOSTMODEL_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * CUDA Cost Model (cucm)
 *
 * Prefix: CUCM
 *
 *****************************************************************************/
extern node *CUCMdoCUDACostModel (node *syntax_tree);

extern node *CUCMfundef (node *arg_node, info *arg_info);
extern node *CUCMpart (node *arg_node, info *arg_info);
// extern node *CUCMassign        ( node *arg_node, info *arg_info);
extern node *CUCMlet (node *arg_node, info *arg_info);
extern node *CUCMwith (node *arg_node, info *arg_info);
extern node *CUCMgenarray (node *arg_node, info *arg_info);
extern node *CUCMmodarray (node *arg_node, info *arg_info);
extern node *CUCMfold (node *arg_node, info *arg_info);

#endif /* _SAC_CUDACOSTMODEL_H_ */
