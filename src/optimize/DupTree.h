/*
 *
 * $Log$
 * Revision 1.8  1995/08/16 09:23:01  asi
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

#ifndef _DupTree_h

#define _DupTree_h

#define DUPTYPE arg_info->flag
#define NORMAL 0
#define INLINE 1
#define INVARIANT 2
#define UNS_NO arg_info->nnode
#define UNS_NODES arg_info->node[0]

#define DUP(s, d)                                                                        \
    d->refcnt = s->refcnt;                                                               \
    d->flag = s->flag;                                                                   \
    d->varno = s->varno;                                                                 \
    d->nnode = s->nnode;                                                                 \
    d->lineno = s->lineno;

extern node *DupTree (node *arg_node, node *arg_info);

extern node *DupInt (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern ids *DupIds (ids *ids, node *arg_info);
extern node *DupIIds (node *arg_node, node *arg_info);
extern node *DupChain (node *arg_node, node *arg_info);
extern node *DupAssign (node *arg_node, node *arg_info);
extern node *DupTypes (node *arg_node, node *arg_info);
extern node *DupPrf (node *arg_node, node *arg_info);
extern node *DupFun (node *arg_node, node *arg_info);
extern node *DupFundef (node *arg_node, node *arg_info);
extern node *DupDec (node *arg_node, node *arg_info);
extern node *DupInfo (node *arg_node, node *arg_info);

#endif /* _DupTree_h */
