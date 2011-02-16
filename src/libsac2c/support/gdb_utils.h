/*
 *
 * $Id$
 *
 */

#ifndef _GDB_UTILS_H_
#define _GDB_UTILS_H_

extern bool GDBbreakAtNid (node *arg_node, char *nm);
extern void GDBwhatIs (char *nm, node *fundef);
extern void GDBprintPrfArgs (node *arg_node, node *fundef);
extern void GDBprintAvisName (node *avis);

#endif /*_GDB_UTILS_H_ */
