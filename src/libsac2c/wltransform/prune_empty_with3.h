/*
 * $Id$
 */
#ifndef _SAC_PRUNE_EMPTY_WITH3_H_
#define _SAC_PRUNE_EMPTY_WITH3_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prune empty with3 traversal ( temp_tab)
 *
 * Prefix: PEW3
 *
 *****************************************************************************/
extern node *PEW3doPruneEmptyWith3 (node *syntax_tree);

extern node *PEW3assign (node *arg_node, info *arg_info);
extern node *PEW3let (node *arg_node, info *arg_info);
extern node *PEW3id (node *arg_node, info *arg_info);
extern node *PEW3with3 (node *arg_node, info *arg_info);
extern node *PEW3range (node *arg_node, info *arg_info);

#endif /* _SAC_PRUNE_EMPTY_WITH3_H_ */
