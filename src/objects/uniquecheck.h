/*
 * $Log$
 * Revision 3.4  2004/11/22 17:53:27  ktr
 * SacDevCamp 2004 Get Ready for Rumble!
 *
 * Revision 3.3  2004/07/17 14:30:09  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.2  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.1  2000/11/20 18:02:03  sacbase
 * new release made
 *
 * Revision 2.2  2000/05/30 12:34:23  dkr
 * functions for old with-loop removed
 *
 * Revision 2.1  1999/02/23 12:43:29  sacbase
 * new release made
 *
 * Revision 1.2  1998/02/05 17:17:39  srs
 * extern UNQNwith
 *
 * Revision 1.1  1995/11/06 08:14:10  cg
 * Initial revision
 */

#ifndef _SAC_UNIQUECHECK_H_
#define _SAC_UNIQUECHECK_H_

#include "types.h"

/******************************************************************************
 *
 * Unique check traversal ( unq_tab)
 *
 * Prefix: UNQ
 *
 *****************************************************************************/
extern node *UNQdoUniquenessCheck (node *syntax_tree);

extern node *UNQmodule (node *arg_node, info *arg_info);
extern node *UNQfundef (node *arg_node, info *arg_info);
extern node *UNQblock (node *arg_node, info *arg_info);
extern node *UNQvardec (node *arg_node, info *arg_info);
extern node *UNQarg (node *arg_node, info *arg_info);
extern node *UNQlet (node *arg_node, info *arg_info);
extern node *UNQid (node *arg_node, info *arg_info);
extern node *UNQdo (node *arg_node, info *arg_info);
extern node *UNQcond (node *arg_node, info *arg_info);
extern node *UNQwith (node *arg_node, info *arg_info);

#endif /* _SAC_UNIQUECHECK_H_  */
