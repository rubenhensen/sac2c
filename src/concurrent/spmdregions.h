/*
 *
 * $Log$
 * Revision 1.1  1998/04/17 17:22:04  dkr
 * Initial revision
 *
 * Revision 1.2  1998/04/09 14:02:22  dkr
 * new funs ConcFundef, ConcNcode, ConcNpart, ...
 *
 * Revision 1.1  1998/04/03 11:37:24  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_spmdregions_h

#define _sac_spmdregions_h

extern node *SpmdFundef (node *arg_node, node *arg_info);
extern node *SpmdAssign (node *arg_node, node *arg_info);
extern node *SpmdNpart (node *arg_node, node *arg_info);
extern node *SpmdNcode (node *arg_node, node *arg_info);

extern node *SpmdRegions (node *syntax_tree);

#endif /* _sac_spmdregions_h */
