/*
 *
 * $Log$
 * Revision 1.2  1999/07/28 13:04:45  jhs
 * Finished implementation of all needed routines.
 *
 * Revision 1.1  1999/07/26 11:35:24  jhs
 * Initial revision
 *
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_cons.h
 *
 * prefix: SPMDC
 *
 * description:
 *
 *   header file for spmd_cons.c
 *
 *****************************************************************************/

#ifndef SPMD_CONS_H

#define SPMD_CONS_H

extern node *SPMDCspmd (node *arg_node, node *arg_info);
extern node *SPMDCsync (node *arg_node, node *arg_info);

#endif /* SPMD_CONS_H */
