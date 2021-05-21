#ifndef _SAC_FILTERRC_H_
#define _SAC_FILTERRC_H_

#include "types.h"

/******************************************************************************
 *
 * Filter reuse candidates traversal (emfrc_tab)
 *
 * prefix: FRC
 *
 *****************************************************************************/
extern node *FRCdoFilterReuseCandidates (node *syntax_tree);
extern node *FRCdoFilterReuseCandidatesWL (node *syntax_tree);
extern node *FRCdoFilterReuseCandidatesPrf (node *syntax_tree);

extern node *FRCap (node *arg_node, info *arg_info);
extern node *FRCarg (node *arg_node, info *arg_info);
extern node *FRCassign (node *arg_node, info *arg_info);
extern node *FRCbreak (node *arg_node, info *arg_info);
extern node *FRCcode (node *arg_node, info *arg_info);
extern node *FRCcond (node *arg_node, info *arg_info);
extern node *FRCfold (node *arg_node, info *arg_info);
extern node *FRCfuncond (node *arg_node, info *arg_info);
extern node *FRCfundef (node *arg_node, info *arg_info);
extern node *FRCgenarray (node *arg_node, info *arg_info);
extern node *FRCid (node *arg_node, info *arg_info);
extern node *FRCmodarray (node *arg_node, info *arg_info);
extern node *FRCprf (node *arg_node, info *arg_info);
extern node *FRCrange (node *arg_node, info *arg_info);
extern node *FRCwith (node *arg_node, info *arg_info);
extern node *FRCwith2 (node *arg_node, info *arg_info);
extern node *FRCwith3 (node *arg_node, info *arg_info);

#endif /* _SAC_FILTERRC_H_ */
