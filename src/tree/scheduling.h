/*
 *
 * $Log$
 * Revision 3.6  2001/06/13 13:06:09  ben
 * SCHMakeTaskselByPragma, SCHRemoveTasksel, SCHCopyTasksel,
 * SCHPrecompileTasksel, SCHPrintTasksel added
 *
 * Revision 3.5  2001/05/09 15:13:00  cg
 * All scheduling ICMs get an additional first parameter,
 * i.e. the segment ID. This is required to identify the appropriate
 * set of scheduler internal variables.
 *
 * Revision 3.4  2001/03/29 14:13:38  dkr
 * SCHMakeCompatibleSyncblockScheduling moved
 *
 * Revision 3.3  2001/03/14 14:11:13  ben
 * return value of SCHAdjustmentRequired is bool
 *
 * Revision 3.2  2001/01/24 23:40:58  dkr
 * some signatures modified
 *
 * Revision 3.1  2000/11/20 18:03:29  sacbase
 * new release made
 *
 * Revision 1.2  2000/08/16 13:43:59  dkr
 * include-guard prefixed with 'SAC_'
 *
 * Revision 1.1  2000/01/24 10:53:36  jhs
 * Initial revision
 *
 * Revision 2.2  1999/09/01 17:14:23  jhs
 * Remove SYNC_SCHEDULING.
 *
 * Revision 2.1  1999/02/23 12:44:11  sacbase
 * new release made
 *
 * Revision 1.3  1998/08/03 10:51:42  cg
 * added function SCHAdjustmentRequired that decides for a particular
 * dimension and a given scheduling whether the scheduling will produce
 * a legal scheduling or an adjustment has to be done.
 *
 * Revision 1.2  1998/06/18 14:19:41  cg
 * file now only contains implementation of abstract data type
 * for the representation of schedulings
 *
 */

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
 *     TravSons,
 *   For this purpose, an abstract data type is defined which stores different
 *   kinds of schedulings requiring different parameter sets.
 *   See file scheduling.c for additional information.
 *
 *****************************************************************************/

#ifndef _SAC_SCHEDULING_H_
#define _SAC_SCHEDULING_H_

#include <stdio.h>

typedef void *SCHsched_t;

extern SCHsched_t SCHMakeScheduling (char *discipline, ...);
extern SCHsched_t SCHMakeSchedulingByPragma (node *ap_node, int line);

extern SCHsched_t SCHRemoveScheduling (SCHsched_t sched);
extern SCHsched_t SCHCopyScheduling (SCHsched_t sched);
extern SCHsched_t SCHPrecompileScheduling (SCHsched_t sched);
extern void SCHPrintScheduling (FILE *outfile, SCHsched_t *sched);

extern void SCHCheckSuitabilityConstSeg (SCHsched_t *sched);
extern void SCHCheckSuitabilityVarSeg (SCHsched_t *sched);
extern void SCHCheckSuitabilityWithloop (SCHsched_t *sched);
extern bool SCHAdjustmentRequired (int dim, node *wlseg);

extern node *SCHCompileSchedulingBegin (int seg_id, char *wl_name, SCHsched_t sched,
                                        node *arg_node);
extern node *SCHCompileSchedulingEnd (int seg_id, char *wl_name, SCHsched_t sched,
                                      node *arg_node);
extern node *SCHCompileSchedulingInit (int seg_id, char *wl_name, SCHsched_t sched,
                                       node *arg_node);

typedef void *SCHtasksel_t;

extern SCHtasksel_t SCHMakeTaskselByPragma (node *ap_node, int line);

extern SCHtasksel_t SCHRemoveTasksel (SCHtasksel_t tasksel);
extern SCHtasksel_t SCHCopyTasksel (SCHtasksel_t tasksel);
extern SCHtasksel_t SCHPrecompileTasksel (SCHtasksel_t tasksel);
extern void SCHPrintTasksel (FILE *outfile, SCHtasksel_t *tasksel);

#endif /* _SAC_SCHEDULING_H_ */
