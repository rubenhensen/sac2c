/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:29  sacbase
 * new release made
 *
 * Revision 1.2  1998/02/05 17:17:39  srs
 * extern UNQNwith
 *
 * Revision 1.1  1995/11/06 08:14:10  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_uniquecheck_h
#define _sac_uniquecheck_h

extern node *UniquenessCheck (node *syntax_tree);

extern node *UNQmodul (node *arg_node, node *arg_info);
extern node *UNQfundef (node *arg_node, node *arg_info);
extern node *UNQblock (node *arg_node, node *arg_info);
extern node *UNQvardec (node *arg_node, node *arg_info);
extern node *UNQarg (node *arg_node, node *arg_info);
extern node *UNQlet (node *arg_node, node *arg_info);
extern node *UNQid (node *arg_node, node *arg_info);
extern node *UNQdo (node *arg_node, node *arg_info);
extern node *UNQwhile (node *arg_node, node *arg_info);
extern node *UNQcond (node *arg_node, node *arg_info);
extern node *UNQwith (node *arg_node, node *arg_info);
extern node *UNQNwith (node *arg_node, node *arg_info);

#endif /*  _sac_uniquecheck_h  */
