/*
 *
 * $Log$
 * Revision 3.3  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.2  2001/04/30 12:17:50  nmw
 * AEap added
 *
 * Revision 3.1  2000/11/20 18:01:38  sacbase
 * new release made
 *
 * Revision 2.2  2000/06/13 12:32:00  dkr
 * function for old with-loop removed
 *
 * Revision 2.1  1999/02/23 12:43:09  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/12 15:10:48  srs
 * added more functions
 *
 * Revision 1.1  1995/07/24 10:00:19  asi
 * Initial revision
 *
 *
 */

#ifndef _ArrayElimination_h

#define _ArrayElimination_h

extern node *ArrayElimination (node *arg_node, node *arg_info);

extern node *AEprf (node *arg_node, node *arg_info);
extern node *AEfundef (node *arg_node, node *arg_info);
extern node *AEassign (node *arg_node, node *arg_info);
extern node *AEcond (node *arg_node, node *arg_info);
extern node *AEdo (node *arg_node, node *arg_info);
extern node *AENwith (node *arg_node, node *arg_info);
extern node *AEap (node *arg_node, node *arg_info);

#endif /* _ArrayElimination_h */
