/*
 *
 * $Log$
 * Revision 3.2  2004/11/21 23:01:01  ktr
 * ISMOP 2004!!!!!!!
 *
 * Revision 3.1  2000/11/20 18:02:27  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:08  sacbase
 * new release made
 *
 * Revision 1.2  1998/08/07 14:40:12  dkr
 * SCHEDwlsegVar added
 *
 * Revision 1.1  1998/06/18 14:37:17  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_SCHEDULE_H_
#define _SAC_SCHEDULE_H_

#include "types.h"

/*****************************************************************************
 *
 * Schedule travesal (sched_tab)
 *
 * prefix: SCHED
 *
 *****************************************************************************/
extern node *SCHEDwlseg (node *arg_node, node *arg_info);
extern node *SCHEDwlsegvar (node *arg_node, node *arg_info);
extern node *SCHEDsync (node *arg_node, node *arg_info);
extern node *SCHEDwith2 (node *arg_node, node *arg_info);

#endif /* _SAC_SCHEDULE_H_ */
