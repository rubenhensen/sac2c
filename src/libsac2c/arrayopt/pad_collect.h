/*
 * $Log$
 * Revision 3.3  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 3.2  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:01:49  sacbase
 * new release made
 *
 * Revision 1.8  2000/08/03 15:32:00  mab
 * completed collection of access patterns and unsupported shapes
 *
 * Revision 1.7  2000/07/21 14:43:43  mab
 * added APCcode and APCwithop dummies
 *
 * Revision 1.6  2000/06/29 10:23:05  mab
 * renamed APCNwith to APCwith
 *
 * Revision 1.5  2000/06/15 14:38:01  mab
 * dummies for APC block and let added
 *
 * Revision 1.4  2000/06/14 10:43:19  mab
 * dummies for APC ap, exprs, id, prf, fundef added
 *
 * Revision 1.3  2000/06/08 11:13:37  mab
 * added functions for nodes arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:03  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad_collect.h
 *
 * prefix: APC
 *
 * description:
 *
 *   This compiler module collects information needed to infer new array
 *   shapes for the inference-phase.
 *
 *
 *****************************************************************************/

#ifndef _SAC_PAD_COLLECT_H_
#define _SAC_PAD_COLLECT_H_

#include "types.h"

extern void APCdoCollect ();
extern node *APCarray (node *arg_node, info *arg_info);
extern node *APCwith (node *arg_node, info *arg_info);
extern node *APCap (node *arg_node, info *arg_info);
extern node *APCid (node *arg_node, info *arg_info);
extern node *APCprf (node *arg_node, info *arg_info);
extern node *APCfundef (node *arg_node, info *arg_info);
extern node *APClet (node *arg_node, info *arg_info);
extern node *APCgenarray (node *arg_node, info *arg_info);
extern node *APCmodarray (node *arg_node, info *arg_info);
extern node *APCfold (node *arg_node, info *arg_info);
extern node *APCcode (node *arg_node, info *arg_info);

#endif /* _SAC_PAD_COLLECT_H_ */
