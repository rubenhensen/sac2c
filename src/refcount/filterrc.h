/*
 *
 * $Log$
 * Revision 1.1  2004/10/12 10:08:05  ktr
 * Initial revision
 *
 */

#ifndef _filterrc_h
#define _filterrc_h

/******************************************************************************
 *
 * Filter reuse candidates traversal (emfrc_tab)
 *
 * prefix: EMFRC
 *
 *****************************************************************************/
extern node *EMFRCFilterReuseCandidates (node *syntax_tree);

extern node *EMFRCap (node *arg_node, info *arg_info);
extern node *EMFRCarg (node *arg_node, info *arg_info);
extern node *EMFRCassign (node *arg_node, info *arg_info);
extern node *EMFRCcond (node *arg_node, info *arg_info);
extern node *EMFRCfundef (node *arg_node, info *arg_info);
extern node *EMFRCid (node *arg_node, info *arg_info);
extern node *EMFRCprf (node *arg_node, info *arg_info);
extern node *EMFRCwith (node *arg_node, info *arg_info);
extern node *EMFRCwith2 (node *arg_node, info *arg_info);
extern node *EMFRCwithop (node *arg_node, info *arg_info);

#endif
