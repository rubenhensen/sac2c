/*
 *
 * $Log$
 * Revision 1.6  1994/11/22 16:39:55  hw
 * added FltnDo
 *
 * Revision 1.5  1994/11/17  16:50:16  hw
 * added FltnWhile
 *
 * Revision 1.4  1994/11/15  14:45:31  hw
 * deleted FltnFor
 *
 * Revision 1.3  1994/11/14  17:50:53  hw
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
extern node *FltnCond (node *arg_node, node *arg_info);
extern node *FltnWhile (node *arg_node, node *arg_info);
extern node *FltnWith (node *arg_node, node *arg_info);
extern node *FltnDo (node *arg_node, node *arg_info);

#endif /* _flatten_h  */
