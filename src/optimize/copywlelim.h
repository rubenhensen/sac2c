/*
 * $Id$
 */
#ifndef _SAC_CWLE_TRAV_H_
#define _SAC_CWLE_TRAV_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: CWLE
 *
 *****************************************************************************/
extern node *CWLEdoTemplateTraversal (node *syntax_tree);

extern node *CWLEfundef (node *arg_node, info *arg_info);
extern node *CWLEwith (node *arg_node, info *arg_info);
extern node *CWLElet (node *arg_node, info *arg_info);
extern node *CWLEcode (node *arg_node, info *arg_info);

#endif /* _SAC_CWLE_TARV_H_ */
