/*****************************************************************************
 *
 * file:   scheduling.h
 *
 * prefix: SCH
 *
 * description:
 *
 *   Scheduling is needed nor the non-sequential execution of with-loops.
 *   In this context, it means a mechanism which guarantees that each array
 *   element selected by a generator is calculated by exactly one thread.
 *   In other words, the scheduling defines a partitioning of the iteration
 *   space onto the threads available.
 *
 *   For this purpose, an abstract data type is defined which stores different
 *   kinds of schedulings requiring different parameter sets.
 *   See file scheduling.c for additional information.
 *
 *****************************************************************************/

#ifndef SCHEDULING_H

#define SCHEDULING_H

typedef void *SCHsched_t;

extern sched_t *SCHMakeSchedulingByPragma (node *ap_node, int line);
extern sched_t *SCHRemoveScheduling (sched_t *sched);
extern sched_t *SCHPrecompileScheduling (sched_t *sched);
extern node *SCHCompileSchedulingEnd (sched_t *sched, node *arg_node);
extern node *SCHCompileSchedulingBegin (sched_t *sched, node *arg_node);

extern node *GenerateSchedulings (node *syntax_tree);
extern node *SCHmodul (node *arg_node, node *arg_info);
extern node *SCHfundef (node *arg_node, node *arg_info);
extern node *SCHwlseg (node *arg_node, node *arg_info);
extern node *SCHwlsegvar (node *arg_node, node *arg_info);
extern node *SCHsync (node *arg_node, node *arg_info);

#endif /* SCHEDULING_H */
