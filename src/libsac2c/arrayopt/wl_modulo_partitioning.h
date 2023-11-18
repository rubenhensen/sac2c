#ifndef _SAC_WL_MODULO_PARTITIONING_H_
#define _SAC_WL_MODULO_PARTITIONING_H_

/*******************************************************************************
 * 
 * For an in-depth explanation of this module, see wl_modulo_partitioning.c
 * 
 * Prefix: WLMP
 * 
 ******************************************************************************/

node *WLMPdoWithLoopModuloPartitioning (node *arg_node);

node *WLMPfundef (node *arg_node, info *arg_info);
node *WLMPassign (node *arg_node, info *arg_info);
node *WLMPwith (node *arg_node, info *arg_info);
node *WLMPpart (node *arg_node, info *arg_info);
node *WLMPprf (node *arg_node, info *arg_info);

#endif /* _SAC_WL_MODULO_PARTITIONING_H_ */
