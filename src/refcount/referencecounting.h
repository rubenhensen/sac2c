/*
 *
 * $Log$
 * Revision 1.2  2005/09/15 17:12:13  ktr
 * removed ICM traversal
 *
 * Revision 1.1  2005/07/16 09:57:27  ktr
 * Initial revision
 *
 */
#ifndef _SAC_REFERENCECOUNTING_H_
#define _SAC_REFERENCECOUNTING_H_

#include "types.h"

/******************************************************************************
 *
 * Reference counting inference traversal (rci_tab)
 *
 * prefix: RCI
 *
 *****************************************************************************/
extern node *RCIdoReferenceCounting (node *syntax_tree);

extern node *RCIap (node *arg_node, info *arg_info);
extern node *RCIarray (node *arg_node, info *arg_info);
extern node *RCIassign (node *arg_node, info *arg_info);
extern node *RCIcode (node *arg_node, info *arg_info);
extern node *RCIcond (node *arg_node, info *arg_info);
extern node *RCIfold (node *arg_node, info *arg_info);
extern node *RCIfuncond (node *arg_node, info *arg_info);
extern node *RCIfundef (node *arg_node, info *arg_info);
extern node *RCIgenarray (node *arg_node, info *arg_info);
extern node *RCIid (node *arg_node, info *arg_info);
extern node *RCIids (node *arg_node, info *arg_info);
extern node *RCIlet (node *arg_node, info *arg_info);
extern node *RCImodarray (node *arg_node, info *arg_info);
extern node *RCIprf (node *arg_node, info *arg_info);
extern node *RCIreturn (node *arg_node, info *arg_info);
extern node *RCIwith (node *arg_node, info *arg_info);
extern node *RCIwith2 (node *arg_node, info *arg_info);
extern node *RCIwithid (node *arg_node, info *arg_info);

#endif /* _SAC_REFERENCECOUNTING_H_ */
