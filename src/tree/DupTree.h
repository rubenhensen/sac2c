/*
 *
 * $Id$
 *
 */

#ifndef _SAC_DUPTREE_H_
#define _SAC_DUPTREE_H_

#include "types.h"

/*
 * following macros are also needed by external functions
 */

#define DUP_NORMAL 0
#define DUP_INLINE 1
#define DUP_WLF 2
#define DUP_SSA 3

/* initializing */
extern void DUPinitDupTree ();

/*
 * Functions for duplicating (parts of) the AST
 */
extern node *DUPdoDupTree (node *arg_node);
extern node *DUPdoDupTreeSsa (node *arg_node, node *fundef);
extern node *DUPdoDupTreeType (node *arg_node, int type);
extern node *DUPdoDupTreeLut (node *arg_node, lut_t *lut);
extern node *DUPdoDupTreeLutSsa (node *arg_node, lut_t *lut, node *fundef);
extern node *DUPdoDupTreeLutType (node *arg_node, lut_t *lut, int type);
extern node *DUPdoDupNode (node *arg_node);
extern node *DUPdoDupNodeSsa (node *arg_node, node *fundef);
extern node *DUPdoDupNodeType (node *arg_node, int type);
extern node *DUPdoDupNodeLut (node *arg_node, lut_t *lut);
extern node *DUPdoDupNodeLutSsa (node *arg_node, lut_t *lut, node *fundef);
extern node *DUPdoDupNodeLutType (node *arg_node, lut_t *lut, int type);

/*
 * Functions for duplicating non-node parts of the AST
 */
extern shpseg *DUPdupShpseg (shpseg *arg_shpseg);

extern types *DUPdupOneTypes (types *arg_types);
extern types *DUPdupAllTypes (types *arg_types);

extern nodelist *DUPdupNodelist (nodelist *arg_nl);

/*
 * Functions for duplicating N_id/N_ids and converting into N_ids/N_id
 */
extern node *DUPdupIdsId (node *arg_ids);
extern node *DUPdupIdIds (node *arg_id);

/*
 * Functions for duplicating N_id/ids and enabling NT_TAG
 */
extern node *DUPdupIdsIdNt (node *arg_ids);
extern node *DUPdupIdNt (node *arg_id);
extern node *DUPdupNodeNt (node *arg_node);
extern node *DUPdupExprsNt (node *exprs);

/*
 * Other functions for external use
 */
/* handling of multiple used special functions */
extern node *DUPgetCopiedSpecialFundefs ();

/*
 * Functions for internal use during AST traversal only!
 */
