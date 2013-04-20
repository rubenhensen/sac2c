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

extern void APTdoTransform (void);

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
