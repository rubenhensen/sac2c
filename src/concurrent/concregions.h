/*
 *
 * $Log$
 * Revision 1.2  1998/04/09 14:02:22  dkr
 * new funs ConcFundef, ConcNcode, ConcNpart, ...
 *
 * Revision 1.1  1998/04/03 11:37:24  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_concregions_h

#define _sac_concregions_h

extern node *ConcFundef (node *arg_node, node *arg_info);
extern node *ConcAssign (node *arg_node, node *arg_info);
extern node *ConcNpart (node *arg_node, node *arg_info);
extern node *ConcNcode (node *arg_node, node *arg_info);

extern node *ConcRegions (node *syntax_tree);

#endif /* _sac_concregions_h */
