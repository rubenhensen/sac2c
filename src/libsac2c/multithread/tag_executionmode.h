/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef _SAC_TAG_EXECUTIONMODE_H_
#define _SAC_TAG_EXECUTIONMODE_H_

#include "types.h"

extern node *TEMdoTagExecutionmode (node *arg_node);

extern node *TEMassign (node *arg_node, info *arg_info);
extern node *TEMwith2 (node *arg_node, info *arg_info);
extern node *TEMprf (node *arg_node, info *arg_info);
extern node *TEMlet (node *arg_node, info *arg_info);
extern node *TEMap (node *arg_node, info *arg_info);
extern node *TEMarray (node *arg_node, info *arg_info);

#endif /* _SAC_TAG_EXECUTIONMODE_H_ */
