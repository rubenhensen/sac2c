/*         $Id$
 *
 * $Log$
 * Revision 1.1  1998/05/13 13:47:57  srs
 * Initial revision
 *
 *
 */
#ifndef _WLUnroll_h
#define _WLUnroll_h

extern int CheckUnrollModarray (node *wln);
extern node *DoUnrollModarray (node *wln);

extern int CheckUnrollGenarray (node *wln);
extern node *DoUnrollGenarray (node *wln);

extern int CheckUnrollFold (node *wln);
extern node *DoUnrollFold (node *wln);

#endif
