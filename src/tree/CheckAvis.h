/*
 * $Log$
 * Revision 1.2  2001/02/13 15:16:34  nmw
 * CheckAvis traversal implemented
 *
 * Revision 1.1  2001/02/12 16:58:52  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   CheckAvis.h
 *
 * prefix: CAV
 *
 * description:
 *
 *   This module restores the AVIS attribute in N_id, N_vardec/N_arg
 *   when old code did not updates all references correctly.
 *
 *
 *****************************************************************************/

#ifndef SAC_CHECKAVIS_H

#define SAC_CHECKAVIS_H

extern node *CheckAvis (node *syntax_tree);

extern node *CAVarg (node *arg_node, node *arg_info);
extern node *CAVvardec (node *arg_node, node *arg_info);
extern node *CAVid (node *arg_node, node *arg_info);
extern node *CAVlet (node *arg_node, node *arg_info);
extern node *CAVfundef (node *arg_node, node *arg_info);
extern node *CAVblock (node *arg_node, node *arg_info);
extern node *CAVNwithid (node *arg_node, node *arg_info);
#endif /* SAC_CHECKAVIS_H */
