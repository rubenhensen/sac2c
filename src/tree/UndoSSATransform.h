/*
 * $Log$
 * Revision 1.3  2001/03/19 14:23:32  nmw
 * removal of ssa phi copy assignments added
 *
 * Revision 1.2  2001/03/12 13:41:53  nmw
 * UndoSSA creates unique result variables in multigenerator fold-withloops.
 *
 * Revision 1.1  2001/02/22 13:14:06  nmw
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   UndoSSATransform.h
 *
 * prefix: USSA
 *
 *
 *****************************************************************************/

#ifndef SAC_UNDOSSATRANSFORM_H

#define SAC_UNDOSSATRANSFORM_H

extern node *UndoSSATransform (node *syntax_tree);

extern node *USSAarg (node *arg_node, node *arg_info);
extern node *USSAvardec (node *arg_node, node *arg_info);
extern node *USSAid (node *arg_node, node *arg_info);
extern node *USSAlet (node *arg_node, node *arg_info);
extern node *USSAassign (node *arg_node, node *arg_info);
extern node *USSAfundef (node *arg_node, node *arg_info);
extern node *USSAblock (node *arg_node, node *arg_info);
extern node *USSANwithid (node *arg_node, node *arg_info);
extern node *USSANcode (node *arg_node, node *arg_info);
extern node *USSANwith (node *arg_node, node *arg_info);
#endif /* SAC_UNDOSSATRANSFORM_H */
