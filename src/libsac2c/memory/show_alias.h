#ifndef _SAC_SHOW_ALIAS_H_
#define _SAC_SHOW_ALIAS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Show alias information
 *
 * prefix: SHAL
 *
 *****************************************************************************/

extern node *SHALprintPreFun (node *arg_node, info *arg_info);
extern node *SHALactivate (node *syntax_tree);
extern node *SHALdeactivate (node *syntax_tree);

#endif /* _SAC_SHOW_ALIAS_H_ */
