/*
 *
 * $Id$
 *
 */

#ifndef _SAC_IVE_SPLIT_SELECTIONS_H_
#define _SAC_IVE_SPLIT_SELECTIONS_H_

#include "types.h"

extern node *IVESPLITfundef (node *arg_node, info *arg_info);
extern node *IVESPLITassign (node *arg_node, info *arg_info);
extern node *IVESPLITprf (node *arg_node, info *arg_info);

extern node *IVESPLITdoSplitSelections (node *syntax_tree);
extern node *IVESPLITdoSplitSelectionsOneFundef (node *arg_node);

#endif /* _SAC_IVE_SPLIT_SELECTIONS_H_ */
