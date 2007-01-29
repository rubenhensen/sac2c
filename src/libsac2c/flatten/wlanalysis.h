/*
 *
 * $Log$
 * Revision 1.3  2005/06/15 17:50:55  ktr
 * removed WLAmodarray as it only restricted with-loops with AUD result from
 * obtaining a full partition.
 *
 * Revision 1.2  2005/06/15 16:47:36  ktr
 * Some brushing. Modarray with-loops with AUD result and AKS index vector
 * are not yet equipped with full partition.
 *
 * Revision 1.1  2005/04/29 20:51:53  khf
 * Initial revision
 *
 *
 *
 */

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
