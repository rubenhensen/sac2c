/*
 * $Log$
 * Revision 1.1  2000/12/15 18:31:16  dkr
 * Initial revision
 *
 * Revision 3.1  2000/11/20 17:59:17  sacbase
 * new release made
 *
 * Revision 1.3  2000/06/05 12:34:36  dkr
 * Prototypes for functions AIwith, AIwith2, AIcode added
 *
 * Revision 1.2  2000/05/29 14:29:11  dkr
 * prototype of AIwithid added
 *
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
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
extern node *AIwith (node *arg_node, node *arg_info);
extern node *AIwith2 (node *arg_node, node *arg_info);
extern node *AIwithid (node *arg_node, node *arg_info);
extern node *AIcode (node *arg_node, node *arg_info);

#endif /* ADJUST_IDS_H */
