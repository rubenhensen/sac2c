#ifndef _SAC_REMOVEWITH3_H_
#define _SAC_REMOVEWITH3_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Remove With3s
 *
 * Prefix: RW3
 *
 *****************************************************************************/
extern node *TEMPdoRemoveWith3 (node *syntax_tree);

extern node *RW3with3 (node *arg_node, info *arg_info);
extern node *RW3fundef (node *arg_node, info *arg_info);
extern node *RW3assign (node *arg_node, info *arg_info);
extern node *RW3range (node *arg_node, info *arg_info);
#endif /* _SAC_REMOVEWITH3_H_ */
