/*
 * $Log$
 * Revision 1.3  1994/12/06 09:58:19  hw
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

#endif /* _typecheck_h */
