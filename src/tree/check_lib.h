/*
 * $Id$
 */

#ifndef _SAC_CHECK_LIB_H_
#define _SAC_CHECK_LIB_H_

#include "types.h"

/******************************************************************************
 *
 * Check
 *
 * Prefix: CHK
 *
 *****************************************************************************/
extern node *CHKdoTreeCheck (node *syntax_tree);

extern node *CHKexistSon (node *son, node *arg_node, char *string);
extern node *CHKexistAttribute (void *attribute, node *arg_node, char *string);
extern node *CHKnotExistAttribute (void *attribute, node *arg_node, char *string);
extern node *CHKcorrectTypeInsertError (node *arg_node, char *string);

extern node *CHKassignAvisSSAAssign (node *arg_node);

#endif /*_SAC_CHECK_LIB_H_ */
