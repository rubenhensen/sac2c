/*
 *
 * $Log$
 * Revision 1.7  2004/12/10 18:41:38  ktr
 * EMRCOfundef added.
 *
 * Revision 1.6  2004/12/09 21:09:26  ktr
 * bugfix roundup
 *
 * Revision 1.5  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.4  2004/10/22 15:18:40  ktr
 * Moved some functionality into reuseelimination.c
 *
 * Revision 1.3  2004/10/21 16:22:07  ktr
 * Added support for static reuse.
 *
 * Revision 1.2  2004/10/11 14:49:27  ktr
 * alloc/with/inc_rc combinations are now treated as well.
 *
 * Revision 1.1  2004/10/10 09:55:30  ktr
 * Initial revision
 *
 */
#ifndef _SAC_RCOPT_H_
#define _SAC_RCOPT_H_

#include "types.h"

/******************************************************************************
 *
 * Reference counting optimizations traversal (emrco_tab)
 *
 * prefix: EMRCO
 *
 *****************************************************************************/
extern node *EMRCOdoRefCountOpt (node *syntax_tree);

extern node *EMRCOassign (node *arg_node, info *arg_info);
extern node *EMRCOblock (node *arg_node, info *arg_info);
extern node *EMRCOfold (node *arg_node, info *arg_info);
extern node *EMRCOfundef (node *arg_node, info *arg_info);
extern node *EMRCOgenarray (node *arg_node, info *arg_info);
extern node *EMRCOlet (node *arg_node, info *arg_info);
extern node *EMRCOmodarray (node *arg_node, info *arg_info);
extern node *EMRCOprf (node *arg_node, info *arg_info);

#endif /* _SAC_RCOPT_H_ */
