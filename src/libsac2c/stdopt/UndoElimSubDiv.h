/* *
 * $Log$
 * Revision 1.6  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.5  2004/11/22 17:58:37  khf
 * more codebrushing
 *
 * Revision 1.4  2004/11/21 20:34:10  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.2  2004/07/07 15:57:05  mwe
 * former log-messages added
 *
 *
 *
 * revision 1.1    locked by: mwe;
 * date: 2003/04/26 20:58:53;  author: mwe;  state: Exp;
 * Initial revision
 */

#ifndef _SAC_UNDOELIMSUBDIV_H_
#define _SAC_UNDOELIMSUBDIV_H_

#include "types.h"

/******************************************************************************
 *
 * Undo Eliminate subtraction and division traversal ( uesd_tab)
 *
 * Prefix: UESD
 *
 *****************************************************************************/
extern node *UESDdoUndoElimSubDiv (node *arg_node);
extern node *UESDdoUndoElimSubDivModule (node *arg_node);

extern node *UESDfundef (node *, info *);
extern node *UESDblock (node *, info *);
extern node *UESDassign (node *, info *);
extern node *UESDlet (node *, info *);
extern node *UESDprf (node *, info *);

#endif /* _SAC_UNDOELIMSUBDIV_H_ */
