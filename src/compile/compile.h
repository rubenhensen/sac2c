/*
 *
 * $Log$
 * Revision 1.2  1995/03/31 15:47:37  hw
 * CompId, CompArg, CompWith, CompArray, CompAp, CompReturn inserted
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 *
 *
 */

#ifndef _compile_h

#define _compile_h

extern node *Compile (node *arg_node);
extern node *CompVardec (node *arg_node, node *arg_info);
extern node *CompPrf (node *arg_node, node *arg_info);
extern node *CompAssign (node *arg_node, node *arg_info);
extern node *CompLet (node *arg_node, node *arg_info);
extern node *CompArray (node *arg_node, node *arg_info);
extern node *CompId (node *arg_node, node *arg_info);
extern node *CompAp (node *arg_node, node *arg_info);
extern node *CompReturn (node *arg_node, node *arg_info);
extern node *CompWith (node *arg_node, node *arg_info);
extern node *CompArg (node *arg_node, node *arg_info);

#endif /* _compile_h */
