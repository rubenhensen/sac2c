
#ifndef _SAC_ELIMSUBDIV_H_
#define _SAC_ELIMSUBDIV_H_

#include "types.h"

/******************************************************************************
 *
 * Eliminate subtraction and division traversal ( esd_tab)
 *
 * Prefix: ESD
 *
 *****************************************************************************/
extern node *ESDdoElimSubDiv (node *arg_node);

extern node *ESDblock (node *, info *);
extern node *ESDassign (node *, info *);
extern node *ESDlet (node *, info *);
extern node *ESDprf (node *, info *);
extern node *ESDfundef (node *, info *);
extern node *ESDmodule (node *, info *);

#endif /* _SAC_ELIMSUBDIV_H_ */
