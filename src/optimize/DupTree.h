/*
 *
 * $Log$
 * Revision 1.1  1995/05/01 15:32:32  asi
 * Initial revision
 *
 */

#ifndef _DupTree_h

#define _DupTree_h

extern node *DupTree (node *arg_node, node *arg_info);

extern node *DupNum (node *arg_node, node *arg_info);
extern node *DupBool (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
extern node *DupArray (node *arg_node, node *arg_info);
extern node *DupExprs (node *arg_node, node *arg_info);

#endif /* _DupTree_h */
