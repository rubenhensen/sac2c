/*
 * $Log$
 * Revision 3.1  2000/11/20 18:01:33  sacbase
 * new release made
 *
 * Revision 2.3  2000/06/25 01:54:26  dkr
 * WLTRAfundef removed
 *
 * Revision 2.2  2000/06/23 15:30:58  dkr
 * minor changes done
 *
 * Revision 2.1  1999/02/23 12:43:02  sacbase
 * new release made
 *
 * Revision 1.6  1998/05/15 15:11:31  dkr
 * added WLTRALet
 *
 * Revision 1.5  1998/05/06 22:19:27  dkr
 * removed WLTRALet
 *
 * Revision 1.4  1998/05/06 21:42:50  dkr
 * added WLTRALet
 *
 * Revision 1.2  1998/05/06 18:09:04  dkr
 * removed WLTRAAssign
 *
 * Revision 1.1  1998/04/29 17:17:17  dkr
 * Initial revision
 */

#ifndef _sac_wltransform_h

#define _sac_wltransform_h

extern node *WlTransform (node *syntax_tree);

extern node *WLTRAwith (node *arg_node, node *arg_info);
extern node *WLTRAcode (node *arg_node, node *arg_info);
extern node *WLTRAlet (node *arg_node, node *arg_info);

extern int GridOffset (int new_bound1, int bound1, int step, int grid_b2);
extern node *InsertWLnodes (node *nodes, node *insert_nodes);

#endif /* _sac_wltransform_h */
