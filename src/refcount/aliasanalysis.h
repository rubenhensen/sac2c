/*
 *
 * $Log$
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
extern node *EMAAreturn (node *arg_node, info *arg_info);
extern node *EMAAwith (node *arg_node, info *arg_info);
extern node *EMAAwith2 (node *arg_node, info *arg_info);
extern node *EMAAwithop (node *arg_node, info *arg_info);
extern node *EMAAvardec (node *arg_node, info *arg_info);

#endif
