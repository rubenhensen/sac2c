/*
 *
 * $Log$
 * Revision 1.1  1995/07/24 10:00:19  asi
 * Initial revision
 *
 *
 */

#ifndef _ArrayElimination_h

#define _ArrayElimination_h

extern node *ArrayElimination (node *arg_node, node *arg_info);

extern node *AEfundef (node *arg_node, node *arg_info);
extern node *AEassign (node *arg_node, node *arg_info);
extern node *AEprf (node *arg_node, node *arg_info);

#endif /* _ArrayElimination_h */
