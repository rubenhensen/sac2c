/*
 * $Log$
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

#ifndef sac_pad_collect_h

#define sac_pad_collect_h

void APcollect ();
extern node *APCarg (node *arg_node, node *arg_info);
extern node *APCvardec (node *arg_node, node *arg_info);
extern node *APCarray (node *arg_node, node *arg_info);
extern node *APCNwith (node *arg_node, node *arg_info);
extern node *APCap (node *arg_node, node *arg_info);
extern node *APCexprs (node *arg_node, node *arg_info);
extern node *APCid (node *arg_node, node *arg_info);
extern node *APCprf (node *arg_node, node *arg_info);
extern node *APCfundef (node *arg_node, node *arg_info);

#endif /* sac_pad_collect_h */
