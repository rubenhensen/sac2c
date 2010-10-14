/*
 * $Id$
 */
#ifndef _SAC_IVE_SPLIT_LOOP_INVARIANTS_H_
#define _SAC_IVE_SPLIT_LOOP_INVARIANTS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Index Vector Elim. Split Loop Invariants ( ivesli_tab)
 *
 * Prefix: IVESLI
 *
 *****************************************************************************/
extern node *IVESLIdoIVESplitLoopInvariants (node *syntax_tree);

extern node *IVESLIfundef (node *arg_node, info *arg_info);
extern node *IVESLIwith (node *arg_node, info *arg_info);
extern node *IVESLIwith2 (node *arg_node, info *arg_info);
extern node *IVESLIassign (node *arg_node, info *arg_info);
extern node *IVESLIprf (node *arg_node, info *arg_info);

#endif /* _SAC_IVE_SPLIT_LOOP_INVARIANTS_H_ */
