/*
 *
 * $Log$
 * Revision 1.1  1998/06/18 14:37:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   schedule.h
 *
 * prefix: SCHED
 *
 * description:
 *
 *   header file for schedule.c
 *
 *****************************************************************************/

#ifndef SCHEDULE_H

#define SCHEDULE_H

#include "types.h"

extern node *SCHEDwlseg (node *arg_node, node *arg_info);
extern node *SCHEDsync (node *arg_node, node *arg_info);
extern node *SCHEDnwith2 (node *arg_node, node *arg_info);

#endif /* SCHEDULE_H */
