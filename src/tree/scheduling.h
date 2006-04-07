/*
 * $Id$
 */

#ifndef _SAC_SCHEDULING_H_
#define _SAC_SCHEDULING_H_

#include <stdio.h>
#include "LookUpTable.h"
#include "types.h"

/*****************************************************************************
 *
 * Scheduling
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
 *     TravSons,
 *   For this purpose, an abstract data type is defined which stores different
 *   kinds of schedulings requiring different parameter sets.
 *   See file scheduling.c for additional information.
 *
 *****************************************************************************/
extern sched_t *SCHmakeScheduling (char *discipline, ...);
extern sched_t *SCHmakeSchedulingByPragma (node *ap_node, int line);

extern sched_t *SCHremoveScheduling (sched_t *sched);
extern void SCHtouchScheduling (sched_t *sched, info *arg_info);
extern sched_t *SCHcopyScheduling (sched_t *sched);
extern sched_t *SCHprecompileScheduling (sched_t *sched);
extern sched_t *SCHmarkmemvalsScheduling (sched_t *sched, lut_t *lut);
extern void SCHprintScheduling (FILE *outfile, sched_t *sched);

extern void SCHcheckSuitabilityConstSeg (sched_t *sched);
extern void SCHcheckSuitabilityVarSeg (sched_t *sched);
extern void SCHcheckSuitabilityWithloop (sched_t *sched);
extern bool SCHadjustmentRequired (int dim, node *wlseg);

extern node *SCHcompileSchedulingBegin (int seg_id, node *wl_ids, sched_t *sched,
                                        node *arg_node);
extern node *SCHcompileSchedulingEnd (int seg_id, node *wl_ids, sched_t *sched,
                                      node *arg_node);
extern node *SCHcompileSchedulingInit (int seg_id, node *wl_ids, sched_t *sched,
                                       node *arg_node);

extern tasksel_t *SCHmakeTaskselByPragma (node *ap_node, int line);

extern tasksel_t *SCHremoveTasksel (tasksel_t *tasksel);
extern void SCHtouchTasksel (tasksel_t *tasksel, info *arg_info);
extern tasksel_t *SCHcopyTasksel (tasksel_t *tasksel);
extern tasksel_t *SCHprecompileTasksel (tasksel_t *tasksel);
extern tasksel_t *SCHmarkmemvalsTasksel (tasksel_t *tasksel, lut_t *lut);
extern void SCHprintTasksel (FILE *outfile, tasksel_t *tasksel);

extern node *SCHcompileSchedulingWithTaskselBegin (int seg_id, node *wl_ids,
                                                   sched_t *sched, tasksel_t *tasksel,
                                                   node *arg_node);
extern node *SCHcompileSchedulingWithTaskselEnd (int seg_id, node *wl_ids, sched_t *sched,
                                                 tasksel_t *tasksel, node *arg_node);
extern node *SCHcompileSchedulingWithTaskselInit (int seg_id, node *wl_ids,
                                                  sched_t *sched, tasksel_t *tasksel,
                                                  node *arg_node);

#endif /* _SAC_SCHEDULING_H_ */
