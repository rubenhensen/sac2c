/*
 *
 * $Log$
 * Revision 1.2  2005/09/15 17:12:13  ktr
 * fixed bug #113
 * DEEP BRUSH
 *
 * Revision 1.1  2004/11/25 19:26:56  ktr
 * Initial revision
 *
 * Revision 3.4  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
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
 */
#ifndef _SAC_REUSEWITHARRAYS_H_
#define _SAC_REUSEWITHARRAYS_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse with-arrays traversal ( reuse_tab)
 *
 * Prefix: REUSE
 *
 *****************************************************************************/
extern node *REUSEdoGetReuseArrays (node *syntax_tree, node *fundef, node *wl_ids);

extern node *REUSEfold (node *arg_node, info *arg_info);
extern node *REUSEgenarray (node *arg_node, info *arg_info);
extern node *REUSEid (node *arg_node, info *arg_info);
extern node *REUSElet (node *arg_node, info *arg_info);
extern node *REUSEmodarray (node *arg_node, info *arg_info);
extern node *REUSEwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEWITHARRAYS_H_ */
