#ifndef _SAC_TRAVTEMPLATE_H_
#define _SAC_TRAVTEMPLATE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Tag functions as threads traversal ( temp_tab)
 *
 * Prefix: TFT
 *
 *****************************************************************************/
extern node *TFTdoTagFunctionsThreads (node *syntax_tree);

extern node *TFTfundef (node *arg_node, info *arg_info);
extern node *TFTap (node *arg_node, info *arg_info);
extern node *TFTwith3 (node *arg_node, info *arg_info);

#endif /* _SAC_TRAVTEMPLATE_H_ */
