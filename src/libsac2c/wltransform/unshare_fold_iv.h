#ifndef _SAC_UNSHARE_FOLD_IV_H_
#define _SAC_UNSHARE_FOLD_IV_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Unshare WL-fold index vectors
 *
 * Prefix: UFIV
 *
 *****************************************************************************/
extern node *UFIVdoUnshareFoldIV (node *syntax_tree);

extern node *UFIVfundef (node *arg_node, info *arg_info);
extern node *UFIVmodule (node *arg_node, info *arg_info);
extern node *UFIVcode (node *arg_node, info *arg_info);
extern node *UFIVpart (node *arg_node, info *arg_info);
extern node *UFIVwith (node *arg_node, info *arg_info);
extern node *UFIVwith2 (node *arg_node, info *arg_info);
extern node *UFIVexprs (node *arg_node, info *arg_info);

#endif /* _SAC_UNSHARE_FOLD_IV_H_ */
