/*
 * $Id$
 */
#ifndef _SAC_RWR_H_
#define _SAC_RWR_H_

#include "types.h"

/******************************************************************************
 *
 * Region-aware With-Loop reuse candidate inference
 *
 * Prefix: RWR
 *
 *****************************************************************************/
extern node *RWRdoRegionAwareReuseCandidateInference (node *with, node *fundef);
extern node *RWRprf (node *arg_node, info *arg_info);
extern node *RWRpart (node *arg_node, info *arg_info);
extern node *RWRassign (node *arg_node, info *arg_info);
extern node *RWRwith (node *arg_node, info *arg_info);
extern node *RWRap (node *arg_node, info *arg_info);
extern node *RWRcond (node *arg_node, info *arg_info);
extern node *RWRfundef (node *arg_node, info *arg_info);

#endif /* _SAC_RWR_H_ */
