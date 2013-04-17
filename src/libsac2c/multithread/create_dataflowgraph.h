/*****************************************************************************
 *
 * file:   create_dataflowgraph.h
 *
 * description:
 *   header file for create_dataflowgraph.c
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_DATAFLOWGRAPH_H_
#define _SAC_CREATE_DATAFLOWGRAPH_H_

#include "types.h"

extern node *CDFGdoCreateDataflowgraph (node *arg_node);

extern node *CDFGblock (node *arg_node, info *arg_info);

extern node *CDFGassign (node *arg_node, info *arg_info);

/*
extern node *CDFGcond(node *arg_node, info *arg_info);
*/

extern node *CDFGid (node *arg_node, info *arg_info);

extern node *CDFGwithid (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_DATAFLOWGRAPH_H_ */
