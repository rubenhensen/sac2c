/*
 *
 * $Log$
 * Revision 1.3  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.2  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2003/09/17 12:36:40  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_TYPE_STATISTICS_H_
#define _SAC_TYPE_STATISTICS_H_

#include "types.h"

extern node *TSdoPrintTypeStatistics (node *arg_node);
extern node *TSfundef (node *arg_node, info *arg_info);
extern node *TSarg (node *arg_node, info *arg_info);
extern node *TSvardec (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_STATISTICS_H_ */
