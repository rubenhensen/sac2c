/*
 * $Id$
 */

#ifndef _SAC_STATISTICS_H_
#define _SAC_STATISTICS_H_

#include "types.h"

extern void STATclearCounters (optimize_counter_t *oc);

extern bool STATdidSomething (optimize_counter_t *oc);

extern void STATcopyCounters (optimize_counter_t *copy, optimize_counter_t *orig);

extern void STATaddCounters (optimize_counter_t *oc, optimize_counter_t *add);

extern void STATprint (optimize_counter_t *oc);

extern node *STATdoPrintStatistics (node *syntax_tree);

#endif /* _SAC_STATISTICS_H_ */
