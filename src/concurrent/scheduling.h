/*
 *
 * $Log$
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

#ifndef SCHEDULING_H

#define SCHEDULING_H

#include <stdio.h>

typedef void *SCHsched_t;

extern SCHsched_t SCHMakeSchedulingByPragma (node *ap_node, int line);
extern SCHsched_t SCHMakeScheduling (char *discipline, ...);

extern SCHsched_t SCHRemoveScheduling (SCHsched_t sched);
extern SCHsched_t SCHCopyScheduling (SCHsched_t sched);
extern SCHsched_t SCHPrecompileScheduling (SCHsched_t sched);
extern void SCHPrintScheduling (FILE *outfile, SCHsched_t *sched);

extern void SCHCheckSuitabilityConstSeg (SCHsched_t *sched);
extern void SCHCheckSuitabilityVarSeg (SCHsched_t *sched);
extern void SCHCheckSuitabilitySyncblock (SCHsched_t *sched);
extern int SCHAdjustmentRequired (int dim, node *wlseg);

extern SCHsched_t *SCHMakeCompatibleSyncblockScheduling (SCHsched_t *old_sched,
                                                         SCHsched_t *new_sched);

extern node *SCHCompileSchedulingEnd (SCHsched_t sched, node *arg_node);
extern node *SCHCompileSchedulingBegin (SCHsched_t sched, node *arg_node);

#endif /* SCHEDULING_H */
