/*
 *
 * $Log$
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

extern node *IDXdoIndexVectorElimination (node *syntax_tree);

extern char *IDXchangeId (char *varname, shape *shp);
extern char *IDXchangeIdOld (char *varname, types *shp);

extern node *IDXmodule (node *arg_node, info *arg_info);
extern node *IDXfundef (node *arg_node, info *arg_info);
extern node *IDXblock (node *arg_node, info *arg_info);
extern node *IDXvardec (node *arg_node, info *arg_info);
extern node *IDXarg (node *arg_node, info *arg_info);
extern node *IDXassign (node *arg_node, info *arg_info);
extern node *IDXreturn (node *arg_node, info *arg_info);
extern node *IDXlet (node *arg_node, info *arg_info);
extern node *IDXprf (node *arg_node, info *arg_info);
extern node *IDXid (node *arg_node, info *arg_info);
extern node *IDXnum (node *arg_node, info *arg_info);
extern node *IDXarray (node *arg_node, info *arg_info);
extern node *IDXwith (node *arg_node, info *arg_info);
extern node *IDXpart (node *arg_node, info *arg_info);
extern node *IDXcode (node *arg_node, info *arg_info);
extern node *IDXcond (node *arg_node, info *arg_info);
extern node *IDXdo (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_H_ */
