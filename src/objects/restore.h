/*
 *
 * $Log$
 * Revision 1.1  2004/11/26 17:43:12  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_RESTORE_H_
#define _SAC_RESTORE_H_

#include "types.h"

/******************************************************************************
 *
 * Object restore
 *
 * Prefix: RSTO
 *
 *****************************************************************************/

extern node *RSTOdoRestoreObjects (node *syntax_tree);
extern char *RSTOobjInitFunctionName (bool before_rename);

extern node *RSTOmodule (node *arg_node, info *arg_info);
extern node *RSTOobjdef (node *arg_node, info *arg_info);
extern node *RSTOfundef (node *arg_node, info *arg_info);
extern node *RSTOarg (node *arg_node, info *arg_info);
extern node *RSTOvardec (node *arg_node, info *arg_info);
extern node *RSTOassign (node *arg_node, info *arg_info);
extern node *RSTOlet (node *arg_node, info *arg_info);
extern node *RSTOicm (node *arg_node, info *arg_info);
extern node *RSTOap (node *arg_node, info *arg_info);
extern node *RSTOprf (node *arg_node, info *arg_info);
extern node *RSTOreturn (node *arg_node, info *arg_info);
extern node *RSTOid (node *arg_node, info *arg_info);
extern node *RSTOids (node *arg_node, info *arg_info);
extern node *RSTOret (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * NODES THAT MUST NOT BE TRAVERSED
 *
 * N_icm
 *
 *****************************************************************************/

#endif /* _SAC_RESTORE_H_ */
