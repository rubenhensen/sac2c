/*
 *
 * $Log$
 * Revision 1.2  2004/11/09 19:39:37  ktr
 * ongoing implementation
 *
 * Revision 1.1  2004/11/02 14:27:11  ktr
 * Initial revision
 *
 */
#ifndef _datareuse_h
#define _datareuse_h

/******************************************************************************
 *
 * Data reuse traversal
 *
 * Prefix: EMDR
 *
 *****************************************************************************/
extern node *EMDRDataReuse (node *syntax_tree);

extern node *EMDRap (node *arg_node, info *arg_info);
extern node *EMDRassign (node *arg_node, info *arg_info);
extern node *EMDRcode (node *arg_node, info *arg_info);
extern node *EMDRid (node *arg_node, info *arg_info);
extern node *EMDRlet (node *arg_node, info *arg_info);
extern node *EMDRfundef (node *arg_node, info *arg_info);
extern node *EMDRprf (node *arg_node, info *arg_info);

#endif
