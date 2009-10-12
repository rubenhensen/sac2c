/*
 * $Id$
 */
#ifndef _SAC_ADDSYNCS_H_
#define _SAC_ADDSYNCS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Add Syncs traversal ( temp_tab)
 *
 * Prefix: ASS
 *
 *****************************************************************************/
extern node *ASSdoAddSyncs (node *syntax_tree);

extern node *ASSrange (node *arg_node, info *arg_info);
extern node *ASSvardec (node *arg_node, info *arg_info);
extern node *ASSwith3 (node *arg_node, info *arg_info);
#endif /* _SAC_ADDSYNCS_H_ */
