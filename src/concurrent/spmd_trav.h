/*
 *
 * $Log$
 * Revision 2.6  1999/08/27 12:46:43  jhs
 * Added Traversal for CountOccurences.
 * Commented and rearranged functions.
 *
 * Revision 2.5  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.4  1999/08/05 13:38:54  jhs
 * Added optimization of sequential assignments between spmd-blocks, main work
 * happens in spmdinit and ist steered by OPT_MTI (default now: off), some
 * traversals were needed and added in spmd_trav.
 *
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

extern int *CreateCM (int varno);
extern int *DestroyCM (int *mask);

extern int *CountOccurences (node *block, DFMmask_t which, node *fundef);
extern node *SPMDCOassign (node *arg_node, node *arg_info);

extern int LetWithFunction (node *let);
extern node *SPMDLCap (node *arg_node, node *arg_info);

extern node *DeleteNested (node *arg_node);
extern node *SPMDDNspmd (node *arg_node, node *arg_info);

extern void ProduceMasks (node *arg_node, node *spmd, node *fundef);
extern node *SPMDPMassign (node *arg_node, node *arg_info);

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDRO - SPMD - Traversal to Reduce Occurences
 **
 ******************************************************************************
 ******************************************************************************/

extern void ReduceOccurences (node *block, int *counters, DFMmask_t mask);

extern node *SPMDROblock (node *arg_node, node *arg_info);
extern node *SPMDROlet (node *arg_node, node *arg_info);
extern node *SPMDROassign (node *arg_node, node *arg_info);

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDRM - SPMD - Traversal to Reduce Masks
 **
 ******************************************************************************
 ******************************************************************************/

extern DFMmask_t ReduceMasks (node *block, DFMmask_t first_out);

extern node *SPMDRMblock (node *arg_node, node *arg_info);
extern node *SPMDRMwhile (node *arg_node, node *arg_info);
extern node *SPMDRMloop (node *arg_node, node *arg_info);
extern node *SPMDRMcond (node *arg_node, node *arg_info);
extern node *SPMDRMid (node *arg_node, node *arg_info);
extern node *SPMDRMlet (node *arg_node, node *arg_info);
extern node *SPMDRMassign (node *arg_node, node *arg_info);

#endif /* _spmd_trav_h */
