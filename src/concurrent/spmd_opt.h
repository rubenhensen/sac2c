/*
 *
 * $Log$
 * Revision 2.4  1999/07/07 15:56:45  jhs
 * Declared MeltBlocks(OnnCopies) as external
 * (should be moved to another, more global place later).
 *
 * Revision 2.3  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.2  1999/05/28 15:31:45  jhs
 * Implemented first steps of spmd-optimisation.
 *
 * Revision 2.1  1999/02/23 12:44:18  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_opt.h
 *
 * prefix: SPMDO
 *
 * description:
 *
 *   header file for spmd_opt.c
 *
 *****************************************************************************/

#ifndef SPMD_OPT_H

#define SPMD_OPT_H

#include "types.h"

extern node *SPMDOspmd (node *arg_node, node *arg_info);
extern node *SPMDOassign (node *arg_node, node *arg_info);
extern node *SPMDOfundef (node *arg_node, node *arg_info);

extern node *MeltBlocks (node *first_block, node *second_block);
extern node *MeltBlocksOnCopies (node *first_block, node *second_block);

#endif /* SPMD_OPT_H */
