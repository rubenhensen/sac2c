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

extern node *UESDmodule (node *, info *);
extern node *UESDfundef (node *, info *);
extern node *UESDblock (node *, info *);
extern node *UESDassign (node *, info *);
extern node *UESDlet (node *, info *);
extern node *UESDprf (node *, info *);

#endif /* _SAC_UNDOELIMSUBDIV_H_ */
