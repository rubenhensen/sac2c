#ifndef _SAC_ENFORCE_SPECIALIZATION_H_
#define _SAC_ENFORCE_SPECIALIZATION_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Enforce Specialization traversal ( esp_tab)
 *
 * Prefix: ESP
 *
 *****************************************************************************/
extern node *ESPdoEnforceSpecialization (node *syntax_tree);

extern node *ESPmodule (node *arg_node, info *arg_info);
extern node *ESPfundef (node *arg_node, info *arg_info);

#endif /* _SAC_ENFORCE_SPECIALIZATION_H_ */
