/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:22  sacbase
 * new release made
 *
 * Revision 1.2  1995/11/01 16:35:12  cg
 * added external declarations of functions OBJap, OBJid and OBJlet.
 *
 * Revision 1.1  1995/10/31  17:22:05  cg
 * Initial revision
 *
 * Revision 1.1  1995/10/31  17:22:05  cg
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
extern node *OBJap (node *arg_node, node *arg_info);
extern node *OBJid (node *arg_node, node *arg_info);
extern node *OBJlet (node *arg_node, node *arg_info);

#endif /* _sac_objects_h */
