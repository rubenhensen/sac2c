/*
 *
 * $Log$
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

#ifndef _sac_DupTree_h_
#define _sac_DupTree_h_

#include "LookUpTable.h"
#include "types.h"

#define DUP_NORMAL 0
#define DUP_INLINE 1
#define DUP_WLF 2

/* initializing */
extern void InitDupTree ();

/*
 * Functions for duplicating (parts of) the AST
 */
extern node *DupTree (node *arg_node);
extern node *DupTree_Type (node *arg_node, int type);
extern node *DupTreeLUT (node *arg_node, LUT_t lut);
extern node *DupTreeLUT_Type (node *arg_node, LUT_t lut, int type);
extern node *DupNode (node *arg_node);
extern node *DupNode_Type (node *arg_node, int type);
extern node *DupNodeLUT (node *arg_node, LUT_t lut);
extern node *DupNodeLUT_Type (node *arg_node, LUT_t lut, int type);

/*
 * Functions for duplicating non-node parts of the AST
 */
extern ids *DupOneIds (ids *arg_ids);
extern ids *DupAllIds (ids *arg_ids);

extern shpseg *DupShpseg (shpseg *arg_shpseg);

extern types *DupOneTypes (types *arg_types);
extern types *DupAllTypes (types *arg_types);

extern nodelist *DupNodelist (nodelist *arg_nl);

/*
 * Functions for duplicating N_id/ids and converting into ids/N_id
 */
extern node *DupIds_Id (ids *arg_ids);
extern ids *DupId_Ids (node *arg_id);

/*
 * Functions for duplicating N_id/ids and enabling NT_TAG
 */
extern node *DupIds_Id_NT (ids *arg_ids);
extern node *DupId_NT (node *arg_id);
extern node *DupNode_NT (node *arg_node);
extern node *DupExprs_NT (node *exprs);

/*
 * Other functions for external use
 */
/* handling of multiple used special functions */
extern node *CheckAndDupSpecialFundef (node *module, node *fundef, node *assign);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *DupVinfo (node *arg_node, node *arg_info);
extern node *DupNum (node *arg_node, node *arg_info);
extern node *DupBool (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupChar (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern node *DupDot (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
extern node *DupCast (node *arg_node, node *arg_info);
extern node *DupReturn (node *arg_node, node *arg_info);
extern node *DupBlock (node *arg_node, node *arg_info);
extern node *DupTypedef (node *arg_node, node *arg_info);
extern node *DupObjdef (node *arg_node, node *arg_info);
extern node *DupImplist (node *arg_node, node *arg_info);
extern node *DupVardec (node *arg_node, node *arg_info);
extern node *DupArg (node *arg_node, node *arg_info);
extern node *DupLet (node *arg_node, node *arg_info);
extern node *DupArray (node *arg_node, node *arg_info);
extern node *DupCond (node *arg_node, node *arg_info);
extern node *DupDo (node *arg_node, node *arg_info);
extern node *DupWhile (node *arg_node, node *arg_info);
extern node *DupExprs (node *arg_node, node *arg_info);
extern node *DupAssign (node *arg_node, node *arg_info);
extern node *DupEmpty (node *arg_node, node *arg_info);
extern node *DupPrf (node *arg_node, node *arg_info);
extern node *DupAp (node *arg_node, node *arg_info);
extern node *DupMop (node *arg_node, node *arg_info);
extern node *DupEmpty (node *arg_node, node *arg_info);
extern node *DupModul (node *arg_node, node *arg_info);
extern node *DupFundef (node *arg_node, node *arg_info);
extern node *DupPragma (node *arg_node, node *arg_info);
extern node *DupIcm (node *arg_node, node *arg_info);
extern node *DupSpmd (node *arg_node, node *arg_info);
extern node *DupSync (node *arg_node, node *arg_info);
extern node *DupMt (node *arg_node, node *arg_info);
extern node *DupSt (node *arg_node, node *arg_info);
extern node *DupMTsignal (node *arg_node, node *arg_info);
extern node *DupMTsync (node *arg_node, node *arg_info);
extern node *DupMTalloc (node *arg_node, node *arg_info);
extern node *DupAvis (node *arg_node, node *arg_info);
extern node *DupSSAstack (node *arg_node, node *arg_info);
extern node *DupSSAcnt (node *arg_node, node *arg_info);

/* frontend with-loop */
extern node *DupNwith (node *arg_node, node *arg_info);
extern node *DupNwithop (node *arg_node, node *arg_info);
extern node *DupNpart (node *arg_node, node *arg_info);
extern node *DupNcode (node *arg_node, node *arg_info);
extern node *DupNwithid (node *arg_node, node *arg_info);
extern node *DupNgen (node *arg_node, node *arg_info);

/* backend with-loop */
extern node *DupNwith2 (node *arg_node, node *arg_info);
extern node *DupWLseg (node *arg_node, node *arg_info);
extern node *DupWLsegVar (node *arg_node, node *arg_info);
extern node *DupWLblock (node *arg_node, node *arg_info);
extern node *DupWLublock (node *arg_node, node *arg_info);
extern node *DupWLstride (node *arg_node, node *arg_info);
extern node *DupWLstrideVar (node *arg_node, node *arg_info);
extern node *DupWLgrid (node *arg_node, node *arg_info);
extern node *DupWLgridVar (node *arg_node, node *arg_info);

/* pre- and post-processing during traversal */
extern node *DupTreeTravPre (node *arg_node, node *arg_info);
extern node *DupTreeTravPost (node *arg_node, node *arg_info);

#endif /* _sac_DupTree_h_ */
