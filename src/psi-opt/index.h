/*
 *
 * $Log$
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

extern node *IdxModul (node *, node *);
extern node *IdxFundef (node *, node *);
extern node *IdxArg (node *, node *);
extern node *IdxAssign (node *, node *);
extern node *IdxWith (node *, node *);
extern node *IdxGenerator (node *, node *);
extern node *IdxLet (node *, node *);
extern node *IdxPrf (node *, node *);
extern node *IdxId (node *, node *);
extern node *IdxNum (node *, node *);
extern node *IdxArray (node *, node *);
extern node *IdxNwith (node *arg_node, node *arg_info);
extern node *IdxNcode (node *arg_node, node *arg_info);

#endif /* sac_index_h */
