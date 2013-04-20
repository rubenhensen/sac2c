
#ifndef _SAC_ELIMTYPECONV_H_
#define _SAC_ELIMTYPECONV_H_

#include "types.h"

/******************************************************************************
 *
 * Eliminate Type Conversions traversal
 *
 * prefix: ETC
 *
 *****************************************************************************/
extern node *ETCdoEliminateTypeConversions (node *arg_node);

extern node *ETCmodule (node *arg_node, info *arg_info);
extern node *ETCfundef (node *arg_node, info *arg_info);
extern node *ETCprf (node *arg_node, info *arg_info);

#endif /* _SAC_ELIMTYPECONV_H_ */
