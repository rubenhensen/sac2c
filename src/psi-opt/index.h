/*
 *
 * $Log$
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
 * Revision 2.5  2000/07/11 15:50:01  dkr
 * function IndexVectorElimination added
 *
 * Revision 2.4  2000/05/29 17:29:58  dkr
 * functions for old with-loop removed
 *
 * Revision 2.3  1999/07/14 12:11:16  sbs
 * Major re-implementation of IVE!
 * - stacking of ACTCHNs for proper handling of CONDs and LOOPs
 * - iv += 3 for non-maximal iv's are handled correctly now
 * - proper arg_info usage added
 * - ivs of WLs can be eliminated now
 * - some code streamlining, e.g. proper usage of MakeIcm...
 *
 * Revision 2.2  1999/07/07 06:02:56  sbs
 * changed vardec chains into $-stacked chaines.
 * Appropriate handling of cond/ do / while is not yet included.
 *
 * Revision 2.1  1999/02/23 12:43:14  sacbase
 * new release made
 *
 * Revision 1.8  1998/06/07 18:38:00  dkr
 * ChgId renamed in IdxChangeId and exported (for ReuseWithArrays.[ch])
 *
 * Revision 1.7  1998/05/07 16:18:59  dkr
 * added IdxNwith, IdxNcode
 *
 * Revision 1.6  1996/02/11 20:17:07  sbs
 * IdxArgs inserted.
 *
 * Revision 1.5  1995/10/05  14:55:52  sbs
 * some bug fixes.
 *
 * Revision 1.4  1995/06/26  09:59:28  sbs
 * preliminary version
 *
 * Revision 1.3  1995/06/06  15:18:10  sbs
 * first usable version ; does not include conditional stuff
 *
 * Revision 1.2  1995/06/02  17:05:15  sbs
 * Idxfuns external declared
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 */

#ifndef sac_index_h
#define sac_index_h

extern node *IndexVectorElimination (node *syntax_tree);

extern char *IdxChangeId (char *varname, shape *shp);

extern node *IdxModul (node *arg_node, info *arg_info);
extern node *IdxFundef (node *arg_node, info *arg_info);
extern node *IdxBlock (node *arg_node, info *arg_info);
extern node *IdxVardec (node *arg_node, info *arg_info);
extern node *IdxArg (node *arg_node, info *arg_info);
extern node *IdxAssign (node *arg_node, info *arg_info);
extern node *IdxReturn (node *arg_node, info *arg_info);
extern node *IdxLet (node *arg_node, info *arg_info);
extern node *IdxPrf (node *arg_node, info *arg_info);
extern node *IdxId (node *arg_node, info *arg_info);
extern node *IdxNum (node *arg_node, info *arg_info);
extern node *IdxArray (node *arg_node, info *arg_info);
extern node *IdxNwith (node *arg_node, info *arg_info);
extern node *IdxNpart (node *arg_node, info *arg_info);
extern node *IdxNcode (node *arg_node, info *arg_info);
extern node *IdxCond (node *arg_node, info *arg_info);
extern node *IdxDo (node *arg_node, info *arg_info);

#endif /* sac_index_h */
