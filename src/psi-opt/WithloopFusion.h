/*
 *
 * $Log$
 * Revision 1.2  2004/07/21 12:47:35  khf
 * switch to new INFO structure
 *
 * Revision 1.1  2004/04/08 08:15:52  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _WithloopFusion_h
#define _WithloopFusion_h

extern node *WithloopFusion (node *arg_node);

extern node *WLFSfundef (node *arg_node, info *arg_info);
extern node *WLFSassign (node *arg_node, info *arg_info);
extern node *WLFSid (node *arg_node, info *arg_info);

extern node *WLFSNwith (node *arg_node, info *arg_info);
extern node *WLFSNwithop (node *arg_node, info *arg_info);
extern node *WLFSNpart (node *arg_node, info *arg_info);
extern node *WLFSNgenerator (node *arg_node, info *arg_info);

#endif
