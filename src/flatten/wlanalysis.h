/*
 *
 * $Log$
 * Revision 1.1  2005/04/29 20:51:53  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_WLANALYSIS_H_
#define _SAC_WLANALYSIS_H_

#include "types.h"

/******************************************************************************
 *
 * With-Loop analysis traversal
 *
 * Prefix: WLA
 *
 *****************************************************************************/
extern node *WLAdoWlAnalysis (node *wl, node *fundef, node *let, node **nassigns,
                              int *gprop);

extern node *WLAwith (node *arg_node, info *arg_info);
extern node *WLApart (node *arg_node, info *arg_info);
extern node *WLAgenerator (node *arg_node, info *arg_info);
extern node *WLAgenarray (node *arg_node, info *arg_info);
extern node *WLAmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_WLANALYSIS_H_ */
