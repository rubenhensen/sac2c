/*
 * $Log$
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

#ifndef sac_pad_transform_h

#define sac_pad_transform_h

extern void APtransform ();
extern node *APTarg (node *arg_node, node *arg_info);
extern node *APTvardec (node *arg_node, node *arg_info);
extern node *APTarray (node *arg_node, node *arg_info);
extern node *APTNwith (node *arg_node, node *arg_info);
#endif /* sac_pad_transform_h */
