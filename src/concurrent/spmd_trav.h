/*
 *
 * $Log$
 * Revision 3.4  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.3  2004/09/28 16:33:12  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.2  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.1  2000/11/20 18:02:34  sacbase
 * new release made
 *
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

extern node *DeleteNested (node *arg_node);
extern node *SPMDDNspmd (node *arg_node, info *arg_info);

extern void ProduceMasks (node *arg_node, node *spmd, node *fundef);
extern node *SPMDPMassign (node *arg_node, info *arg_info);

#endif /* _spmd_trav_h */
