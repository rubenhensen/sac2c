/*         $Id$
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:40  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/15 14:43:29  srs
 * functions for WL unrolling
 *
 * Revision 1.1  1998/05/13 13:47:57  srs
 * Initial revision
 *
 *
 */
#ifndef _WLUnroll_h
#define _WLUnroll_h

extern int CheckUnrollModarray (node *wln);
extern node *DoUnrollModarray (node *wln, node *arg_info);

extern int CheckUnrollGenarray (node *wln, node *arg_info);
extern node *DoUnrollGenarray (node *wln, node *arg_info);

extern int CheckUnrollFold (node *wln);
extern node *DoUnrollFold (node *wln, node *arg_info);

#endif
