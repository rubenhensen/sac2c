/*
 *
 * $Log$
 * Revision 1.7  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.6  2004/07/22 17:26:23  khf
 * added WLPGap
 *
 * Revision 1.5  2004/07/21 12:48:33  khf
 * switch to new INFO structure
 * take changes of sbs in SSAWLT.c over
 *
 * Revision 1.4  2004/04/08 08:13:25  khf
 * some corrections and new startfunction WLPartitionGenerationOPT
 * added
 *
 * Revision 1.3  2004/03/02 09:21:43  khf
 * WLPGlet added
 *
 * Revision 1.2  2004/02/26 13:11:01  khf
 * WLPartitionGeneration implemented in parts (but not tested)
 *
 * Revision 1.1  2004/02/25 13:17:02  khf
 * Initial revision
 *
 *
 */

#include "types.h"

#ifndef _SAC_WLPARTIONGENERATION_H_
#define _SAC_WLPARTIONGENERATION_H_

extern node *WLPGdoWlPartitionGeneration (node *arg_node);
extern node *WLPGdoWlPartitionGenerationOpt (node *arg_node);

extern node *WLPGmodule (node *arg_node, info *arg_info);
extern node *WLPGfundef (node *arg_node, info *arg_info);
extern node *WLPGassign (node *arg_node, info *arg_info);
extern node *WLPGlet (node *arg_node, info *arg_info);
extern node *WLPGap (node *arg_node, info *arg_info);

extern node *WLPGwith (node *arg_node, info *arg_info);
extern node *WLPGgenarray (node *arg_node, info *arg_info);
extern node *WLPGmodarray (node *arg_node, info *arg_info);
extern node *WLPGfold (node *arg_node, info *arg_info);
extern node *WLPGpart (node *arg_node, info *arg_info);
extern node *WLPGgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_WLPARTIONGENERATION_H_ */
