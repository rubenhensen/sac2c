/*
 *
 * $Log$
 * Revision 3.3  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.2  2004/06/08 14:27:46  ktr
 * New Entryfunction GetReuseCandidates yields an N_exprs chain of
 * identifiers which could be reused.
 * Important: NWITH2_DEC_RC_IDS is ignored, the caller must assure that
 * the current reference is the last in the given context.
 *
 * Revision 3.1  2000/11/20 18:01:03  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:25  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/07 18:43:12  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_ReuseWithArrays_h

#define _sac_ReuseWithArrays_h

extern node *GetReuseArrays (node *syntax_tree, node *fundef, ids *wl_ids);
extern node *GetReuseCandidates (node *syntax_tree, node *fundef, ids *wl_ids);

extern node *ReuseFundef (node *arg_node, info *arg_info);
extern node *ReuseNwith2 (node *arg_node, info *arg_info);
extern node *ReuseNwithop (node *arg_node, info *arg_info);
extern node *ReuseLet (node *arg_node, info *arg_info);
extern node *ReuseId (node *arg_node, info *arg_info);

#endif /* _sac_ReuseWithArrays_h */
