/*
 *
 * $Log$
 * Revision 1.2  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.1  2004/11/14 13:43:42  ktr
 * Initial revision
 *
 */
#ifndef _SAC_REUSEBRANCHING_H_
#define _SAC_REUSEBRANCHING_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse branching traversal ( emrb_tab)
 *
 * Prefix: EMRB
 *
 *****************************************************************************/
extern node *EMRBdoReuseBranching (node *syntax_tree);

extern node *EMRBassign (node *arg_node, info *arg_info);
extern node *EMRBfundef (node *arg_node, info *arg_info);
extern node *EMRBprf (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEBRANCHING_H_ */
