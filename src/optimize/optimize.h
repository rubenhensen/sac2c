/*
 *
 * $Log$
 * Revision 1.6  1995/01/02 16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.5  1995/01/02  10:50:49  asi
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

extern node *OPT1fundef (node *, node *);
extern node *OPT1arg (node *, node *);
extern node *OPT1vardec (node *, node *);
extern node *OPT1modul (node *, node *);
extern node *OPT1let (node *, node *);
extern node *OPT1id (node *, node *);
extern node *OPT1assign (node *, node *);
extern node *OPT1cond (node *, node *);
extern node *OPT1do (node *, node *);
extern node *OPT1while (node *, node *);
extern node *OPT1pp (node *, node *);

extern node *Optimize (node *);

#endif /* _optimize_h */
