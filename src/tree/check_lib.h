/*
 *
 * $Log$
 * Revision 1.4  2005/02/10 14:08:49  jhb
 * change the revisionslog
 *
 * Revision 1.0  2004/11/22 22:26:34  jhb
 * RCS-header inserted
 *
 *
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
extern node *CHKdoCheck (node *syntax_tree);

extern node *CHKexistSon (node *son, node *arg_node, char *string);
extern node *CHKexistAttribute (void *attribute, node *arg_node, char *string);
extern node *CHKrightType (void *attribute, node *arg_node, char *type, char *string);

#endif /*_SAC_CHECK_LIB_H_ */
