/*
 *
 * $Log$
 * Revision 1.4  2004/07/23 13:58:18  ktr
 * Added CVPNwithop to prevent CVP to propagate the neutral element.
 *
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.2  2004/03/06 20:06:40  mwe
 * CVPfuncond added
 *
 * Revision 1.1  2004/03/02 16:58:51  mwe
 * Initial revision
 *
 *
 */

#ifndef _ConstVarPropagation_h_
#define _ConstVarPropagation_h_

extern node *ConstVarPropagation (node *arg_node);
extern node *CVPfundef (node *arg_node, info *arg_info);
extern node *CVPblock (node *arg_node, info *arg_info);
extern node *CVPassign (node *arg_node, info *arg_info);
extern node *CVPlet (node *arg_node, info *arg_info);
extern node *CVPNcode (node *arg_node, info *arg_info);
extern node *CVPcond (node *arg_node, info *arg_info);
extern node *CVPNwith (node *arg_node, info *arg_info);
extern node *CVPNwithop (node *arg_node, info *arg_info);
extern node *CVPap (node *arg_node, info *arg_info);
extern node *CVPprf (node *arg_node, info *arg_info);
extern node *CVPexprs (node *arg_node, info *arg_info);
extern node *CVPid (node *arg_node, info *arg_info);
extern node *CVPreturn (node *arg_node, info *arg_info);
extern node *CVParray (node *arg_node, info *arg_info);
extern node *CVPfuncond (node *arg_node, info *arg_info);
#endif
