/*
 *
 * $Log$
 * Revision 1.8  2004/11/23 22:25:39  ktr
 * typo
 *
 * Revision 1.7  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.6  2004/10/10 09:55:45  ktr
 * Reference counting now works transparently over CONDFUN boundaries,
 * using the scheme presented in IFL04 paper.
 *
 * Revision 1.5  2004/07/19 12:39:37  ktr
 * Traversals for Nwithid, funcond and array reintroduced.
 *
 * Revision 1.4  2004/07/18 08:50:42  ktr
 * all functions renamed into EMsomething
 *
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

#ifndef _SAC_REFCOUNTING_H_
#define _SAC_REFCOUNTING_H_

#include "types.h"

/******************************************************************************
 *
 * Reference counting traversal (emrc_tab)
 *
 * prefix: EMRC
 *
 *****************************************************************************/
extern node *EMRCdoRefcounting (node *syntax_tree);

extern node *EMRCap (node *arg_node, info *arg_info);
extern node *EMRCarg (node *arg_node, info *arg_info);
extern node *EMRCarray (node *arg_node, info *arg_info);
extern node *EMRCassign (node *arg_node, info *arg_info);
extern node *EMRCblock (node *arg_node, info *arg_info);
extern node *EMRCcode (node *arg_node, info *arg_info);
extern node *EMRCcond (node *arg_node, info *arg_info);
extern node *EMRCfold (node *arg_node, info *arg_info);
extern node *EMRCfuncond (node *arg_node, info *arg_info);
extern node *EMRCfundef (node *arg_node, info *arg_info);
extern node *EMRCgenarray (node *arg_node, info *arg_info);
extern node *EMRCicm (node *arg_node, info *arg_info);
extern node *EMRCid (node *arg_node, info *arg_info);
extern node *EMRClet (node *arg_node, info *arg_info);
extern node *EMRCmodarray (node *arg_node, info *arg_info);
extern node *EMRCprf (node *arg_node, info *arg_info);
extern node *EMRCreturn (node *arg_node, info *arg_info);
extern node *EMRCvardec (node *arg_node, info *arg_info);
extern node *EMRCwith (node *arg_node, info *arg_info);
extern node *EMRCwith2 (node *arg_node, info *arg_info);
extern node *EMRCwithid (node *arg_node, info *arg_info);

#endif /* _SAC_REFCOUNTING_H_ */
