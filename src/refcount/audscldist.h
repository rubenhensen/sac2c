/*
 *
 * $Log$
 * Revision 1.1  2005/06/30 16:40:32  ktr
 * Initial revision
 *
 */

#ifndef _SAC_AUD_SCL_DISTINCTION_H_
#define _SAC_AUD_SCL_DISTINCTION_H_

#include "types.h"

/******************************************************************************
 *
 * AUD SCL disctinction
 *
 * Prefix: ASD
 *
 *****************************************************************************/
extern node *ASDdoTypeConv (node *syntax_tree);

extern node *ASDmodule (node *arg_node, info *arg_info);
extern node *ASDfundef (node *arg_node, info *arg_info);
extern node *ASDassign (node *arg_node, info *arg_info);
extern node *ASDap (node *arg_node, info *arg_info);
extern node *ASDcond (node *arg_node, info *arg_info);
extern node *ASDdo (node *arg_node, info *arg_info);
extern node *ASDlet (node *arg_node, info *arg_info);

#endif /* _SAC_AUD_SCL_DISTINCTION_H_ */
