/*
 * $Id$
 */
#ifndef _SAC_TRANSFORM_GTGE_TO_LTLE_H_
#define _SAC_TRANSFORM_GTGE_TO_LTLE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Transform all gt and ge operators to lt and le operators.
 *
 * Prefix: TGTL
 *
 *****************************************************************************/
extern node *TGTLdoTransformGtgeToLtle (node *argnode);
extern node *TGTLblock (node *arg_node, info *arg_info);
extern node *TGTLassign (node *arg_node, info *arg_info);
extern node *TGTLlet (node *arg_node, info *arg_info);
extern node *TGTLprf (node *arg_node, info *arg_info);
extern node *TGTLfundef (node *arg_node, info *arg_info);
extern node *TGTLmodule (node *arg_node, info *arg_info);

#endif /* _SAC_TRANSFORM_GTGE_TO_LTLE_H_ */
