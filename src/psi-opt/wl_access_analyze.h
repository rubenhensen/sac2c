/*
 *
 * $Log$
 * Revision 3.5  2004/11/26 20:37:15  jhb
 * compile
 *
 * Revision 3.4  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 3.3  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.2  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.1  2000/11/20 18:01:57  sacbase
 * new release made
 *
 * Revision 2.3  1999/05/18 13:16:09  dkr
 * added prototype for WLAccessAnalyze()
 *
 * Revision 2.2  1999/05/18 13:09:23  dkr
 * removed prototypes for WLAAprintAccesses (moved to print.h) and
 * WLAAprintFeatures (no longer defined)
 *
 * Revision 2.1  1999/05/12 14:03:25  bs
 * new release number
 *
 * Revision 1.2  1999/05/10 12:20:46  bs
 * *** empty log message ***
 *
 *
 * Revision 1.1  1999/05/07 15:31:06  bs
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   wl_access_analyze.h
 *
 * prefix: WLAA
 *
 * description:
 *
 *   This compiler module analyzes array accesses within withloops.
 *   It is used by the tilesize inference.
 *
 *
 *****************************************************************************/

#ifndef _SAC_WL_ACCESS_ANALYZE_H_
#define _SAC_WL_ACCESS_ANALYZE_H_

#include "types.h"

extern node *WLAAdoAccessAnalyze (node *arg_node);

#ifndef WLAA_DEACTIVATED

extern node *WLAAfundef (node *arg_node, info *arg_info);
extern node *WLAAblock (node *arg_node, info *arg_info);
extern node *WLAAwith (node *arg_node, info *arg_info);
extern node *WLAAcode (node *arg_node, info *arg_info);
extern node *WLAAassign (node *arg_node, info *arg_info);
extern node *WLAAlet (node *arg_node, info *arg_info);
extern node *WLAAap (node *arg_node, info *arg_info);
extern node *WLAAid (node *arg_node, info *arg_info);
extern node *WLAAdo (node *arg_node, info *arg_info);
extern node *WLAAcond (node *arg_node, info *arg_info);
extern node *WLAAprf (node *arg_node, info *arg_info);
#endif /* WLAA_DEACTIVATED */
#endif /* _SAC_WL_ACCESS_ANALYZE_H_  */
