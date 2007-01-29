/*
 *
 * $Log$
 * Revision 1.4  2004/11/25 21:14:38  sah
 * COMPILES
 *
 * Revision 1.3  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.2  2004/11/14 15:23:04  sah
 * extended traversal to typedefs
 *
 * Revision 1.1  2004/10/17 14:51:14  sah
 * Initial revision
 *
 *
 *
 */

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
