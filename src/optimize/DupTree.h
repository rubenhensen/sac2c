/*
 *
 * $Log$
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
 * Revision 1.19  1998/04/01 23:56:15  dkr
 * added DupWLstriVar, DupWLgridVar
 *
 * Revision 1.18  1998/03/27 18:39:03  dkr
 * DupWLproj -> DupWLstride
 *
 * Revision 1.17  1998/03/21 17:35:19  dkr
 * new function DupNode added
 *
 * Revision 1.16  1998/03/16 00:23:05  dkr
 * added DupWLseg, DupWLblock, DupWLublock, DupWLproj, DupWLgrid
 *
 * Revision 1.15  1998/03/02 22:27:40  dkr
 * removed bugs in duplication of N_cond, N_do, N_while
 *
 * Revision 1.14  1998/02/12 16:57:02  dkr
 * added support for new with-loop
 *
 * Revision 1.13  1997/11/04 13:32:29  dkr
 * with defined NEWTREE node.nnode is not in use anymore
 *
 * Revision 1.12  1997/09/05 17:46:02  dkr
 * added the declaration of the new function DupShpSeg
 *
 * Revision 1.11  1996/02/21 15:07:02  cg
 * function DupFundef reimplemented. Internal information will now be copied as well.
 * added new function DupPragma
 *
 * Revision 1.10  1996/01/05  14:36:35  cg
 * added DupId for copying sons and info.id
 *
 * Revision 1.9  1995/12/20  08:19:06  cg
 * added new function DupChar to duplicate N_char nodes.
 *
 * Revision 1.8  1995/08/16  09:23:01  asi
 * renamed DupCast to DupTypes
 *
 * Revision 1.7  1995/07/28  12:57:15  asi
 * added macros INVARIANT, UNS_NO, and UNS_NODES
 * and added function DupInfo
 *
 * Revision 1.6  1995/07/24  09:08:05  asi
 * macro DUP moved from DupTree.c to DupTree.h, macro TYPES renamed to INL_TYPES
 *
 * Revision 1.5  1995/07/04  11:39:58  hw
 * DupDouble inserted
 *
 * Revision 1.4  1995/06/23  13:11:54  hw
 * functions "DupDec" & "DupFundef" inserted
 *
 * Revision 1.3  1995/06/02  11:25:48  asi
 * Added functions for all nodes below fundef node
 *
 * Revision 1.2  1995/05/03  12:41:51  asi
 * added DupPrf, DupAp and DupIds
 *
 * Revision 1.1  1995/05/01  15:32:32  asi
 * Initial revision
 *
 */

#ifndef _sac_DupTree_h

#define _sac_DupTree_h

#define DUPTYPE arg_info->flag
#define DUP_NORMAL 0
#define DUP_INLINE 1
#define DUP_INVARIANT 2
#define DUP_WLF 3

#ifndef NEWTREE
#define UNS_NO arg_info->nnode
#endif /* NEWTREE */

#define UNS_NODES arg_info->node[0]

#ifndef NEWTREE
#define DUP(s, d)                                                                        \
    d->refcnt = s->refcnt;                                                               \
    d->flag = s->flag;                                                                   \
    d->varno = s->varno;                                                                 \
    d->nnode = s->nnode;                                                                 \
    d->lineno = s->lineno;
#else
#define DUP(s, d)                                                                        \
    d->refcnt = s->refcnt;                                                               \
    d->flag = s->flag;                                                                   \
    d->varno = s->varno;                                                                 \
    d->lineno = s->lineno;
#endif /* NEWTREE */

extern node *DupTree (node *arg_node, node *arg_info);
extern node *DupNode (node *arg_node);

extern node *DupInt (node *arg_node, node *arg_info);
extern node *DupChar (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern node *DupModarray (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
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
