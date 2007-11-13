/*
 * $Id$
 */
#ifndef _SAC_SYMB_WLF_H_
#define _SAC_SYMB_WLF_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Symbolic With-Loop folding  traversal ( swlf_tab)
 *
 * Prefix: SWLF
 *
 *****************************************************************************/
extern node *SWLFdoSymbolicWithLoopFolding (node *fundef);

extern node *SWLFfundef (node *arg_node, info *arg_info);
extern node *SWLFassign (node *arg_node, info *arg_info);
extern node *SWLFwith (node *arg_node, info *arg_info);
extern node *SWLFpart (node *arg_node, info *arg_info);
extern node *SWLFcode (node *arg_node, info *arg_info);
extern node *SWLFids (node *arg_node, info *arg_info);
extern node *SWLFprf (node *arg_node, info *arg_info);

#endif /* _SAC_SYMB_WLF_H_ */
