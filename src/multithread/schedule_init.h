/*
 *
 * $Log$
 * Revision 1.2  2000/01/24 18:24:21  jhs
 * Added some infrastructure ...
 *
 * Revision 1.1  2000/01/24 10:27:35  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   schedule_init.h
 *
 * prefix:
 *
 * description:
 *
 *   header file for schedule_init.c
 *
 *****************************************************************************/

#ifndef SCHEDULE_INIT_H

#define SCHEDULE_INIT_H

extern node *ScheduleInit (node *arg_node, node *arg_info);

extern node *SCHINassign (node *arg_node, node *arg_info);
extern node *SCHINnwith2 (node *arg_node, node *arg_info);
extern node *SCHINwlseg (node *arg_node, node *arg_info);
extern node *SCHINwlsegVar (node *arg_node, node *arg_info);

#endif /* SCHEDULE_INIT_H */
