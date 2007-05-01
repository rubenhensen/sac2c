/*
 * $Id$
 */
#ifndef _SAC_DEMORGAN_H_
#define _SAC_DEMORGAN_H_

#include "types.h"

/******************************************************************************
 *
 * deMorgan optimization
 *
 * prefix: DML
 *
 *****************************************************************************/
extern node *DMLdoDeMorgan (node *arg_node);

extern node *DMLfundef (node *arg_node, info *arg_info);
extern node *DMLassign (node *arg_node, info *arg_info);
extern node *DMLprf (node *arg_node, info *arg_info);

#endif /* _SAC_DEMORGAN_H_ */
