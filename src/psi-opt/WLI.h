/*    $Id$
 *
 * $Log$
 * Revision 2.2  1999/02/26 14:46:49  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:35  sacbase
 * new release made
 *
 * Revision 1.4  1998/05/05 11:18:59  srs
 * moved definition of TRANSF_TRUE_PERMUTATIONS to WithloopFolding.h
 *
 * Revision 1.3  1998/04/07 08:19:36  srs
 * inserted wli_phase
 *
 * Revision 1.2  1998/04/01 07:41:03  srs
 * added WLINwithop
 *
 * Revision 1.1  1998/03/22 18:21:34  srs
 * Initial revision
 *
 */

#ifndef _WLI_h
#define _WLI_h

extern node *WLIfundef (node *, node *);
extern node *WLIid (node *, node *);
extern node *WLIassign (node *, node *);
extern node *WLIcond (node *, node *);
extern node *WLIdo (node *, node *);
extern node *WLIwhile (node *, node *);
extern node *WLIwith (node *, node *);
extern node *WLINwith (node *, node *);
extern node *WLIlet (node *, node *);

extern node *WLINwithop (node *, node *);
extern node *WLINpart (node *, node *);
extern node *WLINcode (node *, node *);

extern int wli_phase;

#endif
