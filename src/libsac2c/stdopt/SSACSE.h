/*
 * $Id$
 */

/*****************************************************************************
 *
 * file:   SSACSE.h
 *
 * prefix: CSE
 *
 * description:
 *
 *   This module does the Common Subexpression Elimination in the AST per
 *   function (including the special functions in order of application).
 *
 *
 *****************************************************************************/

#ifndef _SAC_CSE_H_
#define _SAC_CSE_H_

#include "types.h"

extern node *CSEdoCommonSubexpressionElimination (node *fundef);
extern node *CSEdoCommonSubexpressionEliminationModule (node *module);

extern node *CSEfundef (node *arg_node, info *arg_info);
extern node *CSEavis (node *arg_node, info *arg_info);
extern node *CSEblock (node *arg_node, info *arg_info);
extern node *CSEassign (node *arg_node, info *arg_info);
extern node *CSEcond (node *arg_node, info *arg_info);
extern node *CSEreturn (node *arg_node, info *arg_info);
extern node *CSElet (node *arg_node, info *arg_info);
extern node *CSEap (node *arg_node, info *arg_info);
extern node *CSEids (node *arg_node, info *arg_info);
extern node *CSEid (node *arg_node, info *arg_info);
extern node *CSEwith (node *arg_node, info *arg_info);
extern node *CSEcode (node *arg_node, info *arg_info);

#endif /* _SAC_CSE_H_ */
