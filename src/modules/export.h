/*
 *
 * $Log$
 * Revision 1.2  2004/11/14 15:23:04  sah
 * extended traversal to typedefs
 *
 * Revision 1.1  2004/10/17 14:51:14  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _EXPORT_H
#define _EXPORT_H

#include "types.h"

/*
 * Traversal Functions
 */

extern node *EXPUse (node *arg_node, info *arg_info);
extern node *EXPImport (node *arg_node, info *arg_info);
extern node *EXPProvide (node *arg_node, info *arg_info);
extern node *EXPExport (node *arg_node, info *arg_info);
extern node *EXPSymbol (node *arg_node, info *arg_info);
extern node *EXPFundef (node *arg_node, info *arg_info);
extern node *EXPTypedef (node *arg_node, info *arg_info);
extern node *EXPObjdef (node *arg_node, info *arg_info);
extern node *EXPModul (node *arg_node, info *arg_info);

/*
 * Start of Traversal
 */

extern void DoExport (node *syntax_tree);

#endif /* _EXPORT_H */
