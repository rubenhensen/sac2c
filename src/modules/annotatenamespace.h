/*
 *
 * $Log$
 * Revision 1.4  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
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
extern node *ANSap (node *arg_node, info *arg_info);
extern node *ANSarg (node *arg_node, info *arg_info);
extern node *ANSvardec (node *arg_node, info *arg_info);
extern node *ANSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATENAMESPACES_H_ */
