/*
 * $Log$
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

extern node *SSACSE (node *fundef, node *modul);

extern node *SSACSEfundef (node *arg_node, info *arg_info);
extern node *SSACSEarg (node *arg_node, info *arg_info);
extern node *SSACSEblock (node *arg_node, info *arg_info);
extern node *SSACSEvardec (node *arg_node, info *arg_info);
extern node *SSACSEassign (node *arg_node, info *arg_info);
extern node *SSACSEcond (node *arg_node, info *arg_info);
extern node *SSACSEreturn (node *arg_node, info *arg_info);
extern node *SSACSElet (node *arg_node, info *arg_info);
extern node *SSACSEap (node *arg_node, info *arg_info);
extern node *SSACSEid (node *arg_node, info *arg_info);
extern node *SSACSENwith (node *arg_node, info *arg_info);
extern node *SSACSENcode (node *arg_node, info *arg_info);

#endif /* SAC_SSACSE_H */
