/*
 *
 * $Log$
 * Revision 3.41  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 3.40  2004/12/02 15:12:29  sah
 * added support for ops node
 *
 * Revision 3.39  2004/11/26 18:22:05  mwe
 * DUPglobobj added
 *
 * Revision 3.38  2004/11/26 12:22:23  mwe
 * moved some macros from .c to .h file (needed by external functions)
 *
 * Revision 3.37  2004/11/24 17:44:26  mwe
 * support for N_symbol added
 *
 * Revision 3.36  2004/11/24 17:42:22  mwe
 * support for new nodes added
 *
 * Revision 3.35  2004/11/23 19:07:39  khf
 * SacDevCampDk: compiles!
 *
 * Revision 3.34  2004/11/23 13:31:14  khf
 * more codebrushing
 *
 * Revision 3.33  2004/11/22 21:35:54  khf
 * codebrushing part2
 *
 * Revision 3.32  2004/11/22 17:04:55  khf
 * the big 2004 codebrushing session (part1)
 *
 * Revision 3.31  2004/11/21 19:48:32  khf
 * the big 2004 codebrushing event
 *
 * Revision 3.30  2004/07/31 13:51:14  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.29  2004/05/07 10:01:19  khf
 * Added functions DupTreeSSA, DupTreeLUTSSA, DupNodeSSA and
 * DupNodeLUTSSA to obtain the ssa-form
 *
 * Revision 3.28  2004/03/05 19:14:27  mwe
 * support for new node N_funcond added
 *
 * Revision 3.27  2004/03/04 17:36:11  mwe
 * DupCondfun removed
 *
 * Revision 3.26  2004/03/04 13:23:04  mwe
 * DupCondfun added
 *
 * Revision 3.25  2002/09/06 10:36:09  sah
 * added DupSetWL
 *
 * Revision 3.24  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.23  2002/07/02 09:27:43  dkr
 * DupExprs_NT() added
 *
 * Revision 3.22  2002/07/02 09:20:21  dkr
 * DupNode_NT() added
 *
 * Revision 3.21  2002/06/25 14:01:53  sbs
 * DupDot added.
 *
 * Revision 3.20  2002/04/09 15:55:02  dkr
 * some comments added
 *
 * Revision 3.19  2002/02/22 13:57:50  dkr
 * functions Dup...TypesOnly(), DupOneTypesOnly_Inplace() removed
 * (no longer needed after redesign of the TYPES structure :-)
 *
 * Revision 3.18  2002/02/20 15:59:36  dkr
 * fundef DupOneTypesOnly_Inplace() added
 *
 * Revision 3.17  2002/02/20 15:01:55  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 * fundef DupTypesOnly() renamed into DupAllTypesOnly()
 * fundefs DupOneTypes() and DupOneTypesOnly() added
 *
 * Revision 3.16  2001/12/12 11:14:14  dkr
 * functions DupIds_Id_NT, DupId_NT added
 *
 * Revision 3.15  2001/05/17 11:39:20  dkr
 * InitDupTree() added
 *
 * Revision 3.14  2001/04/26 21:06:30  dkr
 * DupTypesOnly() added
 *
 * Revision 3.13  2001/04/04 09:57:26  nmw
 * DupSSAcnt added
 *
 * Revision 3.12  2001/04/02 11:10:17  nmw
 * DupAp increments used counter of special fundefs and adds external
 * assignment to FUNDEF_EXT_ASSIGNS list
 *
 * Revision 3.11  2001/03/27 21:39:53  dkr
 * macro DUPVECT renamed into DUP_VECT and moved to internal_lib.h
 *
 * Revision 3.10  2001/03/22 13:29:32  dkr
 * DUP_INVARIANT removed
 *
 * Revision 3.9  2001/03/21 18:16:47  dkr
 * functions Dup..._Type added
 * function DupTreeInfo removed
 *
 * Revision 3.8  2001/02/15 16:56:58  nmw
 * DupSSAstack added, DupID and DupIds are now aware of the AVIS
 * attribte.
 *
 * Revision 3.7  2001/02/12 17:03:03  nmw
 * N_avis node added
 *
 * Revision 3.6  2001/02/06 01:46:16  dkr
 * no changes done
 *
 * Revision 3.3  2001/01/09 17:26:31  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * [ ... ]
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
extern node *DUPcheckAndDupSpecialFundef (node *module, node *fundef, node *assign);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *DUPvinfo (node *arg_node, info *arg_info);
extern node *DUPnum (node *arg_node, info *arg_info);
extern node *DUPbool (node *arg_node, info *arg_info);
extern node *DUPfloat (node *arg_node, info *arg_info);
extern node *DUPdouble (node *arg_node, info *arg_info);
extern node *DUPchar (node *arg_node, info *arg_info);
extern node *DUPstr (node *arg_node, info *arg_info);
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
extern node *DUPsync (node *arg_node, info *arg_info);
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

/* frontend with-loop */
extern node *DUPwith (node *arg_node, info *arg_info);
extern node *DUPgenarray (node *arg_node, info *arg_info);
extern node *DUPmodarray (node *arg_node, info *arg_info);
extern node *DUPfold (node *arg_node, info *arg_info);
extern node *DUPpart (node *arg_node, info *arg_info);
extern node *DUPcode (node *arg_node, info *arg_info);
extern node *DUPwithid (node *arg_node, info *arg_info);
extern node *DUPgenerator (node *arg_node, info *arg_info);

/* backend with-loop */
extern node *DUPwith2 (node *arg_node, info *arg_info);
extern node *DUPwlseg (node *arg_node, info *arg_info);
extern node *DUPwlsegvar (node *arg_node, info *arg_info);
extern node *DUPwlblock (node *arg_node, info *arg_info);
extern node *DUPwlublock (node *arg_node, info *arg_info);
extern node *DUPwlstride (node *arg_node, info *arg_info);
extern node *DUPwlstridevar (node *arg_node, info *arg_info);
extern node *DUPwlgrid (node *arg_node, info *arg_info);
extern node *DUPwlgridvar (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *DUPtreeTravPre (node *arg_node, info *arg_info);
extern node *DUPtreeTravPost (node *arg_node, info *arg_info);

#endif /* _SAC_DUPTREE_H_ */
