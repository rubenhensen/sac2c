/*
 *
 * $Log$
 * Revision 3.9  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 3.8  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.7  2001/04/18 10:06:50  dkr
 * signature of InlineSingleApplication modified
 *
 * Revision 3.6  2001/04/03 14:13:02  dkr
 * INL_NAIVE removed
 *
 * Revision 3.5  2001/03/29 01:37:32  dkr
 * signature of InlineSingleApplication modified
 *
 * Revision 3.4  2001/03/27 13:47:56  dkr
 * signature of Inline() modified
 *
 * Revision 3.3  2001/03/22 13:32:47  dkr
 * CreateInlineName removed
 *
 * Revision 3.2  2001/03/21 17:49:56  dkr
 * INLvardec, INLarg removed
 *
 * Revision 3.1  2000/11/20 18:00:31  sacbase
 * new release made
 *
 * ...[eliminated].....
 *
 * Revision 1.1  1995/05/26  14:22:18  asi
 * Initial revision
 *
 */

#ifndef _SAC_INLINE_H_
#define _SAC_INLINE_H_

#include "types.h"

extern node *INLdoInline (node *arg_node);
extern node *INLinlineSingleApplication (node *let, node *fundef);

extern node *INLmodule (node *arg_node, info *arg_info);
extern node *INLfundef (node *arg_node, info *arg_info);
extern node *INLassign (node *arg_node, info *arg_info);

#endif /* _SAC_INLINE_H_ */
