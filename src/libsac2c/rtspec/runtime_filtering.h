/** <!--*******************************************************************-->
 *
 * @file runtime_filtering.h
 *
 * @brief Header file for runtime_filtering.
 *
 * @author tvd
 *
 ****************************************************************************/

#ifndef _SAC_RUNTIME_FILTERING_H_
#define _SAC_RUNTIME_FILTERING_H_

#include "types.h"

extern node *RTFILTERmodule (node *arg_node, info *arg_info);

extern node *RTFILTERfundef (node *arg_node, info *arg_info);

extern node *RTFILTERarg (node *arg_node, info *arg_info);

extern node *RTFILTERret (node *arg_node, info *arg_info);

extern node *RTdoFilter (node *syntax_tree);

#endif /* _SAC_RUNTIME_FILTERING_H_ */
