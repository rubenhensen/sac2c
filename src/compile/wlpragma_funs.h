/*
 *
 * $Log$
 * Revision 3.2  2001/01/08 11:41:28  dkr
 * wlpragma_funs.mac used for generation of function prototypes now
 *
 * Revision 3.1  2000/11/20 18:01:29  sacbase
 * new release made
 *
 * Revision 2.2  2000/03/15 13:55:26  dkr
 * comment changed
 *
 * Revision 2.1  1999/02/23 12:42:57  sacbase
 * new release made
 *
 * Revision 1.4  1998/05/25 13:15:42  dkr
 * signature of wlpragma-funs changed:
 *   lineno added for error-messages
 *
 * Revision 1.3  1998/04/13 19:59:06  dkr
 * added ConstSegs
 *
 * Revision 1.2  1998/04/13 18:11:13  dkr
 * rcs-header added
 *
 */

#ifndef _sac_wlpragma_funs_h_
#define _sac_wlpragma_funs_h_

#define WLP(fun, str)                                                                    \
    extern node *fun (node *segs, node *parms, node *cubes, int dims, int line);
#include "wlpragma_funs.mac"
#undef WLP

#endif /* _sac_wlpragma_funs_h_ */
