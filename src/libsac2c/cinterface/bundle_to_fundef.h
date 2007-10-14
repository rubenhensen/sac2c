/*
 * $Id$
 */
#ifndef _SAC_BUNDLE_TO_FUNDEF_H_
#define _SAC_BUNDLE_TO_FUNDEF_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Bundle to fundef traversal ( btf_tab)
 *
 * Prefix: BTF
 *
 *****************************************************************************/
extern node *BTFdoBundleToFundef (node *syntax_tree);

extern node *BTFfunbundle (node *arg_node, info *arg_info);
extern node *BTFfundef (node *arg_node, info *arg_info);

#endif /* _SAC_BUNDLE_TO_FUNDEF_H_ */
