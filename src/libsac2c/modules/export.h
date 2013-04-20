#ifndef _SAC_EXPORT_H_
#define _SAC_EXPORT_H_

#include "types.h"

/******************************************************************************
 *
 * Export traversal ( exp_tab)
 *
 * Prefix: EXP
 *
 *****************************************************************************/

/*
 * Start of Traversal
 */

extern node *EXPdoExport (node *syntax_tree);

/*
 * Traversal Functions
 */

extern node *EXPuse (node *arg_node, info *arg_info);
extern node *EXPimport (node *arg_node, info *arg_info);
extern node *EXPprovide (node *arg_node, info *arg_info);
extern node *EXPexport (node *arg_node, info *arg_info);
extern node *EXPsymbol (node *arg_node, info *arg_info);
extern node *EXPfundef (node *arg_node, info *arg_info);
extern node *EXPtypedef (node *arg_node, info *arg_info);
extern node *EXPobjdef (node *arg_node, info *arg_info);
extern node *EXPmodule (node *arg_node, info *arg_info);

#endif /* _SAC_EXPORT_H_ */
