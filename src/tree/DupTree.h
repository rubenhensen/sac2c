/*
 *
 * $Log$
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
 * Functions for duplicating N_id/ids and converting into ids/N_id
 */
extern node *DUPdupIdsId (ids *arg_ids);
extern ids *DUPdupIdIds (node *arg_id);

/*
 * Functions for duplicating N_id/ids and enabling NT_TAG
 */
extern node *DUPdupIdsIdNt (ids *arg_ids);
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
extern node *DUPdupVinfo (node *arg_node, info *arg_info);
extern node *DUPdupNum (node *arg_node, info *arg_info);
extern node *DUPdupBool (node *arg_node, info *arg_info);
extern node *DUPdupFloat (node *arg_node, info *arg_info);
extern node *DUPdupDouble (node *arg_node, info *arg_info);
extern node *DUPdupChar (node *arg_node, info *arg_info);
extern node *DUPdupStr (node *arg_node, info *arg_info);
extern node *DUPdupDot (node *arg_node, info *arg_info);
extern node *DUPdupSetWl (node *arg_node, info *arg_info);
extern node *DUPdupId (node *arg_node, info *arg_info);
extern node *DUPdupCast (node *arg_node, info *arg_info);
extern node *DUPdupReturn (node *arg_node, info *arg_info);
extern node *DUPdupBlock (node *arg_node, info *arg_info);
extern node *DUPdupTypedef (node *arg_node, info *arg_info);
extern node *DUPdupObjdef (node *arg_node, info *arg_info);
extern node *DUPdupImplist (node *arg_node, info *arg_info);
extern node *DUPdupVardec (node *arg_node, info *arg_info);
extern node *DUPdupArg (node *arg_node, info *arg_info);
extern node *DUPdupLet (node *arg_node, info *arg_info);
extern node *DUPdupArray (node *arg_node, info *arg_info);
extern node *DUPdupCond (node *arg_node, info *arg_info);
extern node *DUPdupDo (node *arg_node, info *arg_info);
extern node *DUPdupWhile (node *arg_node, info *arg_info);
extern node *DUPdupExprs (node *arg_node, info *arg_info);
extern node *DUPdupAssign (node *arg_node, info *arg_info);
extern node *DUPdupEmpty (node *arg_node, info *arg_info);
extern node *DUPdupPrf (node *arg_node, info *arg_info);
extern node *DUPdupAp (node *arg_node, info *arg_info);
extern node *DUPdupMop (node *arg_node, info *arg_info);
extern node *DUPdupEmpty (node *arg_node, info *arg_info);
extern node *DUPdupModule (node *arg_node, info *arg_info);
extern node *DUPdupFundef (node *arg_node, info *arg_info);
extern node *DUPdupPragma (node *arg_node, info *arg_info);
extern node *DUPdupIcm (node *arg_node, info *arg_info);
extern node *DUPdupSpmd (node *arg_node, info *arg_info);
extern node *DUPdupSync (node *arg_node, info *arg_info);
extern node *DUPdupMt (node *arg_node, info *arg_info);
extern node *DUPdupSt (node *arg_node, info *arg_info);
extern node *DUPdupMtsignal (node *arg_node, info *arg_info);
extern node *DUPdupMtsync (node *arg_node, info *arg_info);
extern node *DUPdupMtalloc (node *arg_node, info *arg_info);
extern node *DUPdupAvis (node *arg_node, info *arg_info);
extern node *DUPdupSsastack (node *arg_node, info *arg_info);
extern node *DUPdupSsacnt (node *arg_node, info *arg_info);
extern node *DUPdupFuncond (node *arg_node, info *arg_info);

/* frontend with-loop */
extern node *DUPdupWith (node *arg_node, info *arg_info);
extern node *DUPdupGenarray (node *arg_node, info *arg_info);
extern node *DUPdupModarray (node *arg_node, info *arg_info);
extern node *DUPdupFold (node *arg_node, info *arg_info);
extern node *DUPdupPart (node *arg_node, info *arg_info);
extern node *DUPdupCode (node *arg_node, info *arg_info);
extern node *DUPdupWithid (node *arg_node, info *arg_info);
extern node *DUPdupGenerator (node *arg_node, info *arg_info);

/* backend with-loop */
extern node *DUPdupWith2 (node *arg_node, info *arg_info);
extern node *DUPdupWlseg (node *arg_node, info *arg_info);
extern node *DUPdupWlsegVar (node *arg_node, info *arg_info);
extern node *DUPdupWlblock (node *arg_node, info *arg_info);
extern node *DUPdupWlublock (node *arg_node, info *arg_info);
extern node *DUPdupWlstride (node *arg_node, info *arg_info);
extern node *DUPdupWlstrideVar (node *arg_node, info *arg_info);
extern node *DUPdupWlgrid (node *arg_node, info *arg_info);
extern node *DUPdupWlgridVar (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *DUPdupTreeTravPre (node *arg_node, info *arg_info);
extern node *DUPdupTreeTravPost (node *arg_node, info *arg_info);

#endif /* _SAC_DUPTREE_H_ */
