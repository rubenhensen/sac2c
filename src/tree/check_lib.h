/*
 * Revision 1.0  2004/11/22 22:26:34  jhb
 * RCS-header inserted
 *
 *
 */

#ifndef _SAC_CHECK_H_
#define _SAC_CHECK_H_

#include "types.h"

/******************************************************************************
 *
 * Check
 *
 * Prefix: CHK
 *
 *****************************************************************************/
extern node *CHKExistSon (node *arg_node, char *node, char *son);
extern node *CHKRightType (node *arg_node, char *node, char *son);

#endif /*_SAC_CHECK_H_ */
