/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:02:31  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:15  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_lift.h
 *
 * prefix: SPMDL
 *
 * description:
 *
 *   header file for spmd_lift.c
 *
 *****************************************************************************/

#ifndef SPMD_LIFT_H

#define SPMD_LIFT_H

#include "types.h"

extern node *SPMDLspmd (node *arg_node, node *arg_info);
extern node *SPMDLid (node *arg_node, node *arg_info);
extern node *SPMDLlet (node *arg_node, node *arg_info);
extern node *SPMDLnwith2 (node *arg_node, node *arg_info);
extern node *SPMDLnwithid (node *arg_node, node *arg_info);
extern ids *SPMDLids (ids *arg_node, node *arg_info);

#endif /* SPMD_LIFT_H */
