/*
 *
 * $Log$
 * Revision 1.3  2000/01/26 23:26:12  dkr
 * DupTreePre() and DupTreePost() added.
 * Some code brushing done.
 *
 * Revision 1.2  2000/01/26 17:27:53  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.1  2000/01/21 11:16:27  dkr
 * Initial revision
 *
 * Revision 2.6  1999/09/01 17:11:01  jhs
 * Fixed Duplicating of masks in DupAssign.
 *
 * Revision 2.5  1999/07/07 15:04:25  sbs
 * DupVinfo added; it implicitly generates consistent VINFO_DOLLAR
 * pointers!!!
 *
 * Revision 2.4  1999/04/13 14:01:48  cg
 * added function DupBlock for duplication of N_block nodes.
 *
 * Revision 2.3  1999/03/17 16:16:31  bs
 * Macro DUP(s,d) modified. Now s->counter will also be duplicated.
 *
 * Revision 2.2  1999/02/25 10:58:18  bs
 * DupArray added
 *
 * Revision 2.1  1999/02/23 12:41:18  sacbase
 * new release made
 *
 * Revision 1.34  1998/08/07 14:37:06  dkr
 * DupWLsegVar added
 *
 * [ ... ]
 *
 * Revision 1.1  1995/05/01  15:32:32  asi
 * Initial revision
 *
 */

#ifndef _sac_DupTree_h
#define _sac_DupTree_h

#define DUP_NORMAL 0
#define DUP_INLINE 1
#define DUP_INVARIANT 2
#define DUP_WLF 3

extern node *DupTree (node *arg_node, node *arg_info);
extern node *DupNode (node *arg_node);

extern node *DupVinfo (node *arg_node, node *arg_info);
extern node *DupNum (node *arg_node, node *arg_info);
extern node *DupBool (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupChar (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
extern node *DupArray (node *arg_node, node *arg_info);
extern node *DupLet (node *arg_node, node *arg_info);
extern node *DupExprs (node *arg_node, node *arg_info);
extern node *DupCond (node *arg_node, node *arg_info);
extern node *DupLoop (node *arg_node, node *arg_info);
extern node *DupChain (node *arg_node, node *arg_info);
extern node *DupAssign (node *arg_node, node *arg_info);
extern node *DupTypes (node *arg_node, node *arg_info);
extern node *DupPrf (node *arg_node, node *arg_info);
extern node *DupFun (node *arg_node, node *arg_info);
extern node *DupFundef (node *arg_node, node *arg_info);
extern node *DupDec (node *arg_node, node *arg_info);
extern node *DupPragma (node *arg_node, node *arg_info);
extern node *DupIcm (node *arg_node, node *arg_info);
extern node *DupSpmd (node *arg_node, node *arg_info);
extern node *DupSync (node *arg_node, node *arg_info);
extern node *DupBlock (node *arg_node, node *arg_info);

/* new with-loop */
extern node *DupNwith (node *arg_node, node *arg_info);
extern node *DupNwithop (node *arg_node, node *arg_info);
extern node *DupNpart (node *arg_node, node *arg_info);
extern node *DupNcode (node *arg_node, node *arg_info);
extern node *DupNwithid (node *arg_node, node *arg_info);
extern node *DupNgen (node *arg_node, node *arg_info);

extern node *DupNwith2 (node *arg_node, node *arg_info);
extern node *DupWLseg (node *arg_node, node *arg_info);
extern node *DupWLblock (node *arg_node, node *arg_info);
extern node *DupWLublock (node *arg_node, node *arg_info);
extern node *DupWLstride (node *arg_node, node *arg_info);
extern node *DupWLgrid (node *arg_node, node *arg_info);
extern node *DupWLsegVar (node *arg_node, node *arg_info);
extern node *DupWLstriVar (node *arg_node, node *arg_info);
extern node *DupWLgridVar (node *arg_node, node *arg_info);

extern ids *DupOneIds (ids *ids, node *arg_info);
extern ids *DupIds (ids *ids, node *arg_info);
extern shpseg *DupShpSeg (shpseg *shp_seg);

extern node *DupTreePre (node *arg_node, node *arg_info);
extern node *DupTreePost (node *arg_node, node *arg_info);

#endif /* _sac_DupTree_h */
