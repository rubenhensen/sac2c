/*
 *
 * $Log$
 * Revision 1.3  1998/05/06 21:39:40  dkr
 * added WLTRAAssign
 *
 * Revision 1.2  1998/05/06 18:09:04  dkr
 * removed WLTRAAssign
 *
 * Revision 1.1  1998/04/29 17:17:17  dkr
 * Initial revision
 *
 *
 */

#ifndef _sac_wltransform_h

#define _sac_wltransform_h

extern node *WlTransform (node *syntax_tree);

extern node *WLTRANwith (node *arg_node, node *arg_info);
extern node *WLTRANcode (node *arg_node, node *arg_info);
extern node *WLTRAFundef (node *arg_node, node *arg_info);
extern node *WLTRAAssign (node *arg_node, node *arg_info);

extern int GridOffset (int new_bound1, int bound1, int step, int grid_b2);
extern node *InsertWLnodes (node *nodes, node *insert_nodes);

#endif /* _sac_wltransform_h */
