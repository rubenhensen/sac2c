/*
 * $Log$
 * Revision 1.4  2005/08/26 12:29:13  ktr
 * major brushing,seams to work
 *
 * Revision 1.3  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 1.2  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2001/05/15 15:41:17  nmw
 * Initial revision
 *
 *
 * created from WLF.h, Revision 3.1 on 2001/05/15 by nmw
 *
 */

#ifndef _SAC_WLF_H_
#define _SAC_WLF_H_

#include "types.h"

extern node *WLFdoWLF (node *arg_node);

extern node *WLFmodule (node *arg_node, info *arg_info);
extern node *WLFfundef (node *arg_node, info *arg_info);
extern node *WLFassign (node *arg_node, info *arg_info);
extern node *WLFid (node *arg_node, info *arg_info);
extern node *WLFwith (node *arg_node, info *arg_info);
extern node *WLFlet (node *arg_node, info *arg_info);
extern node *WLFcode (node *arg_node, info *arg_info);

#endif /* _SAC_SSAWLF_H_ */
