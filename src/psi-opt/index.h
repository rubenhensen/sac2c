/*
 *
 * $Log$
 * Revision 3.9  2005/07/15 15:24:13  ktr
 * Adapted to new types. However, IVE does not work yet.
 *
 * Revision 3.8  2004/11/26 19:54:47  skt
 * IDX -> IVE
 *
 * Revision 3.7  2004/11/26 16:39:02  sbs
 * done
 *
 * Revision 3.6  2004/11/22 16:34:41  sbs
 * SacDevCamp04
 *
 * Revision 3.5  2004/10/14 14:13:56  sbs
 * provided IdxChangeIdOld in addition to IdxChangeId to ensure
 * compatibility in ReuseArray.c if needed.
 * Eventually, this function should be deleted.
 *
 * Revision 3.4  2004/10/14 13:40:11  sbs
 * changed types component within Vinfo into shape and copied these explicihan sharing a
 * pointer.
 *
 * Revision 3.3  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.2  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.1  2000/11/20 18:01:47  sacbase
 * new release made
 *
 * ....[ eliminated] ....
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 */

#ifndef _SAC_INDEX_H_
#define _SAC_INDEX_H_

#include "types.h"

extern node *IVEdoIndexVectorElimination (node *syntax_tree);

extern char *IVEchangeId (char *varname, shape *shp);

extern node *IVEmodule (node *arg_node, info *arg_info);
extern node *IVEfundef (node *arg_node, info *arg_info);
extern node *IVEblock (node *arg_node, info *arg_info);
extern node *IVEvardec (node *arg_node, info *arg_info);
extern node *IVEarg (node *arg_node, info *arg_info);
extern node *IVEassign (node *arg_node, info *arg_info);
extern node *IVEreturn (node *arg_node, info *arg_info);
extern node *IVElet (node *arg_node, info *arg_info);
extern node *IVEprf (node *arg_node, info *arg_info);
extern node *IVEid (node *arg_node, info *arg_info);
extern node *IVEnum (node *arg_node, info *arg_info);
extern node *IVEarray (node *arg_node, info *arg_info);
extern node *IVEwith (node *arg_node, info *arg_info);
extern node *IVEpart (node *arg_node, info *arg_info);
extern node *IVEcode (node *arg_node, info *arg_info);
extern node *IVEcond (node *arg_node, info *arg_info);
extern node *IVEdo (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_H_ */
