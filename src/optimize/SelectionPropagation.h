/*
 *
 * $Log$
 * Revision 1.3  2003/09/16 18:15:26  ktr
 * Index vectors are now treated as structural constants.
 *
 *
 */

#ifndef _SAC_SELECTIONPROPAGATION_H

#define _SAC_SELECTIONPROPAGATION_H

extern node *SelectionPropagation (node *fundef, node *modul);

extern node *SPfundef (node *arg_node, node *arg_info);
extern node *SPreturn (node *arg_node, node *arg_info);
extern node *SPid (node *arg_node, node *arg_info);
extern node *SPwith (node *arg_node, node *arg_info);
extern node *SPwithid (node *arg_node, node *arg_info);
extern node *SPpart (node *arg_node, node *arg_info);
extern node *SPcode (node *arg_node, node *arg_info);
extern node *SParg (node *arg_node, node *arg_info);
extern node *SPap (node *arg_node, node *arg_info);
extern node *SPprf (node *arg_node, node *arg_info);
extern node *SPlet (node *arg_node, node *arg_info);
extern node *SPassign (node *arg_node, node *arg_info);

#endif /* SAC_SELECTIONPROPAGATION_H */
