/*
 * $Log$
 * Revision 1.1  2005/08/19 18:17:47  ktr
 * Initial revision
 *
 */
#ifndef _SAC_INFERNEEDCOUNTERS_H_
#define _SAC_INFERNEEDCOUNTERS_H_

#include "types.h"

/******************************************************************************
 *
 * Needcount inference traversal
 *
 * prefix: INFNC
 *
 *****************************************************************************/
extern node *INFNCdoInferNeedCounters (node *arg_node, trav_t trav);
extern node *INFNCdoInferNeedCountersOneFundef (node *arg_node, trav_t trav);

extern node *INFNCfundef (node *arg_node, info *arg_info);
extern node *INFNCblock (node *arg_node, info *arg_info);
extern node *INFNCprf (node *arg_node, info *arg_info);
extern node *INFNCavis (node *arg_node, info *arg_info);
extern node *INFNCid (node *arg_node, info *arg_info);

#endif /* _SAC_INFERNEEDCOUNTERS_H_ */
