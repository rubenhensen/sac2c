/*
 *
 * $Log$
 * Revision 1.2  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.1  2004/11/09 22:15:14  ktr
 * Initial revision
 *
 */
#ifndef _SAC_EXPLICITCOPY_H_
#define _SAC_EXPLICITCOPY_H_

#include "types.h"

/******************************************************************************
 *
 * Explicit Copy traversal ( emec_tab)
 *
 * Prefix: EMEC
 *
 *****************************************************************************/
extern node *EMECdoExplicitCopy (node *syntax_tree);

extern node *EMECassign (node *arg_node, info *arg_info);
extern node *EMECfundef (node *arg_node, info *arg_info);
extern node *EMECprf (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITCOPY_H_ */
