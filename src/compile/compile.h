/*
 *
 * $Log$
 * Revision 1.6  1995/05/08 15:45:13  hw
 * added CompBlock
 *
 * Revision 1.5  1995/04/24  18:07:54  hw
 * - renamed CompWhile to CompLoop
 *
 * Revision 1.4  1995/04/24  14:18:20  hw
 * CompWhile & CompCond inserted
 *
 * Revision 1.3  1995/04/03  16:34:48  hw
 * CompFundef inserted
 *
 * Revision 1.2  1995/03/31  15:47:37  hw
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
extern node *CompFundef (node *arg_node, node *arg_info);
extern node *CompLoop (node *arg_node, node *arg_info);
extern node *CompCond (node *arg_node, node *arg_info);
extern node *CompBlock (node *arg_node, node *arg_info);

#endif /* _compile_h */
