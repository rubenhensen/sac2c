/*
 *
 * $Log$
 * Revision 1.1  1995/10/31 17:22:05  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_objects_h
#define _sac_objects_h

extern node *HandleObjects (node *syntax_tree);

extern node *OBJmodul (node *arg_node, node *arg_info);
extern node *OBJfundef (node *arg_node, node *arg_info);
extern node *OBJobjdef (node *arg_node, node *arg_info);
extern node *OBJarg (node *arg_node, node *arg_info);

#endif /* _sac_objects_h */
