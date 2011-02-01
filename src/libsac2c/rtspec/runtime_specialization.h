/*
 *
 * $Id$
 *
 */

/** <!-- *******************************************************************-->
 *
 * @file runtime_specialization.h
 *
 * @brief Header file with function declarations needed for runtime
 * specialization.
 *
 * @author tvd
 *
 * ***************************************************************************/

#ifndef _SAC_RUNTIME_SPECIALIZATION_H_
#define _SAC_RUNTIME_SPECIALIZATION_H_

#include "types.h"

extern node *RTSPECmodule (node *arg_node, info *arg_info);

extern node *RTSPECfundef (node *arg_node, info *arg_info);

extern node *RTSPECap (node *arg_node, info *arg_info);

extern node *RTSPECdoCreateWrapperEntries (node *arg_node);

#endif /* _SAC_RUNTIME_SPECIALIZATION_H_ */
