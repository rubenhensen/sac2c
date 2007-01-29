#ifndef _SAC_STATICREUSE_H_
#define _SAC_STATICREUSE_H_

#include "types.h"

/******************************************************************************
 *
 * Static reuse traversal ( emsr_tab)
 *
 * Prefix: EMSR
 *
 *****************************************************************************/
extern node *EMSRdoStaticReuse (node *syntax_tree);

extern node *EMSRprf (node *arg_node, info *arg_info);

#endif /* _SAC_STATICREUSE_H_ */
