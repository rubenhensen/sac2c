/** <!-- *******************************************************************-->
 *
 * @file runtime_function_id.h
 *
 * @brief Header file with function declarations needed for assigning
 * unique function ids to generic functions.
 *
 * @author hmw
 *
 * ***************************************************************************/

#ifndef _SAC_RUNTIME_FUNCTION_ID_H_
#define _SAC_RUNTIME_FUNCTION_ID_H_

#include "types.h"

extern node *UIDmodule (node *arg_node, info *arg_info);

extern node *UIDfundef (node *arg_node, info *arg_info);

extern node *UIDarg (node *arg_node, info *arg_info);

extern node *UIDdoSetFunctionIDs (node *arg_node);

#endif /* _SAC_RUNTIME_FUNCTION_ID_H_ */
