/*
 *
 * $Log$
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/08/10 16:14:33  ktr
 * RIicm added.
 *
 * Revision 1.1  2004/08/10 13:30:12  ktr
 * Initial revision
 *
 */
#ifndef _SAC_REUSE_H_
#define _SAC_REUSE_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse inference traversal ( emri_tab)
 *
 * prefix: EMRI
 *
 *****************************************************************************/
extern node *EMRIdoReuseInference (node *syntax_tree);

extern node *EMRIarg (node *arg_node, info *arg_info);
extern node *EMRIassign (node *arg_node, info *arg_info);
extern node *EMRIcode (node *arg_node, info *arg_info);
extern node *EMRIcond (node *arg_node, info *arg_info);
extern node *EMRIfundef (node *arg_node, info *arg_info);
extern node *EMRIicm (node *arg_node, info *arg_info);
extern node *EMRIlet (node *arg_node, info *arg_info);
extern node *EMRIprf (node *arg_node, info *arg_info);
extern node *EMRIwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_REUSE_H_ */
