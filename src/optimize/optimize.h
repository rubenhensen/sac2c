/*
 *
 * $Log$
 * Revision 1.5  1995/01/02 10:50:49  asi
 * added OPTassign
 *
 * Revision 1.4  1994/12/19  15:26:07  asi
 * Added - OPTid
 *
 * Revision 1.3  1994/12/19  14:43:17  asi
 * Added - OPTfundef OPTarg OPTvardec OPTmodul OPTlet OPTblock
 * preperation for loop invariant removal
 *
 * Revision 1.2  1994/12/12  11:04:18  asi
 * added OPTdo
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _optimize_h

#define _optimize_h

extern node *OPTfundef (node *, node *);
extern node *OPTarg (node *, node *);
extern node *OPTvardec (node *, node *);
extern node *OPTmodul (node *, node *);
extern node *OPTlet (node *, node *);
extern node *OPTblock (node *, node *);
extern node *OPTid (node *, node *);
extern node *OPTassign (node *, node *);

extern node *Optimize (node *);

#endif /* _optimize_h */
