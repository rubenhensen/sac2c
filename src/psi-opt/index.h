/*
 *
 * $Log$
 * Revision 1.3  1995/06/06 15:18:10  sbs
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
extern node *IdxAssign (node *, node *);
extern node *IdxLet (node *, node *);
extern node *IdxPrf (node *, node *);
extern node *IdxId (node *, node *);

#endif /* sac_index_h */
