/*
 * $Log$
 * Revision 1.6  1994/12/20 14:20:44  hw
 * added function TCunaryOp to handle typechecking of N_post & N_pre
 *
 * Revision 1.5  1994/12/19  13:01:42  hw
 * inserted TCdo & TCwhile
 *
 * Revision 1.4  1994/12/14  08:47:42  hw
 * added declarations of TCcond & TCassign
 *
 * Revision 1.3  1994/12/06  09:58:19  hw
 * changed log-header
 *
 *
 */

#ifndef _typecheck_h

#define _typecheck_h

extern void Typecheck (node *arg_node);

extern node *TCfundef (node *arg_node, node *arg_info);

extern node *TClet (node *arg_node, node *arg_info);
extern node *TCreturn (node *arg_node, node *arg_info);
extern node *TCcond (node *arg_node, node *arg_info);
extern node *TCassign (node *arg_node, node *arg_info);
extern node *TCdo (node *arg_node, node *arg_info);
extern node *TCwhile (node *arg_node, node *arg_info);
extern node *TCunaryOp (node *arg_node, node *arg_info);

#endif /* _typecheck_h */
