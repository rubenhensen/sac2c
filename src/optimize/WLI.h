/*    $Id$
 *
 * $Log$
 * Revision 1.1  1998/03/22 18:21:34  srs
 * Initial revision
 *
 */

#ifndef _WLI_h
#define _WLI_h

/* if not defined, indexes with more than one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
#define TRANSF_TRUE_PERMUTATIONS

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

#endif
