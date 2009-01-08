/*
 *
 * $Log$
 * Revision 1.2  2004/11/27 02:25:29  ktr
 * new version
 *
 * Revision 1.1  2004/11/26 23:13:29  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_FUNCTIONPRECOMPILE_H_
#define _SAC_FUNCTIONPRECOMPILE_H_

#include "types.h"

/******************************************************************************
 *
 * Function Precompile
 *
 * Prefix: PREC2
 *
 *****************************************************************************/

extern node *FPCdoFunctionPrecompile (node *syntax_tree);

extern node *FPCmodule (node *arg_node, info *arg_info);
extern node *FPCfundef (node *arg_node, info *arg_info);
extern node *FPCassign (node *arg_node, info *arg_info);
extern node *FPClet (node *arg_node, info *arg_info);
extern node *FPCap (node *arg_node, info *arg_info);
extern node *FPCret (node *arg_node, info *arg_info);
extern node *FPCarg (node *arg_node, info *arg_info);

#endif /* _SAC_FUNCTIONPRECOMPITE_H_ */
