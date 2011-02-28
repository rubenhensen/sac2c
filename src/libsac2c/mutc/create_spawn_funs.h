/*
 * $Id$
 */
#ifndef _SAC_CREATE_SPAWN_FUNS_H_
#define _SAC_CREATE_SPAWN_FUNS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Create Spawn Functions traversal ( cspf_tab )
 *
 * Prefix: CSPF
 *
 *****************************************************************************/
extern node *CSPFdoCreateSpawnFunctions (node *syntax_tree);

extern node *CSPFmodule (node *arg_node, info *arg_info);
extern node *CSPFap (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_SPAWN_FUNS_H_ */
