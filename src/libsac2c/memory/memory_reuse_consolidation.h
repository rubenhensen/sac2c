#ifndef _SAC_MEMREUSECONS_H_
#define _SAC_MEMREUSECONS_H_

#include "types.h"

/******************************************************************************
 *
 * memory reuse consolidation for EMR candidates
 *
 *****************************************************************************/
extern node *MRCdoRefConsolidation (node *syntax_tree);

extern node *MRCfundef (node *syntax_tree, info *arg_info);
extern node *MRCwith (node *arg_node, info *arg_info);

#endif /* _SAC_MEMREUSECONS_H_ */
