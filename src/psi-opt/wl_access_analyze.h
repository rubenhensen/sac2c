/*
 *
 * $Log$
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

#ifndef _wl_access_analyze_h

#define _wl_access_analyze_h

extern node *WLAccessAnalyze (node *arg_node);

extern node *WLAAfundef (node *arg_node, node *arg_info);
extern node *WLAAblock (node *arg_node, node *arg_info);
extern node *WLAAnwith (node *arg_node, node *arg_info);
extern node *WLAAncode (node *arg_node, node *arg_info);
extern node *WLAAassign (node *arg_node, node *arg_info);
extern node *WLAAlet (node *arg_node, node *arg_info);
extern node *WLAAap (node *arg_node, node *arg_info);
extern node *WLAAid (node *arg_node, node *arg_info);
extern node *WLAAwhile (node *arg_node, node *arg_info);
extern node *WLAAdo (node *arg_node, node *arg_info);
extern node *WLAAcond (node *arg_node, node *arg_info);
extern node *WLAAprf (node *arg_node, node *arg_info);

extern void WLAAprintAccesses (node *arg_node, node *arg_info);
extern void WLAAprintFeatures (node *arg_node, node *arg_info);

#endif /* _wl_access_analyze_h  */
