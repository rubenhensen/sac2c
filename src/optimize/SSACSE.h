/*
 * $Log$
 * Revision 1.9  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.8  2004/11/26 19:43:02  khf
 * corrected traversal functions
 *
 * Revision 1.7  2004/11/26 15:00:42  khf
 * SacDevCamp04: COMPILES!!!
 *
 * Revision 1.6  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 1.5  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.4  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.3  2001/03/23 09:29:29  nmw
 * SSACSEdo/while removed
 *
 * Revision 1.2  2001/03/07 15:58:35  nmw
 * SSA Common Subexpression Elimination implemented
 *
 * Revision 1.1  2001/03/05 16:02:25  nmw
 * Initial revision
 *
 *
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
extern node *CSEarg (node *arg_node, info *arg_info);
extern node *CSEblock (node *arg_node, info *arg_info);
extern node *CSEvardec (node *arg_node, info *arg_info);
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
