/*
 *
 * $Log$
 * Revision 1.1  1995/11/28 12:23:34  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_precompile_h

#define _sac_precompile_h

extern node *precompile (node *syntax_tree);

extern node *PRECmodul (node *arg_node, node *arg_info);
extern node *PRECobjdef (node *arg_node, node *arg_info);
extern node *PRECfundef (node *arg_node, node *arg_info);
extern node *PRECarg (node *arg_node, node *arg_info);
extern node *PREClet (node *arg_node, node *arg_info);
extern node *PRECap (node *arg_node, node *arg_info);
extern node *PRECreturn (node *arg_node, node *arg_info);
extern node *PRECexprs (node *arg_node, node *arg_info);
extern node *PRECid (node *arg_node, node *arg_info);

#endif /* _sac_precompile_h */
