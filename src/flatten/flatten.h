/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:39:42  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _flatten_h

#define _flatten_h

extern node *Flatten (node *);
extern node *FltnAssign (node *arg_node, node *arg_info);
extern node *FltnPrf (node *arg_node, node *arg_info);
extern node *FltnExprs (node *arg_node, node *arg_info);

#endif /* _flatten_h  */
