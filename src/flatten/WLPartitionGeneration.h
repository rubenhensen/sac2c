/*
 *
 * $Log$
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

#ifndef _WLPartitionGeneration_h
#define _WLPartitionGeneration_h

extern node *WLPartitionGeneration (node *arg_node);
extern node *WLPartitionGenerationOPT (node *arg_node);

extern node *WLPGmodul (node *arg_node, info *arg_info);
extern node *WLPGfundef (node *arg_node, info *arg_info);
extern node *WLPGassign (node *arg_node, info *arg_info);
extern node *WLPGlet (node *arg_node, info *arg_info);
extern node *WLPGap (node *arg_node, info *arg_info);

extern node *WLPGNwith (node *arg_node, info *arg_info);
extern node *WLPGNwithop (node *arg_node, info *arg_info);
extern node *WLPGNpart (node *arg_node, info *arg_info);
extern node *WLPGNgenerator (node *arg_node, info *arg_info);

#endif
