/*
 *
 * $Log$
 * Revision 1.8  2005/09/15 17:12:13  ktr
 * remove ICM traversal
 *
 * Revision 1.7  2005/08/24 10:20:09  ktr
 * added support for WITHID_IDXS, some brushing
 *
 * Revision 1.6  2004/11/28 18:14:21  ktr
 * added traversal functions for bool, num, float, double, char
 *
 * Revision 1.5  2004/11/21 20:43:42  ktr
 * Ismop 2004
 *
 * Revision 1.4  2004/07/17 10:26:04  ktr
 * EMAL now uses an INFO structure.
 *
 * Revision 1.3  2004/07/16 12:07:14  ktr
 * EMAL now traverses into N_ap and N_funcond, too.
 *
 * Revision 1.2  2004/07/15 13:39:23  ktr
 * renamed EMALAllocateFill into EMAllocateFill
 *
 * Revision 1.1  2004/07/14 15:27:00  ktr
 * Initial revision
 *
 *
 */

#ifndef _SAC_ALLOC_H_
#define _SAC_ALLOC_H_

#include "types.h"

/******************************************************************************
 *
 * Explicit allocation traversal ( emal_tab)
 *
 * prefix: EMAL
 *
 *****************************************************************************/
extern node *EMALdoAlloc (node *syntax_tree);

extern node *EMALbool (node *arg_node, info *arg_info);
extern node *EMALchar (node *arg_node, info *arg_info);
extern node *EMALdouble (node *arg_node, info *arg_info);
extern node *EMALfloat (node *arg_node, info *arg_info);
extern node *EMALnum (node *arg_node, info *arg_info);

extern node *EMALap (node *arg_node, info *arg_info);
extern node *EMALarray (node *arg_node, info *arg_info);
extern node *EMALassign (node *arg_node, info *arg_info);
extern node *EMALcode (node *arg_node, info *arg_info);
extern node *EMALfold (node *arg_node, info *arg_info);
extern node *EMALfuncond (node *arg_node, info *arg_info);
extern node *EMALfundef (node *arg_node, info *arg_info);
extern node *EMALgenarray (node *arg_node, info *arg_info);
extern node *EMALid (node *arg_node, info *arg_info);
extern node *EMALlet (node *arg_node, info *arg_info);
extern node *EMALmodarray (node *arg_node, info *arg_info);
extern node *EMALprf (node *arg_node, info *arg_info);
extern node *EMALwith (node *arg_node, info *arg_info);
extern node *EMALwith2 (node *arg_node, info *arg_info);
extern node *EMALwithid (node *arg_node, info *arg_info);

#endif /* _SAC_ALLOC_H_ */
