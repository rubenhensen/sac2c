/*
 *
 * $Log$
 * Revision 1.1  2004/07/27 14:10:37  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_adjustaccumulation_h
#define _sac_adjustaccumulation_h

extern node *AdjustAccumulation (node *syntax_tree);

extern node *AACCdo (node *arg_node, info *arg_info);
extern node *AACCfundef (node *arg_node, info *arg_info);
extern node *AACCid (node *arg_node, info *arg_info);
extern node *AACClet (node *arg_node, info *arg_info);
extern node *AACCprf (node *arg_node, info *arg_info);
extern node *AACCwith (node *arg_node, info *arg_info);
extern node *AACCwith2 (node *arg_node, info *arg_info);
extern node *AACCcode (node *arg_node, info *arg_info);

#endif /* _sac_adjustaccumulation_h */
