/*
 * $Id: trav_template.h 15657 2007-11-13 13:57:30Z cg $
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
