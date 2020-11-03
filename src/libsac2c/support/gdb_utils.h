#ifndef _GDB_UTILS_H_
#define _GDB_UTILS_H_

#include <stdbool.h>
#include "tree_basic.h"

extern bool GDBbreakAtNid (node *arg_node, char *nm);
extern void GDBwhatIs (char *nm, node *fundef);
extern void GDBwhatAre (char *nm, node *fundef);
extern void GDBwhatIsNid (node *arg_node, node *fundef);
extern void GDBwhatAreNid (node *arg_node, node *fundef);
extern void GDBprintPrfArgs (node *arg_node, node *fundef);
extern void GDBprintAvisName (node *avis);
extern void GDBprintAvisForFundef (node *fundef);
extern void GDBprintFundefChain (node *fundef);

#endif /*_GDB_UTILS_H_ */
