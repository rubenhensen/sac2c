/*
 * $Log$
 * Revision 1.1  2001/02/13 15:16:19  nmw
 * Initial revision
 *
 *
 */

/* this module traverses the AST and transformes the code in SSA form */
#ifndef _SSAtransform_h

#define _SSAtransform_h

extern node *SSAfundef (node *arg_node, node *arg_info);
extern node *SSAblock (node *arg_node, node *arg_info);
extern node *SSAexprs (node *arg_node, node *arg_info);
extern node *SSAlet (node *arg_node, node *arg_info);
extern node *SSAarg (node *arg_node, node *arg_info);
extern node *SSAvardec (node *arg_node, node *arg_info);
extern node *SSAid (node *arg_node, node *arg_info);
extern node *SSANwithid (node *arg_node, node *arg_info);
extern node *SSAcond (node *arg_node, node *arg_info);
extern node *SSAreturn (node *arg_node, node *arg_info);
extern node *SSAdo (node *arg_node, node *arg_info);
extern node *SSAwhile (node *arg_node, node *arg_info);

extern node *SSATransform (node *arg_node);

#endif /* _SSAtransform_h */
