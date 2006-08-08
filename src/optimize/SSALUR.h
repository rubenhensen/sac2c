/*
 * $Log$
 * Revision 1.6  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.5  2004/11/22 18:33:19  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.4  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.3  2001/05/25 08:42:18  nmw
 * comments added, code beautyfied
 *
 * Revision 1.2  2001/05/07 09:03:00  nmw
 * withloop unrolling by WLUnroll integrated in traversal
 *
 * Revision 1.1  2001/04/20 11:20:56  nmw
 * Initial revision
 *
 *
 */

#ifndef _SAC_SSALUR_H_
#define _SAC_SSALUR_H_

#include "types.h"

/*****************************************************************************
 *
 * SSALUR
 *
 * prefix: LUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. all while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *
 *****************************************************************************/
extern node *LURdoLoopUnrolling (node *fundef);

extern node *LURfundef (node *arg_node, info *arg_info);
extern node *LURassign (node *arg_node, info *arg_info);
extern node *LURap (node *arg_node, info *arg_info);

#endif /* _SAC_SSALUR_H_ */
