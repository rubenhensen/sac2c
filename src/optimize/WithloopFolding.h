/*    $Id$
 *
 * $Log$
 * Revision 1.8  1998/03/18 08:33:15  srs
 * first running version of WLI
 *
 * Revision 1.7  1998/03/06 13:29:05  srs
 * added new WLI functions
 *
 * Revision 1.6  1998/02/24 14:19:26  srs
 * *** empty log message ***
 *
 * Revision 1.5  1998/02/09 15:58:47  srs
 * *** empty log message ***
 *
 * Revision 1.4  1998/02/06 14:33:29  srs
 * RCS-Test
 *
 * Revision 1.3  1998/02/06 14:32:49  srs
 * *** empty log message ***
 *
 */

#ifndef _WithloopFolding_h
#define _WithloopFolding_h

/* if not defined, indexes with more then one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
#define TRANSF_TRUE_PERMUTATIONS

extern node *WLFWithloopFolding (node *, node *);

extern node *WLIfundef (node *, node *);
extern node *WLIid (node *, node *);
extern node *WLIassign (node *, node *);
extern node *WLIcond (node *, node *);
extern node *WLIdo (node *, node *);
extern node *WLIwhile (node *, node *);
extern node *WLIwith (node *, node *);
extern node *WLINwith (node *, node *);
extern node *WLIlet (node *, node *);

extern node *WLINpart (node *, node *);
extern node *WLINcode (node *, node *);

extern node *WLFNwith (node *, node *);

#endif
