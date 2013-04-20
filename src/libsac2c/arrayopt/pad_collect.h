/*****************************************************************************
 *
 * file:   pad_collect.h
 *
 * prefix: APC
 *
 * description:
 *
 *   This compiler module collects information needed to infer new array
 *   shapes for the inference-phase.
 *
 *
 *****************************************************************************/

#ifndef _SAC_PAD_COLLECT_H_
#define _SAC_PAD_COLLECT_H_

#include "types.h"

extern void APCdoCollect (node *arg_node);
extern node *APCarray (node *arg_node, info *arg_info);
extern node *APCwith (node *arg_node, info *arg_info);
extern node *APCap (node *arg_node, info *arg_info);
extern node *APCid (node *arg_node, info *arg_info);
extern node *APCprf (node *arg_node, info *arg_info);
extern node *APCfundef (node *arg_node, info *arg_info);
extern node *APClet (node *arg_node, info *arg_info);
extern node *APCgenarray (node *arg_node, info *arg_info);
extern node *APCmodarray (node *arg_node, info *arg_info);
extern node *APCfold (node *arg_node, info *arg_info);
extern node *APCcode (node *arg_node, info *arg_info);

#endif /* _SAC_PAD_COLLECT_H_ */
