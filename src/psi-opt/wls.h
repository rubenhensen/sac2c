/*
 *
 * $Log$
 * Revision 1.1  2004/10/07 12:35:54  ktr
 * Initial revision
 *
 */
#ifndef _wls_h
#define _wls_h

/******************************************************************************
 *
 * WLS traversal ( wls_tab)
 *
 * prefix: WLS
 *
 *****************************************************************************/
extern node *WLSWithloopScalarization (node *fundef);

extern node *WLSap (node *arg_node, info *arg_info);
extern node *WLSassign (node *arg_node, info *arg_info);
extern node *WLSfundef (node *arg_node, info *arg_info);
extern node *WLSwith (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * WLS check traversal ( wlsc_tab)
 *
 * prefix: WLSC
 *
 *****************************************************************************/
extern int WLSCheck (node *with);

extern node *WLSCblock (node *arg_node, info *arg_info);
extern node *WLSCcode (node *arg_node, info *arg_info);
extern node *WLSCid (node *arg_node, info *arg_info);
extern node *WLSCpart (node *arg_node, info *arg_info);
extern node *WLSCwith (node *arg_node, info *arg_info);
extern node *WLSCwithid (node *arg_node, info *arg_info);
extern node *WLSCwithop (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * WLS withloopification traversal ( wlsw_tab)
 *
 * prefix: WLSW
 *
 *****************************************************************************/
extern node *WLSWithloopify (node *arg_node, node *fundef, int innerdims);

extern node *WLSWcode (node *arg_node, info *arg_info);
extern node *WLSWid (node *arg_node, info *arg_info);
extern node *WLSWlet (node *arg_node, info *arg_info);
extern node *WLSWpart (node *arg_node, info *arg_info);
extern node *WLSWwith (node *arg_node, info *arg_info);
extern node *WLSWwithid (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * WLS build traversal ( wlsb_tab)
 *
 * prefix: WLSB
 *
 *****************************************************************************/
extern node *WLSBuild (node *with, node *fundef);

extern node *WLSBcode (node *arg_node, info *arg_info);
extern node *WLSBgen (node *arg_node, info *arg_info);
extern node *WLSBpart (node *arg_node, info *arg_info);
extern node *WLSBwith (node *arg_node, info *arg_info);
extern node *WLSBwithid (node *arg_node, info *arg_info);
extern node *WLSBwithop (node *arg_node, info *arg_info);

#endif
