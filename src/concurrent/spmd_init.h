/*
 *
 * $Log$
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:30  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:13  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_init.h
 *
 * prefix: SPMDI
 *
 * description:
 *
 *   header file for spmd_init.c
 *
 *****************************************************************************/

#ifndef SPMD_INIT_H

#define SPMD_INIT_H

#include "types.h"

extern node *SPMDIassign (node *arg_node, info *arg_info);

#endif /* SPMD_INIT_H */
