/*
 * $Id: symbi_wlf.h 16002 2009-02-18 22:03:23Z rbe $
 */
#ifndef _SAC_SYMB_WLFI_H_
#define _SAC_SYMB_WLFI_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Symbolic With-Loop-Folding-Inference traversal
 *
 * Prefix: SWLFI
 *
 *****************************************************************************/
extern node *SWLFIdoSymbolicWithLoopFolding (node *arg_node);

extern node *SWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns,
                                     node *restypeavis);
extern node *SWLFIfundef (node *arg_node, info *arg_info);
extern node *SWLFIblock (node *arg_node, info *arg_info);
extern node *SWLFIassign (node *arg_node, info *arg_info);
extern node *SWLFIlet (node *arg_node, info *arg_info);
extern node *SWLFIcond (node *arg_node, info *arg_info);
extern node *SWLFIid (node *arg_node, info *arg_info);
extern node *SWLFIwith (node *arg_node, info *arg_info);
extern node *SWLFIpart (node *arg_node, info *arg_info);
extern node *SWLFIcode (node *arg_node, info *arg_info);
extern node *SWLFImodarray (node *arg_node, info *arg_info);
extern node *SWLFIgenerator (node *arg_node, info *arg_info);
extern node *SWLFIids (node *arg_node, info *arg_info);
extern node *SWLFIprf (node *arg_node, info *arg_info);
extern node *SWLFIavis (node *arg_node, info *arg_info);

#endif /* _SAC_SYMB_WLFI_H_ */
