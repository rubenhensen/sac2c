/*
 *
 * $Log$
 * Revision 1.1  2004/07/14 15:27:00  ktr
 * Initial revision
 *
 *
 */

#ifndef _sac_alloc_h
#define _sac_alloc_h

extern node *EMALAllocateFill (node *syntax_tree);

extern node *EMALap (node *arg_node, node *arg_info);
extern node *EMALarray (node *arg_node, node *arg_info);
extern node *EMALassign (node *arg_node, node *arg_info);
extern node *EMALcode (node *arg_node, node *arg_info);
extern node *EMALconst (node *arg_node, node *arg_info);
extern node *EMALfundef (node *fundef, node *arg_info);
extern node *EMALicm (node *arg_node, node *arg_info);
extern node *EMALlet (node *arg_node, node *arg_info);
extern node *EMALprf (node *arg_node, node *arg_info);
extern node *EMALwith (node *arg_node, node *arg_info);
#endif /* _sac_alloc_h */
