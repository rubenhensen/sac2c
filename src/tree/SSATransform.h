/*
 * $Log$
 * Revision 1.3  2001/03/23 09:31:19  nmw
 * SSAwhile/do removed SSADummy added
 *
 * Revision 1.2  2001/02/14 14:40:36  nmw
 * function bodies and traversal order implemented
 *
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
extern node *SSAassign (node *arg_node, node *arg_info);
extern node *SSAlet (node *arg_node, node *arg_info);
extern node *SSAarg (node *arg_node, node *arg_info);
extern node *SSAvardec (node *arg_node, node *arg_info);
extern node *SSAid (node *arg_node, node *arg_info);
extern node *SSANwith (node *arg_node, node *arg_info);
extern node *SSANcode (node *arg_node, node *arg_info);
extern node *SSANpart (node *arg_node, node *arg_info);
extern node *SSANwithid (node *arg_node, node *arg_info);
extern node *SSAcond (node *arg_node, node *arg_info);
extern node *SSAreturn (node *arg_node, node *arg_info);

extern node *SSADummy (node *arg_node, node *arg_info);

extern node *SSATransform (node *arg_node);

#endif /* _SSAtransform_h */
