/*
 * $Log$
 * Revision 3.1  2000/11/20 18:01:43  sacbase
 * new release made
 *
 * Revision 2.3  2000/05/30 12:35:11  dkr
 * functions for old with-loop removed
 *
 * Revision 2.2  1999/02/26 14:49:00  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:38  sacbase
 * new release made
 *
 * Revision 1.1  1998/03/22 18:21:38  srs
 * Initial revision
 */

#ifndef _WLT_h
#define _WLT_h

extern node *WLTfundef (node *, node *);
extern node *WLTassign (node *, node *);
extern node *WLTcond (node *, node *);
extern node *WLTdo (node *, node *);
extern node *WLTwhile (node *, node *);
extern node *WLTNwith (node *, node *);
extern node *WLTlet (node *, node *);

extern node *WLTNpart (node *, node *);
extern node *WLTNgenerator (node *, node *);
extern node *WLTNcode (node *, node *);

#endif
