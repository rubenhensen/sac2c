/*
 *
 * $Log$
 * Revision 1.1  2004/10/26 11:18:22  ktr
 * Initial revision
 *
 */
#ifndef _interfaceanalysis_h
#define _interfaceanalysis_h
/*****************************************************************************
 *
 * Interface analysis traversal (emia_tab)
 *
 * prefix: EMAA
 *
 ****************************************************************************/
extern node *EMIAInterfaceAnalysis (node *syntax_tree);

extern node *EMIAap (node *arg_node, info *arg_info);
extern node *EMIAarg (node *arg_node, info *arg_info);
extern node *EMIAassign (node *arg_node, info *arg_info);
extern node *EMIAblock (node *arg_node, info *arg_info);
extern node *EMIAcond (node *arg_node, info *arg_info);
extern node *EMIAfuncond (node *arg_node, info *arg_info);
extern node *EMIAfundef (node *arg_node, info *arg_info);
extern node *EMIAid (node *arg_node, info *arg_info);
extern node *EMIAlet (node *arg_node, info *arg_info);
extern node *EMIAreturn (node *arg_node, info *arg_info);
extern node *EMIAvardec (node *arg_node, info *arg_info);
extern node *EMIAwith (node *arg_node, info *arg_info);
extern node *EMIAwith2 (node *arg_node, info *arg_info);
extern node *EMIAwithop (node *arg_node, info *arg_info);

/****************************************************************************
 *
 * Nodes which MUST NOT be traversed
 *
 * - N_prf
 * - N_icm
 * - N_array
 *
 ****************************************************************************/

#endif
