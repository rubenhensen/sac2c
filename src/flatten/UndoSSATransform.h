/*
 * $Log$
 * Revision 1.6  2005/01/08 09:52:35  ktr
 * Fixed some issues related to loops.
 *
 * Revision 1.5  2004/11/25 12:50:53  khf
 * removed USSATid, USSATcode
 *
 * Revision 1.4  2004/11/24 19:16:24  mwe
 * changing function names according to naming conventions
 *
 * Revision 1.3  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.2  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2004/01/28 16:56:51  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.3  2001/03/19 14:23:32  nmw
 * removal of ssa phi copy assignments added
 *
 * Revision 1.2  2001/03/12 13:41:53  nmw
 * UndoSSA creates unique result variables in multigenerator fold-withloops.
 *
 * Revision 1.1  2001/02/22 13:14:06  nmw
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_UNDOSSATRANSFORM_H_
#define _SAC_UNDOSSATRANSFORM_H_

#include "types.h"

/*****************************************************************************
 *
 * Undo SSA-Transform traversal ( ussa_tab)
 *
 * prefix: USSAT
 *
 *
 *****************************************************************************/
extern node *USSATdoUndoSsaTransform (node *syntax_tree);

extern node *USSATarg (node *arg_node, info *arg_info);
extern node *USSATvardec (node *arg_node, info *arg_info);
extern node *USSATid (node *arg_node, info *arg_info);
extern node *USSATids (node *arg_node, info *arg_info);
extern node *USSATlet (node *arg_node, info *arg_info);
extern node *USSATassign (node *arg_node, info *arg_info);
extern node *USSATfundef (node *arg_node, info *arg_info);
extern node *USSATblock (node *arg_node, info *arg_info);
extern node *USSATwith (node *arg_node, info *arg_info);

#endif /* _SAC_UNDOSSATRANSFORM_H_ */
