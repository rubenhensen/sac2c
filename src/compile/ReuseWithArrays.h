/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:01:03  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:25  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/07 18:43:12  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_ReuseWithArrays_h

#define _sac_ReuseWithArrays_h

extern node *GetReuseArrays (node *syntax_tree, node *fundef, ids *wl_ids);

extern node *ReuseFundef (node *arg_node, node *arg_info);
extern node *ReuseNwith2 (node *arg_node, node *arg_info);
extern node *ReuseNwithop (node *arg_node, node *arg_info);
extern node *ReuseLet (node *arg_node, node *arg_info);
extern node *ReuseId (node *arg_node, node *arg_info);

#endif /* _sac_ReuseWithArrays_h */
