/*
 * $Log$
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
extern node *USSAfundef (node *arg_node, node *arg_info);
extern node *USSAblock (node *arg_node, node *arg_info);
extern node *USSANwithid (node *arg_node, node *arg_info);
#endif /* SAC_UNDOSSATRANSFORM_H */
