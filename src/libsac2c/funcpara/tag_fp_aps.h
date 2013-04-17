#ifndef _SAC_TAG_FP_APS_H_
#define _SAC_TAG_FP_APS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Tag ap nodes if they call a function that contains spawns ( syn_tab)
 *
 * Prefix: TFA
 *
 *****************************************************************************/
extern node *TFAdoTagFPAps (node *arg_node);
extern node *TFAap (node *arg_node, info *arg_info);

#endif /* _SAC_TAG_FP_APS_H_ */
