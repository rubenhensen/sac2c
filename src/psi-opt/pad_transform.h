/*
 * $Log$
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
 *   This compiler module appplies array padding.
 *
 *
 *****************************************************************************/

#ifndef sac_pad_transform_h

#define sac_pad_transform_h

extern void APtransform ();

extern node *APTarg (node *arg_node, node *arg_info);
extern node *APTvardec (node *arg_node, node *arg_info);
extern node *APTassign (node *arg_node, node *arg_info);
extern node *APTarray (node *arg_node, node *arg_info);
extern node *APTwith (node *arg_node, node *arg_info);
extern node *APTcode (node *arg_node, node *arg_info);
extern node *APTwithop (node *arg_node, node *arg_info);
extern node *APTap (node *arg_node, node *arg_info);
extern node *APTid (node *arg_node, node *arg_info);
extern node *APTprf (node *arg_node, node *arg_info);
extern node *APTfundef (node *arg_node, node *arg_info);
extern node *APTblock (node *arg_node, node *arg_info);
extern node *APTlet (node *arg_node, node *arg_info);

#endif /* sac_pad_transform_h */
