#ifndef _SAC_TAG_FP_FUNDEFS_H_
#define _SAC_TAG_FP_FUNDEFS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Tag fundef nodes if they contain a spawn statement ( syn_tab)
 *
 * Prefix: TFF
 *
 *****************************************************************************/
extern node *TFFdoTagFPFundefs (node *arg_node);
extern node *TFFfundef (node *arg_node, info *arg_info);
extern node *TFFap (node *arg_node, info *arg_info);

#endif /* _SAC_TAG_FP_FUNDEFS_H_ */
