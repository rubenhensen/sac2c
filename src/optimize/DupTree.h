/*
 *
 * $Log$
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
 * Revision 1.33  1998/05/12 15:00:52  srs
 * added constant DUP_WLF
 *
 * Revision 1.32  1998/05/06 18:56:49  dkr
 * added DupExprs
 *
 * Revision 1.31  1998/04/26 21:52:53  dkr
 * DupSPMD renamed to DupSpmd
 *
 * Revision 1.30  1998/04/25 13:19:50  dkr
 * added DupIcm
 *
 * Revision 1.29  1998/04/25 12:33:56  dkr
 * added DupNwith2
 *
 * Revision 1.28  1998/04/23 17:32:36  dkr
 * added DupSync
 *
 * Revision 1.27  1998/04/20 01:11:54  dkr
 * added DupOneIds
 *
 * Revision 1.26  1998/04/17 17:26:38  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.25  1998/04/17 11:42:17  srs
 * added DupNcode()
 *
 * Revision 1.23  1998/04/11 15:16:54  srs
 * inserted DupNwith()
 *
 * Revision 1.22  1998/04/09 21:20:57  dkr
 * renamed macros:
 *   INLINE -> DUP_INLINE
 *   NORMAL -> DUP_NORMAL
 *   INVARIANT -> DUP_INVARIANT
 *
 * Revision 1.21  1998/04/07 15:17:54  srs
 * renamed DupId to DupModarray and
 *         DupIIds to DupId.
 * Added Initialization of ID_WL to DupId.
 *
 * Revision 1.20  1998/04/02 17:41:13  dkr
 * added DupConc
 *
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

#ifndef NEWTREE
#define UNS_NO arg_info->nnode
#endif /* NEWTREE */

#define UNS_NODES arg_info->node[0]

#define DUP(s, d)                                                                        \
    d->refcnt = s->refcnt;                                                               \
    d->flag = s->flag;                                                                   \
    d->counter = s->counter;                                                             \
    d->varno = s->varno;                                                                 \
    d->lineno = s->lineno;

extern node *DupTree (node *arg_node, node *arg_info);
extern node *DupNode (node *arg_node);

extern node *DupVinfo (node *arg_node, node *arg_info);
extern node *DupInt (node *arg_node, node *arg_info);
extern node *DupChar (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern node *DupModarray (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
extern node *DupArray (node *arg_node, node *arg_info);
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
extern node *DupInfo (node *arg_node, node *arg_info);
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

#endif /* _sac_DupTree_h */
