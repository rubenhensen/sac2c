#ifndef _SAC_SCOPERCE_H_
#define _SAC_SCOPERCE_H_

#include "types.h"

/******************************************************************************
 *
 * $Id: explicitcopy.h 14536 2006-01-13 14:39:57Z ktr $
 *
 * Scope-based reuse candidate elimination traversal (srce_tab)
 *
 * Prefix: SRCE
 *
 *****************************************************************************/
extern node *SRCEdoRemoveReuseCandidate (node *syntax_tree);

extern node *SRCEfundef (node *arg_node, info *arg_info);
extern node *SRCEarg (node *arg_node, info *arg_info);
extern node *SRCEassign (node *arg_node, info *arg_info);
extern node *SRCElet (node *arg_node, info *arg_info);
extern node *SRCEids (node *arg_node, info *arg_info);
extern node *SRCEap (node *arg_node, info *arg_info);
extern node *SRCEcode (node *arg_node, info *arg_info);
extern node *SRCEprf (node *arg_node, info *arg_info);
extern node *SRCEexprs (node *arg_node, info *arg_info);

#endif /* _SAC_SCOPERCE_H_ */
