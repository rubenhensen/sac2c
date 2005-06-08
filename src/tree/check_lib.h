/*
 *
 * $Log$
 * Revision 1.8  2005/06/08 13:37:08  jhb
 * attribute are now check correctly
 *
 * Revision 1.7  2005/05/19 13:34:44  jhb
 * begin to add the ranges for the attributes
 *
 * Revision 1.6  2005/02/14 14:08:48  jhb
 * change name
 *
 * Revision 1.5  2005/02/11 14:49:34  jhb
 * change CHKdoCheck in CHKdoTreeCheck
 *
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
extern node *CHKdoTreeCheck (node *syntax_tree);

extern node *CHKexistSon (node *son, node *arg_node, char *string);
extern node *CHKexistAttribute (void *attribute, node *arg_node, char *string);
extern node *CHKnotExistAttribute (void *attribute, node *arg_node, char *string);
extern node *CHKcorrectType (void *attribute, node *arg_node, char *type, char *string);

#endif /*_SAC_CHECK_LIB_H_ */
