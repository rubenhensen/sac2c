/*
 * $Log$
 * Revision 3.5  2004/11/26 18:44:43  skt
 * removew APTwithop by APTgenarray, APTmodarray, APTfold
 *
 * Revision 3.4  2004/11/26 18:22:51  jhb
 * compile
 *
 * Revision 3.3  2004/11/22 16:34:41  sbs
 * SacDevCamp04
 *
 * Revision 3.2  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:01:54  sacbase
 * new release made
 *
 * Revision 1.9  2000/08/03 15:36:33  mab
 * debugged transformation
 * (conversion functions not yet supported)
 *
 * Revision 1.8  2000/07/07 12:05:27  mab
 * added APTassign
 *
 * Revision 1.7  2000/06/29 10:23:38  mab
 * added dummy functions for APTpart, APTwithid, APTgenerator, APTcode, APTwithop
 * renamed APTNwith to APTwith
 *
 * Revision 1.6  2000/06/28 10:41:35  mab
 * completed padding functions except with node
 * some code modifications according to code review
 *
 * Revision 1.5  2000/06/15 14:38:55  mab
 * implemented APTfundef, APTblock, APTid
 * dummy for APTlet added
 *
 * Revision 1.4  2000/06/14 10:46:04  mab
 * implemented APTvardec, APTarg
 * added dummies for APT ap, exprs, id, prf, fundef
 *
 * Revision 1.3  2000/06/08 11:14:14  mab
 * added functions for arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:35  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad_transform.h
 *
 * prefix: APT
 *
 * description:
 *
 *   This compiler module applies array padding.
 *
 *
 *****************************************************************************/

#ifndef _SAC_PAD_TRANSFORM_H_
#define _SAC_PAD_TRANSFORM_H_

extern void APTdoTransform ();

extern node *APTarg (node *arg_node, info *arg_info);
extern node *APTvardec (node *arg_node, info *arg_info);
extern node *APTassign (node *arg_node, info *arg_info);
extern node *APTarray (node *arg_node, info *arg_info);
extern node *APTwith (node *arg_node, info *arg_info);
extern node *APTcode (node *arg_node, info *arg_info);
extern node *APTgenarray (node *arg_node, info *arg_info);
extern node *APTmodarray (node *arg_node, info *arg_info);
extern node *APTfold (node *arg_node, info *arg_info);
extern node *APTap (node *arg_node, info *arg_info);
extern node *APTid (node *arg_node, info *arg_info);
extern node *APTprf (node *arg_node, info *arg_info);
extern node *APTfundef (node *arg_node, info *arg_info);
extern node *APTblock (node *arg_node, info *arg_info);
extern node *APTlet (node *arg_node, info *arg_info);

#endif /* _SAC_PAD_TRANSFORM_H_ */
