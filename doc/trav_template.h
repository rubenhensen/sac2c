/*
 * $Id$
 */
#ifndef _SAC_TRAVTEMPLATE_H_
#define _SAC_TRAVTEMPLATE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: TEMP
 *
 *****************************************************************************/
extern node *TEMPdoTemplateTraversal (node *syntax_tree);

extern node *TEMPfundef (node *arg_node, info *arg_info);

#endif /* _SAC_TRAVTEMPLATE_H_ */
