/*
 *
 * $Log$
 * Revision 3.7  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.6  2002/10/30 14:13:44  dkr
 * signature of WLP macro modified
 *
 * Revision 3.5  2001/03/20 16:04:51  ben
 * wlcomp-pragma functions SchedulingWL, SchedulingSegs added
 *
 * Revision 3.4  2001/01/08 16:12:37  dkr
 * function ExtractNaiveCompPragma added
 *
 * Revision 3.3  2001/01/08 13:40:25  dkr
 * functions ExtractAplPragma... moved from wltransform.c to
 * wlpragma_funs.c
 *
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

#ifndef _SAC_WLPRAGMA_FUNS_H_
#define _SAC_WLPRAGMA_FUNS_H_

#include "types.h"

extern bool ExtractNaiveCompPragma (node *pragma, int line);

extern node *ExtractAplPragma (node *pragma, int line);

#define WLP(fun, str, ieee)                                                              \
    extern node *fun (node *segs, node *parms, node *cubes, int dims, int line);
#include "wlpragma_funs.mac"
#undef WLP

#endif /* _SAC_WLPRAGMA_FUNS_H_ */
