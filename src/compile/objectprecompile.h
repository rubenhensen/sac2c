/*
 *
 * $Log$
 * Revision 1.1  2004/11/25 15:14:46  ktr
 * Initial revision
 *
 */

#ifndef _SAC_OBJECTPRECOMPILE_H_
#define _SAC_OBJECTPRECOMPILE_H_

#include "types.h"

/******************************************************************************
 *
 * Object Precompililation
 *
 * Prefix: OPC
 *
 *****************************************************************************/
extern node *OPCdoObjectPrecompile (node *syntax_tree);
extern char *OPCobjInitFunctionName (bool before_rename);

extern node *OPCmodule (node *arg_node, info *arg_info);
extern node *OPCobjdef (node *arg_node, info *arg_info);
extern node *OPCfundef (node *arg_node, info *arg_info);
extern node *OPCarg (node *arg_node, info *arg_info);
extern node *OPCvardec (node *arg_node, info *arg_info);
extern node *OPCassign (node *arg_node, info *arg_info);
extern node *OPClet (node *arg_node, info *arg_info);
extern node *OPCicm (node *arg_node, info *arg_info);
extern node *OPCap (node *arg_node, info *arg_info);
extern node *OPCprf (node *arg_node, info *arg_info);
extern node *OPCreturn (node *arg_node, info *arg_info);
extern node *OPCid (node *arg_node, info *arg_info);
extern node *OPCids (node *arg_node, info *arg_info);
extern node *OPCret (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * NODES THAT MUST NOT BE TRAVERSED
 *
 * N_icm
 *
 *****************************************************************************/

#endif /* _SAC_OBJECTPRECOMPILE_H_ */
