/*
 *
 * $Log$
 * Revision 1.3  2004/07/17 10:26:04  ktr
 * changed header guards to refcounting-something...
 *
 * Revision 1.2  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2004/07/14 15:43:50  ktr
 * Initial revision
 *
 * Revision 1.8  2004/06/23 09:33:44  ktr
 * Major code brush done.
 *
 * Revision 1.7  2004/06/03 15:22:53  ktr
 * New version featuring:
 * - alloc_or_reuse
 * - fill
 * - explicit index vector allocation
 *
 * Revision 1.6  2004/05/06 17:50:43  ktr
 * Added SSARCicm
 *
 * Revision 1.5  2004/05/05 15:34:05  ktr
 * Log added.
 *
 *
 */

#ifndef _sac_refcounting_h
#define _sac_refcounting_h

extern node *SSARefCount (node *syntax_tree);

extern node *SSARCfundef (node *arg_node, info *arg_info);
extern node *SSARCassign (node *arg_node, info *arg_info);
extern node *SSARCreturn (node *arg_node, info *arg_info);
extern node *SSARCprf (node *arg_node, info *arg_info);
extern node *SSARCid (node *arg_node, info *arg_info);
extern node *SSARClet (node *arg_node, info *arg_info);
extern node *SSARCarg (node *arg_node, info *arg_info);
extern node *SSARCblock (node *arg_node, info *arg_info);
extern node *SSARCvardec (node *arg_node, info *arg_info);
extern node *SSARCap (node *arg_node, info *arg_info);
extern node *SSARCconst (node *arg_node, info *arg_info);
extern node *SSARCarray (node *arg_node, info *arg_info);
extern node *SSARCfuncond (node *arg_node, info *arg_info);
extern node *SSARCcond (node *arg_node, info *arg_info);
extern node *SSARCNwith (node *arg_node, info *arg_info);
extern node *SSARCNwith2 (node *arg_node, info *arg_info);
extern node *SSARCNcode (node *arg_node, info *arg_info);
extern node *SSARCNwithid (node *arg_node, info *arg_info);
extern node *SSARCicm (node *arg_node, info *arg_info);
#endif /* _sac_refcounting_h */
