/*
 *
 * $Log$
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

extern void DestroyCM (int *mask);

extern int *CountOccurences (node *block, DFMmask_t which, node *fundef);

extern int LetWithFunction (node *let);
extern node *SPMDTLCap (node *arg_node, node *arg_info);

extern node *DeleteNested (node *arg_node);
extern node *SPMDDNspmd (node *arg_node, node *arg_info);

extern void ProduceMasks (node *arg_node, node *spmd, node *fundef);
extern node *SPMDPMlet (node *arg_node, node *arg_info);
extern node *SPMDPMassign (node *arg_node, node *arg_info);

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
