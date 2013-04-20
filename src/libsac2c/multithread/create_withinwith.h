/*****************************************************************************
 *
 * file:   create_withinwith.h
 *
 * description:
 *   header file for create_withinwith.c
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_WITHINWITH_H_
#define _SAC_CREATE_WITHINWITH_H_

#include "types.h"
extern node *CRWIWdoCreateWithinwith (node *arg_node);

extern node *CRWIWfundef (node *arg_node, info *arg_info);

extern node *CRWIWassign (node *arg_node, info *arg_info);

extern node *CRWIWap (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WITHINWITH_H_ */
