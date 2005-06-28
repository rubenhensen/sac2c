/*
 *
 * $Log$
 * Revision 1.1  2005/06/28 20:53:17  cg
 * Initial revision
 *
 */

#ifndef _SAC_HANDLE_CONDEXPR_H_
#define _SAC_HANDLE_CONDEXPR_H_

#include "types.h"

/******************************************************************************
 *
 * Handle conditional expressions traversal ( hce_tab)
 *
 * Prefix: HCE
 *
 *****************************************************************************/
extern node *HCEdoHandleConditionalExpressions (node *arg_node);

extern node *HCEassign (node *arg_node, info *arg_info);
extern node *HCEcond (node *arg_node, info *arg_info);
extern node *HCEfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_HANDLE_CONDEXPR_H_ */
