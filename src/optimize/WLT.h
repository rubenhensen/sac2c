/*    $Id$
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:38  sacbase
 * new release made
 *
 * Revision 1.1  1998/03/22 18:21:38  srs
 * Initial revision
 *
 */

#ifndef _WLT_h
#define _WLT_h

extern node *WLTfundef (node *, node *);
extern node *WLTassign (node *, node *);
extern node *WLTcond (node *, node *);
extern node *WLTdo (node *, node *);
extern node *WLTwhile (node *, node *);
extern node *WLTwith (node *, node *);
extern node *WLTNwith (node *, node *);
extern node *WLTlet (node *, node *);

extern node *WLTNpart (node *, node *);
extern node *WLTNgenerator (node *, node *);
extern node *WLTNcode (node *, node *);

#endif
