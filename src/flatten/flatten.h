/*
 *
 * $Log$
 * Revision 1.3  1994/11/14 17:50:53  hw
 * added FltnCond FltnFor
 *
 * Revision 1.2  1994/11/10  15:39:42  sbs
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
extern node *FltnCond (node *aarg_node, node *arg_info);
extern node *FltnFor (node *aarg_node, node *arg_info);

#endif /* _flatten_h  */
