/*
 * $Log$
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

void APTpadShpseg (int dim, shpseg *old, shpseg *new);
char *APTpadName (char *unpaddedName);

extern node *APTarg (node *arg_node, node *arg_info);
extern node *APTvardec (node *arg_node, node *arg_info);
extern node *APTarray (node *arg_node, node *arg_info);
extern node *APTNwith (node *arg_node, node *arg_info);
extern node *APTap (node *arg_node, node *arg_info);
extern node *APTexprs (node *arg_node, node *arg_info);
extern node *APTid (node *arg_node, node *arg_info);
extern node *APTprf (node *arg_node, node *arg_info);
extern node *APTfundef (node *arg_node, node *arg_info);

#endif /* sac_pad_transform_h */
