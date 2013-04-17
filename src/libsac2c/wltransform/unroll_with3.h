#ifndef _SAC_UNROLL_WITH3_H_
#define _SAC_UNROLL_WITH3_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Unroll With3s
 *
 * Prefix: UW3
 *
 *****************************************************************************/
extern node *UW3doUnrollWith3 (node *syntax_tree);

extern node *UW3with3 (node *arg_node, info *arg_info);
extern node *UW3fundef (node *arg_node, info *arg_info);
extern node *UW3assign (node *arg_node, info *arg_info);
extern node *UW3range (node *arg_node, info *arg_info);
extern node *UW3fundef (node *arg_node, info *arg_info);
#endif /* _SAC_UNROLL_WITH3_H_ */
