/*
 *
 * $Log$
 * Revision 1.1  2004/09/18 15:59:41  ktr
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_emm.h
 *
 * prefix: SPMDEMM
 *
 * description:
 *
 *   header file for spmd_emm.c
 *
 *****************************************************************************/

#ifndef SPMD_EMM_H

#define SPMD_EMM_H

extern node *SpmdEmm (node *arg_node);

extern node *SPMDEMMassign (node *arg_node, info *arg_info);
extern node *SPMDEMMlet (node *arg_node, info *arg_info);
extern node *SPMDEMMprf (node *arg_node, info *arg_info);
extern node *SPMDEMMspmd (node *arg_node, info *arg_info);

#endif /* SPMD_INIT_H */
