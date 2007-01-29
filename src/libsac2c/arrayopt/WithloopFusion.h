/*
 *
 * $Log$
 * Revision 1.7  2005/09/04 12:54:00  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.6  2004/11/22 17:57:54  khf
 * more codebrushing
 *
 * Revision 1.5  2004/11/21 20:18:08  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.4  2004/11/07 19:26:35  khf
 * added WLFSblock
 *
 * Revision 1.3  2004/07/22 17:28:37  khf
 * added WLFSap
 *
 * Revision 1.2  2004/07/21 12:47:35  khf
 * switch to new INFO structure
 *
 * Revision 1.1  2004/04/08 08:15:52  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_WITHLOOPFUSION_H_
#define _SAC_WITHLOOPFUSION_H_

#include "types.h"

/******************************************************************************
 *
 * With-Loop-Fusion traversal ( wlfs_tab)
 *
 * Prefix: WLFS
 *
 *****************************************************************************/
extern node *WLFSdoWithloopFusion (node *arg_node);

extern node *WLFSfundef (node *arg_node, info *arg_info);
extern node *WLFSblock (node *arg_node, info *arg_info);
extern node *WLFSassign (node *arg_node, info *arg_info);
extern node *WLFSid (node *arg_node, info *arg_info);

extern node *WLFSwith (node *arg_node, info *arg_info);
extern node *WLFSgenarray (node *arg_node, info *arg_info);
extern node *WLFSmodarray (node *arg_node, info *arg_info);
extern node *WLFSfold (node *arg_node, info *arg_info);
extern node *WLFSpart (node *arg_node, info *arg_info);
extern node *WLFSgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOPFUSION_H_ */
