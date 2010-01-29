#ifndef _SAC_UNDOCONDZEROCMP_H_
#define _SAC_UNDOCONDZEROCMP_H_

#include "types.h"

/******************************************************************************
 *
 * Prefix: UCZC
 *
 *****************************************************************************/
extern node *UCZCdoUndoCondZeroCmp (node *arg_node);

extern node *UCZCfundef (node *, info *);
extern node *UCZCblock (node *, info *);
extern node *UCZCprf (node *, info *);
extern node *UCZClet (node *, info *);
extern node *UCZCassign (node *, info *);

#endif /* _SAC_UNDOCONDZEROCMP_H_ */
