/*
 *
 * $Log$
 * Revision 1.1  2004/11/14 13:43:42  ktr
 * Initial revision
 *
 */
#ifndef _reusebranching_h
#define _reusebranching_h

/******************************************************************************
 *
 * Reuse branching traversal
 *
 * Prefix: EMRB
 *
 *****************************************************************************/
extern node *EMRBReuseBranching (node *syntax_tree);

extern node *EMRBassign (node *arg_node, info *arg_info);
extern node *EMRBfundef (node *arg_node, info *arg_info);
extern node *EMRBprf (node *arg_node, info *arg_info);
extern node *EMRBwith (node *arg_node, info *arg_info);
extern node *EMRBwith2 (node *arg_node, info *arg_info);
extern node *EMRBwithop (node *arg_node, info *arg_info);

#endif
