/*
 * $Id$
 */

#ifndef _SAC_RENAMEIDENTIFIERS_H_
#define _SAC_RENAMEIDENTIFIERS_H_

#include "types.h"

extern node *RIDdoRenameIdentifiers (node *arg_node);

extern node *RIDmodule (node *arg_node, info *arg_info);
extern node *RIDtypedef (node *arg_node, info *arg_info);
extern node *RIDobjdef (node *arg_node, info *arg_info);
extern node *RIDfundef (node *arg_node, info *arg_info);
extern node *RIDarg (node *arg_node, info *arg_info);
extern node *RIDreturn (node *arg_node, info *arg_info);
extern node *RIDap (node *arg_node, info *arg_info);
extern node *RIDicm (node *arg_node, info *arg_info);
extern node *RIDwlseg (node *arg_node, info *arg_info);
extern node *RIDwlsegvar (node *arg_node, info *arg_info);
extern node *RIDavis (node *arg_node, info *arg_info);
// extern node *RIDprf(node * arg_node, info * arg_info);

extern char *RIDrenameLocalIdentifier (char *id);

#endif /* _SAC_RENAMEIDENTIFIERS_H_ */
