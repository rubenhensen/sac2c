/* $Id$ */

#ifndef _SAC_OBJECT_ANALYSIS_H_
#define _SAC_OBJECT_ANALYSIS_H_

#include "types.h"

/******************************************************************************
 *
 * Object Analysis traversal ( oan_tab)
 *
 * Prefix: OAN
 *
 *****************************************************************************/
extern node *OANdoObjectAnalysis (node *syntax_tree);

extern node *OANmodule (node *arg_node, info *arg_info);
extern node *OANglobobj (node *arg_node, info *arg_info);
extern node *OANap (node *arg_node, info *arg_info);
extern node *OANfundef (node *arg_node, info *arg_info);
extern node *OANobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_OBJECT_ANALYSIS_H_ */
