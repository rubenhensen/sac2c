/*
 * $Log$
 * Revision 1.1  2001/03/09 11:08:08  sbs
 * Initial revision
 *
 *
 */

#ifndef _annotate_fun_calls_h
#define _annotate_fun_calls_h

extern node *ProfileFunCalls (node *fundef);

extern node *PFfundef (node *arg_node, node *arg_info);
extern node *PFassign (node *arg_node, node *arg_info);
extern node *PFap (node *arg_node, node *arg_info);

#endif /* _annotate_fun_calls_h */
