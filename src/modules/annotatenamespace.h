/*
 *
 * $Log$
 * Revision 1.1  2004/10/22 08:50:04  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _ANNOTATENAMESPACES_H
#define _ANNOTATENAMESPACES_H

#include "types.h"

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

#endif /* _ANNOTATENAMESPACES_H */
