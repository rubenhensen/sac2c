/*
 *
 * $Log$
 * Revision 3.5  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 3.4  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
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

#ifndef _SAC_ARRAYELIMINATION_H_
#define _SAC_ARRAYELIMINATION_H_

include "types.h"

  extern node *
  AEdoArrayElimination (node *arg_node);

extern node *AEprf (node *arg_node, info *arg_info);
extern node *AEfundef (node *arg_node, info *arg_info);
extern node *AEassign (node *arg_node, info *arg_info);
extern node *AEcond (node *arg_node, info *arg_info);
extern node *AEdo (node *arg_node, info *arg_info);
extern node *AEwith (node *arg_node, info *arg_info);
extern node *AEap (node *arg_node, info *arg_info);

#endif /* _SAC_ARRAYELIMINATION_H_ */
