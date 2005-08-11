/*
 *
 * $Log$
 * Revision 1.2  2005/08/11 07:40:56  ktr
 * funcond nodes are now handled correctly as well
 *
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

#endif /* _SAC_AUD_SCL_DISTINCTION_H_ */
