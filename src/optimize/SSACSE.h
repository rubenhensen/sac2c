/*
 * $Log$
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
 * prefix: SSACSE
 *
 * description:
 *
 *   This module does the Common Subexpression Elimination in the AST per
 *   function (including the special functions in order of application).
 *
 *
 *****************************************************************************/

#ifndef SAC_SSACSE_H

#define SAC_SSACSE_H

extern node *SSACSE (node *fundef);

extern node *SSACSEfundef (node *arg_node, node *arg_info);
extern node *SSACSEarg (node *arg_node, node *arg_info);
extern node *SSACSEblock (node *arg_node, node *arg_info);
extern node *SSACSEvardec (node *arg_node, node *arg_info);
extern node *SSACSEassign (node *arg_node, node *arg_info);
extern node *SSACSEcond (node *arg_node, node *arg_info);
extern node *SSACSEreturn (node *arg_node, node *arg_info);
extern node *SSACSElet (node *arg_node, node *arg_info);
extern node *SSACSEap (node *arg_node, node *arg_info);
extern node *SSACSEid (node *arg_node, node *arg_info);
extern node *SSACSENwith (node *arg_node, node *arg_info);
extern node *SSACSENcode (node *arg_node, node *arg_info);
extern node *SSACSEwhile (node *arg_node, node *arg_info);
extern node *SSACSEdo (node *arg_node, node *arg_info);

#endif /* SAC_SSACSE_H */
