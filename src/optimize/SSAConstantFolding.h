/*
 * $Log$
 * Revision 1.2  2001/03/29 16:31:21  nmw
 * Constant Folding for Loops and Conditionals implemented
 *
 * Revision 1.1  2001/03/20 16:16:54  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSAConstantFolding.h
 *
 * prefix: SSACF
 *
 * description:
 *   this module does constant folding on code in ssa form.
 *
 *
 *****************************************************************************/

#ifndef SAC_SSACONSTANTFOLDING_H

#define SAC_SSACONSTANTFOLDING_H

extern node *SSAConstantFolding (node *syntax_tree);

extern node *SSACFfundef (node *arg_node, node *arg_info);
extern node *SSACFblock (node *arg_node, node *arg_info);
extern node *SSACFarg (node *arg_node, node *arg_info);
extern node *SSACFvardec (node *arg_node, node *arg_info);
extern node *SSACFassign (node *arg_node, node *arg_info);
extern node *SSACFcond (node *arg_node, node *arg_info);
extern node *SSACFreturn (node *arg_node, node *arg_info);
extern node *SSACFlet (node *arg_node, node *arg_info);
extern node *SSACFap (node *arg_node, node *arg_info);
extern node *SSACFid (node *arg_node, node *arg_info);
extern node *SSACFarray (node *arg_node, node *arg_info);
extern node *SSACFprf (node *arg_node, node *arg_info);
/* ##nmw## */
extern node *SSACFNgen (node *arg_node, node *arg_info);
#endif /* SAC_SSACONSTANTFOLDING_H */
