/*
 *
 * $Log$
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

#ifndef _sac_alloc_h
#define _sac_alloc_h

extern node *EMAllocateFill (node *syntax_tree);

extern node *EMALap (node *arg_node, info *arg_info);
extern node *EMALarray (node *arg_node, info *arg_info);
extern node *EMALassign (node *arg_node, info *arg_info);
extern node *EMALcode (node *arg_node, info *arg_info);
extern node *EMALconst (node *arg_node, info *arg_info);
extern node *EMALfuncond (node *arg_node, info *arg_info);
extern node *EMALfundef (node *fundef, info *arg_info);
extern node *EMALicm (node *arg_node, info *arg_info);
extern node *EMALid (node *arg_node, info *arg_info);
extern node *EMALlet (node *arg_node, info *arg_info);
extern node *EMALprf (node *arg_node, info *arg_info);
extern node *EMALwith (node *arg_node, info *arg_info);
extern node *EMALwith2 (node *arg_node, info *arg_info);
extern node *EMALwithop (node *arg_node, info *arg_info);
#endif /* _sac_alloc_h */
