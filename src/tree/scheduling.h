/*
 *
 * $Log$
 * Revision 3.14  2004/11/27 00:21:34  ktr
 * JUHUUUUU!!!!!!!!!!!!!!!!!!
 *
 * Revision 3.13  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 3.12  2004/08/28 14:31:03  sah
 * modified definition of SCHsched_t in
 * NEW_AST mode
 *
 * Revision 3.11  2004/08/02 14:07:58  sah
 * added lots of ugly defines to remove
 * compiler warning when using the old
 * ast;)
 *
 * Revision 3.10  2004/08/01 16:28:53  sah
 * all abstract types are now abstract structures
 * instead of void pointers
 *
 * Revision 3.9  2004/08/01 13:17:08  ktr
 * Added SCHMMVScheduling, SCHMMVTasksel
 *
 * Revision 3.8  2002/07/15 14:44:31  dkr
 * function signatures modified
 *
 * Revision 3.7  2001/06/27 14:36:22  ben
 * last routines for tasksel-pragma implemented
 *
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
