/*
 * $Log : $
 *
 */

#ifndef _typecheck_h

#define _typecheck_h

extern void Typecheck (node *arg_node);

extern node *TCfundef (node *arg_node, node *arg_info);

extern node *TClet (node *arg_node, node *arg_info);
extern node *TCreturn (node *arg_node, node *arg_info);

#endif /* _typecheck_h */
