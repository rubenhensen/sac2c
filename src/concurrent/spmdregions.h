/*
 *
 * $Log$
 * Revision 1.2  1998/04/17 17:50:59  dkr
 * modified header
 *
 * Revision 1.1  1998/04/17 17:22:04  dkr
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