extern node *DUPnum (node *arg_node, info *arg_info);
extern node *DUPbool (node *arg_node, info *arg_info);
extern node *DUPfloat (node *arg_node, info *arg_info);
extern node *DUPdouble (node *arg_node, info *arg_info);
extern node *DUPchar (node *arg_node, info *arg_info);
extern node *DUPstr (node *arg_node, info *arg_info);
extern node *DUPtype (node *arg_node, info *arg_info);
extern node *DUPdot (node *arg_node, info *arg_info);
extern node *DUPsetwl (node *arg_node, info *arg_info);
extern node *DUPid (node *arg_node, info *arg_info);
extern node *DUPspid (node *arg_node, info *arg_info);
extern node *DUPcast (node *arg_node, info *arg_info);
extern node *DUPreturn (node *arg_node, info *arg_info);
extern node *DUPblock (node *arg_node, info *arg_info);
extern node *DUPtypedef (node *arg_node, info *arg_info);
extern node *DUPobjdef (node *arg_node, info *arg_info);
extern node *DUPvardec (node *arg_node, info *arg_info);
extern node *DUParg (node *arg_node, info *arg_info);
extern node *DUPret (node *arg_node, info *arg_info);
extern node *DUPlet (node *arg_node, info *arg_info);
extern node *DUPids (node *arg_node, info *arg_info);
extern node *DUPspids (node *arg_node, info *arg_info);
extern node *DUParray (node *arg_node, info *arg_info);
extern node *DUPcond (node *arg_node, info *arg_info);
extern node *DUPdo (node *arg_node, info *arg_info);
extern node *DUPwhile (node *arg_node, info *arg_info);
extern node *DUPexprs (node *arg_node, info *arg_info);
extern node *DUPassign (node *arg_node, info *arg_info);
extern node *DUPempty (node *arg_node, info *arg_info);
extern node *DUPprf (node *arg_node, info *arg_info);
extern node *DUPap (node *arg_node, info *arg_info);
extern node *DUPspap (node *arg_node, info *arg_info);
extern node *DUPspmop (node *arg_node, info *arg_info);
extern node *DUPempty (node *arg_node, info *arg_info);
extern node *DUPmodule (node *arg_node, info *arg_info);
extern node *DUPfundef (node *arg_node, info *arg_info);
extern node *DUPpragma (node *arg_node, info *arg_info);
extern node *DUPicm (node *arg_node, info *arg_info);
extern node *DUPspmd (node *arg_node, info *arg_info);
extern node *DUPmt (node *arg_node, info *arg_info);
extern node *DUPst (node *arg_node, info *arg_info);
extern node *DUPavis (node *arg_node, info *arg_info);
extern node *DUPssastack (node *arg_node, info *arg_info);
extern node *DUPssacnt (node *arg_node, info *arg_info);
extern node *DUPfuncond (node *arg_node, info *arg_info);
extern node *DUPcseinfo (node *arg_node, info *arg_info);
extern node *DUPannotate (node *arg_node, info *arg_info);
extern node *DUPex (node *arg_node, info *arg_info);
extern node *DUPcwrapper (node *arg_node, info *arg_info);
extern node *DUPdataflownode (node *arg_node, info *arg_info);
extern node *DUPdataflowgraph (node *arg_node, info *arg_info);
extern node *DUPimport (node *arg_node, info *arg_info);
extern node *DUPexport (node *arg_node, info *arg_info);
extern node *DUPuse (node *arg_node, info *arg_info);
extern node *DUPprovide (node *arg_node, info *arg_info);
extern node *DUPlinklist (node *arg_node, info *arg_info);
extern node *DUPnums (node *arg_node, info *arg_info);
extern node *DUPsymbol (node *arg_node, info *arg_info);
extern node *DUPglobobj (node *arg_node, info *arg_info);
extern node *DUPerror (node *arg_node, info *arg_info);
extern node *DUPfungroup (node *arg_node, info *arg_info);

/* frontend with-loop */
extern node *DUPwith (node *arg_node, info *arg_info);
extern node *DUPgenarray (node *arg_node, info *arg_info);
extern node *DUPmodarray (node *arg_node, info *arg_info);
extern node *DUPfold (node *arg_node, info *arg_info);
extern node *DUPpart (node *arg_node, info *arg_info);
extern node *DUPcode (node *arg_node, info *arg_info);
extern node *DUPwithid (node *arg_node, info *arg_info);
extern node *DUPgenerator (node *arg_node, info *arg_info);
extern node *DUPdefault (node *arg_node, info *arg_info);

/* backend with-loop */
extern node *DUPwith2 (node *arg_node, info *arg_info);
extern node *DUPwlseg (node *arg_node, info *arg_info);
extern node *DUPwlsegvar (node *arg_node, info *arg_info);
extern node *DUPwlblock (node *arg_node, info *arg_info);
extern node *DUPwlublock (node *arg_node, info *arg_info);
extern node *DUPwlsimd (node *arg_node, info *arg_info);
extern node *DUPwlstride (node *arg_node, info *arg_info);
extern node *DUPwlstridevar (node *arg_node, info *arg_info);
extern node *DUPwlgrid (node *arg_node, info *arg_info);
extern node *DUPwlgridvar (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *DUPtreeTravPre (node *arg_node, info *arg_info);
extern node *DUPtreeTravPost (node *arg_node, info *arg_info);

#endif /* _SAC_DUPTREE_H_ */
