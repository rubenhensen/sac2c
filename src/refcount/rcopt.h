/*
 *
 * $Log$
 * Revision 1.2  2004/10/11 14:49:27  ktr
 * alloc/with/inc_rc combinations are now treated as well.
 *
 * Revision 1.1  2004/10/10 09:55:30  ktr
 * Initial revision
 *
 */
#ifndef _rcopt_h
#define _rcopt_h

/******************************************************************************
 *
 * Reference counting optimizations traversal (emrco_tab)
 *
 * prefix: EMRCO
 *
 *****************************************************************************/
extern node *EMRCORefCountOpt (node *syntax_tree);

extern node *EMRCOassign (node *arg_node, info *arg_info);
extern node *EMRCOblock (node *arg_node, info *arg_info);
extern node *EMRCOlet (node *arg_node, info *arg_info);
extern node *EMRCOprf (node *arg_node, info *arg_info);
extern node *EMRCOwithop (node *arg_node, info *arg_info);

#endif
