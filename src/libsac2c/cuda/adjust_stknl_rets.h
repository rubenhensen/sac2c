/*****************************************************************************
 *
 * file:   adjust_stknl_rets.h
 *
 * description:
 *   header file for adjust_stknl_rets.c
 *
 *****************************************************************************/

#ifndef _ADJUST_STKNL_RETS_H_
#define _ADJUST_STKNL_RETS_H_

#include "types.h"

extern node *CUASRdoAdjustStknlRets (node *arg_node);
extern node *CUASRmodule (node *arg_node, info *arg_info);
extern node *CUASRfundef (node *arg_node, info *arg_info);
extern node *CUASRassign (node *arg_node, info *arg_info);
extern node *CUASRlet (node *arg_node, info *arg_info);
extern node *CUASRap (node *arg_node, info *arg_info);
extern node *CUASRreturn (node *arg_node, info *arg_info);
extern node *CUASRid (node *arg_node, info *arg_info);
extern node *CUASRids (node *arg_node, info *arg_info);
extern node *CUASRarg (node *arg_node, info *arg_info);

#endif /* _ADJUST_STKNL_RETS_H_ */
