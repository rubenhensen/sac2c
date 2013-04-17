#ifndef _SAC_CREATE_FUNCTION_PAIRS_H_
#define _SAC_CREATE_FUNCTION_PAIRS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Create Function Paris ( cfp_tab)
 *
 * Prefix: CFP
 *
 *****************************************************************************/
extern node *CFPdoCreateFunctionPairs (node *syntax_tree);

extern node *CFPfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_FUNCTION_PAIRS_H_ */
