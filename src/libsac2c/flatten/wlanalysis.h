#ifndef _SAC_WLANALYSIS_H_
#define _SAC_WLANALYSIS_H_

#include "types.h"

typedef enum { GPT_empty, GPT_full, GPT_partial, GPT_unknown } gen_prop_t;

/******************************************************************************
 *
 * With-Loop analysis traversal
 *
 * Prefix: WLA
 *
 *****************************************************************************/
extern node *WLAdoWlAnalysis (node *wl, node *fundef, node *let, node **nassigns,
                              gen_prop_t *gprop);

extern node *WLAwith (node *arg_node, info *arg_info);
extern node *WLApart (node *arg_node, info *arg_info);
extern node *WLAgenerator (node *arg_node, info *arg_info);
extern node *WLAgenarray (node *arg_node, info *arg_info);

#endif /* _SAC_WLANALYSIS_H_ */
