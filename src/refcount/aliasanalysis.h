/*
 *
 * $Log$
 * Revision 1.3  2004/11/02 14:31:11  ktr
 * Aliasanalysis is now performed seperately for each branch of a
 * conditional.
 *
 * Revision 1.2  2004/10/26 11:19:38  ktr
 * Intermediate update for stephan
 *
 * Revision 1.1  2004/10/15 09:05:14  ktr
 * Initial revision
 *
 */
#ifndef _aliasanalysis_h
#define _aliasanalysis_h

/******************************************************************************
 *
 * Alias analysis traversal (emaa_tab
 *
 * prefix: EMAA
 *
 *****************************************************************************/
extern node *EMAAAliasAnalysis (node *syntax_tree);

extern node *EMAAap (node *arg_node, info *arg_info);
extern node *EMAAarg (node *arg_node, info *arg_info);
extern node *EMAAassign (node *arg_node, info *arg_info);
extern node *EMAAcode (node *arg_node, info *arg_info);
extern node *EMAAcond (node *arg_node, info *arg_info);
extern node *EMAAfuncond (node *arg_node, info *arg_info);
extern node *EMAAfundef (node *arg_node, info *arg_info);
extern node *EMAAid (node *arg_node, info *arg_info);
extern node *EMAAlet (node *arg_node, info *arg_info);
extern node *EMAAprf (node *arg_node, info *arg_info);
extern node *EMAAwith (node *arg_node, info *arg_info);
extern node *EMAAwith2 (node *arg_node, info *arg_info);
extern node *EMAAwithop (node *arg_node, info *arg_info);
extern node *EMAAvardec (node *arg_node, info *arg_info);

/****************************************************************************
 *
 * Nodes which MUST NOT be traversed
 *
 * - N_icm
 * - N_return
 * - N_array
 * - N_objdef
 *
 ****************************************************************************/

#endif
