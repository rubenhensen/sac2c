/*
 *
 * $Log$
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

extern node *EMALap (node *arg_node, info *arg_info);
extern node *EMALarray (node *arg_node, info *arg_info);
extern node *EMALassign (node *arg_node, info *arg_info);
extern node *EMALcode (node *arg_node, info *arg_info);
extern node *EMALconst (node *arg_node, info *arg_info);
extern node *EMALfold (node *arg_node, info *arg_info);
extern node *EMALfuncond (node *arg_node, info *arg_info);
extern node *EMALfundef (node *arg_node, info *arg_info);
extern node *EMALgenarray (node *arg_node, info *arg_info);
extern node *EMALicm (node *arg_node, info *arg_info);
extern node *EMALid (node *arg_node, info *arg_info);
extern node *EMALlet (node *arg_node, info *arg_info);
extern node *EMALmodarray (node *arg_node, info *arg_info);
extern node *EMALprf (node *arg_node, info *arg_info);
extern node *EMALwith (node *arg_node, info *arg_info);
extern node *EMALwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_ALLOC_H_ */
