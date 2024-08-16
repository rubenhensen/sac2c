#ifndef _SAC_ANNOTATENAMESPACES_H_
#define _SAC_ANNOTATENAMESPACES_H_

#include "types.h"

/******************************************************************************
 *
 * Annotate namespaces traversal ( ans_tab)
 *
 * Prefix: ANS
 *
 *****************************************************************************/
extern node *ANSdoAnnotateNamespace (node *module);

extern node *ANSsymbol (node *arg_node, info *arg_info);
extern node *ANSuse (node *arg_node, info *arg_info);
extern node *ANSimport (node *arg_node, info *arg_info);
extern node *ANSexport (node *arg_node, info *arg_info);
extern node *ANSprovide (node *arg_node, info *arg_info);
extern node *ANSfundef (node *arg_node, info *arg_info);
extern node *ANStypedef (node *arg_node, info *arg_info);
extern node *ANSobjdef (node *arg_node, info *arg_info);
extern node *ANSspap (node *arg_node, info *arg_info);
extern node *ANSwhile (node *arg_node, info *arg_info);
extern node *ANSdo (node *arg_node, info *arg_info);
extern node *ANSavis (node *arg_node, info *arg_info);
extern node *ANSarray (node *arg_node, info *arg_info);
extern node *ANSarg (node *arg_node, info *arg_info);
extern node *ANSret (node *arg_node, info *arg_info);
extern node *ANSspids (node *arg_node, info *arg_info);
extern node *ANSspid (node *arg_node, info *arg_info);
extern node *ANSspmop (node *arg_node, info *arg_info);
extern node *ANSlet (node *arg_node, info *arg_info);
extern node *ANSwith (node *arg_node, info *arg_info);
extern node *ANSspfold (node *arg_node, info *arg_info);
extern node *ANSvardec (node *arg_node, info *arg_info);
extern node *ANScast (node *arg_node, info *arg_info);
extern node *ANSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATENAMESPACES_H_ */
