/*
 *
 * $Log$
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
 *
 */

#ifndef sac_index_h

#define sac_index_h

extern char *IdxChangeId (char *varname, types *type);

extern node *IdxModul (node *arg_node, node *arg_info);
extern node *IdxFundef (node *arg_node, node *arg_info);
extern node *IdxArg (node *arg_node, node *arg_info);
extern node *IdxAssign (node *arg_node, node *arg_info);
extern node *IdxWith (node *arg_node, node *arg_info);
extern node *IdxGenerator (node *arg_node, node *arg_info);
extern node *IdxLet (node *arg_node, node *arg_info);
extern node *IdxPrf (node *arg_node, node *arg_info);
extern node *IdxId (node *arg_node, node *arg_info);
extern node *IdxNum (node *arg_node, node *arg_info);
extern node *IdxArray (node *arg_node, node *arg_info);
extern node *IdxNwith (node *arg_node, node *arg_info);
extern node *IdxNcode (node *arg_node, node *arg_info);

#endif /* sac_index_h */
