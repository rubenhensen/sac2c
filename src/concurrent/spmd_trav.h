/*
 *
 * $Log$
 * Revision 2.3  1999/07/28 13:07:45  jhs
 * CountOccurences gets fundef now.
 *
 * Revision 2.2  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.1  1999/06/15 14:12:32  jhs
 * Initial revision generated.
 *
 *
 */

#ifndef _spmd_trav_h
#define _spmd_trav_h

#include "DataFlowMask.h"

extern void DestroyCM (int *mask);

extern int *CountOccurences (node *block, DFMmask_t which, node *fundef);

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDTRO - SPMD - Traversal to Reduce Occurences
 **
 ******************************************************************************
 ******************************************************************************/

extern void ReduceOccurences (node *block, int *counters, DFMmask_t mask);

extern node *SPMDTROblock (node *arg_node, node *arg_info);
extern node *SPMDTROlet (node *arg_node, node *arg_info);
extern node *SPMDTROassign (node *arg_node, node *arg_info);

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDTRM - SPMD - Traversal to Reduce Masks
 **
 ******************************************************************************
 ******************************************************************************/

extern DFMmask_t ReduceMasks (node *block, DFMmask_t first_out);

extern node *SPMDTRMblock (node *arg_node, node *arg_info);
extern node *SPMDTRMloop (node *arg_node, node *arg_info);
extern node *SPMDTRMcond (node *arg_node, node *arg_info);
extern node *SPMDTRMid (node *arg_node, node *arg_info);
extern node *SPMDTRMlet (node *arg_node, node *arg_info);
extern node *SPMDTRMassign (node *arg_node, node *arg_info);

#endif /* _spmd_trav_h */
