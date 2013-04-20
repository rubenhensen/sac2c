/*****************************************************************************
 *
 * file:   replicate_functions.h
 *
 * description:
 *   header file for replicate_functions.c
 *
 *****************************************************************************/

#ifndef _SAC_REPLICATE_FUNCTIONS_H_
#define _SAC_REPLICATE_FUNCTIONS_H_

#include "types.h"

extern node *REPFUNdoReplicateFunctions (node *arg_node);

extern node *REPFUNmodule (node *arg_node, info *arg_info);
extern node *REPFUNfundef (node *arg_node, info *arg_info);
extern node *REPFUNassign (node *arg_node, info *arg_info);
extern node *REPFUNap (node *arg_node, info *arg_info);
extern node *REPFUNex (node *arg_node, info *arg_info);
extern node *REPFUNst (node *arg_node, info *arg_info);
extern node *REPFUNmt (node *arg_node, info *arg_info);

#endif /* _SAC_REPLICATE_FUNCTIONS_H_ */
