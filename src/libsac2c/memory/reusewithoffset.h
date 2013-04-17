#ifndef _SAC_RWO_H_
#define _SAC_RWO_H_

#include "types.h"

/******************************************************************************
 *
 * Offset-aware With-Loop reuse candidate inference
 *
 * Prefix: RWO
 *
 *****************************************************************************/
extern node *RWOdoOffsetAwareReuseCandidateInference (node *syntax_tree);

extern node *RWOprf (node *arg_node, info *arg_info);
extern node *RWOid (node *arg_node, info *arg_info);
extern node *RWOids (node *arg_node, info *arg_info);

extern node *RWOidentifyNoopArray (node *wl);
extern bool RWOisNoopPart (node *part, node *rc, node *withid);
extern node *RWOidentifyOtherPart (node *with, node *rc);
extern node *RWOannotateCopyPart (node *with, node *rc);

#endif /* _SAC_RWO_H_ */
