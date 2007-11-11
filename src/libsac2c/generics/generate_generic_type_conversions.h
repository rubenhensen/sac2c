/*
 * $Id$
 */
#ifndef _SAC_GENERATE_GENERIC_TYPE_CONVERSIONS_H_
#define _SAC_GENERATE_GENERIC_TYPE_CONVERSIONS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( ggtc_tab)
 *
 * Prefix: GGTC
 *
 *****************************************************************************/
extern node *GGTCdoGenerateGenericTypeConversions (node *syntax_tree);

extern node *GGTCtypedef (node *arg_node, info *arg_info);
extern node *GGTCexport (node *arg_node, info *arg_info);
extern node *GGTCprovide (node *arg_node, info *arg_info);
extern node *GGTCsymbol (node *arg_node, info *arg_info);
extern node *GGTCmodule (node *arg_node, info *arg_info);

#endif /* _SAC_GENERATE_GENERIC_TYPE_CONVERSIONS_H_ */
