/*
 *
 * $Log$
 * Revision 1.3  2004/10/22 13:23:51  sah
 * working implementation for fundefs
 *
 * Revision 1.2  2004/10/22 09:02:16  sah
 * added ANSSymbol
 *
 * Revision 1.1  2004/10/22 08:50:04  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _ANNOTATENAMESPACES_H
#define _ANNOTATENAMESPACES_H

#include "types.h"

extern node *ANSSymbol (node *arg_node, info *arg_info);
extern node *ANSUse (node *arg_node, info *arg_info);
extern node *ANSImport (node *arg_node, info *arg_info);
extern node *ANSExport (node *arg_node, info *arg_info);
extern node *ANSProvide (node *arg_node, info *arg_info);
extern node *ANSFundef (node *arg_node, info *arg_info);
extern node *ANSTypedef (node *arg_node, info *arg_info);
extern node *ANSObjdef (node *arg_node, info *arg_info);
extern node *ANSAp (node *arg_node, info *arg_info);
extern node *ANSArg (node *arg_node, info *arg_info);
extern node *ANSVardec (node *arg_node, info *arg_info);
extern node *ANSModul (node *arg_node, info *arg_info);

extern void DoAnnotateNamespace (node *module);

#endif /* _ANNOTATENAMESPACES_H */
