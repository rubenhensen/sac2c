/*
 *
 * $Log$
 * Revision 1.4  2005/04/13 15:27:21  ktr
 * WLSCgenerator added.
 *
 * Revision 1.3  2004/11/24 15:21:03  ktr
 * COMPILES!
 *
 * Revision 1.2  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 1.1  2004/10/07 12:35:54  ktr
 * Initial revision
 *
 */
#ifndef _SAC_WLS_H_
#define _SAC_WLS_H_

#include "types.h"

/******************************************************************************
 *
 * WLS traversal ( wls_tab)
 *
 * prefix: WLS
 *
 *****************************************************************************/
extern node *WLSdoWithloopScalarization (node *fundef);

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
extern int WLSCdoCheck (node *with);

extern node *WLSCblock (node *arg_node, info *arg_info);
extern node *WLSCcode (node *arg_node, info *arg_info);
extern node *WLSCid (node *arg_node, info *arg_info);
extern node *WLSCpart (node *arg_node, info *arg_info);
extern node *WLSCwith (node *arg_node, info *arg_info);
extern node *WLSCwithid (node *arg_node, info *arg_info);
extern node *WLSCgenerator (node *arg_node, info *arg_info);
extern node *WLSCgenarray (node *arg_node, info *arg_info);
extern node *WLSCmodarray (node *arg_node, info *arg_info);
extern node *WLSCfold (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * WLS withloopification traversal ( wlsw_tab)
 *
 * prefix: WLSW
 *
 *****************************************************************************/
extern node *WLSWdoWithloopify (node *arg_node, node *fundef, int innerdims);

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
extern node *WLSBdoBuild (node *with, node *fundef);

extern node *WLSBcode (node *arg_node, info *arg_info);
extern node *WLSBgenerator (node *arg_node, info *arg_info);
extern node *WLSBpart (node *arg_node, info *arg_info);
extern node *WLSBwith (node *arg_node, info *arg_info);
extern node *WLSBwithid (node *arg_node, info *arg_info);
extern node *WLSBgenarray (node *arg_node, info *arg_info);
extern node *WLSBmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_WLS_H_ */
