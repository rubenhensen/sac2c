/*
 * $Id$
 */
#ifndef _SAC_TRAVMARKSUBALLOC_H_
#define _SAC_TRAVMARKSUBALLOC_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Mark SubAlloc traversal
 *
 * Prefix: MSA
 *
 *****************************************************************************/
extern node *MSAdoMarkSubAlloc (node *syntax_tree);

extern node *MSAlet (node *arg_node, info *arg_info);
extern node *MSAprf (node *arg_node, info *arg_info);
extern node *MSAids (node *arg_node, info *arg_info);

#endif /* _SAC_TRAVMARKSUBALLOC_H_ */
