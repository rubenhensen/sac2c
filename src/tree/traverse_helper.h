/*
 *
 * $Log$
 * Revision 1.1  2004/11/23 22:21:41  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_TRAVERSE_HELPER_H_
#define _SAC_TRAVERSE_HELPER_H_

#include "types.h"

extern node *TRAVSons (node *arg_node, info *arg_info);
extern node *TRAVNone (node *arg_node, info *arg_info);
extern node *TRAVError (node *arg_node, info *arg_info);

#endif /* _SAC_TRAVERSE_HELPER_H_ */
