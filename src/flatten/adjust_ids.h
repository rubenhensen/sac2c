/*
 *
 * $Log$
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   adjust_ids.h
 *
 * prefix: AI
 *
 * description:
 *
 *   header file for adjust_ids.c.
 *
 *
 *****************************************************************************/

#ifndef ADJUST_IDS_H
#define ADJUST_IDS_H

extern node *AdjustIdentifiers (node *fundef, node *let);

extern node *AIfundef (node *arg_node, node *arg_info);
extern node *AIblock (node *arg_node, node *arg_info);
extern node *AIvardec (node *arg_node, node *arg_info);
extern node *AIarg (node *arg_node, node *arg_info);
extern node *AIassign (node *arg_node, node *arg_info);
extern node *AIlet (node *arg_node, node *arg_info);
extern node *AIap (node *arg_node, node *arg_info);
extern node *AIreturn (node *arg_node, node *arg_info);
extern node *AIexprs (node *arg_node, node *arg_info);
extern node *AIid (node *arg_node, node *arg_info);
extern node *AIids (node *arg_node, node *arg_info);

#endif /* ADJUST_IDS_H */
