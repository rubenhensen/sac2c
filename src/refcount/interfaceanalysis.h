/*
 *
 * $Log$
 * Revision 1.6  2004/12/09 21:09:26  ktr
 * bugfix roundup
 *
 * Revision 1.5  2004/11/24 15:05:23  ktr
 * added EMIAret.
 *
 * Revision 1.4  2004/11/24 13:11:01  ktr
 * ismop^2
 *
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/11/02 14:28:44  ktr
 * Better loop support.
 *
 * Revision 1.1  2004/10/26 11:18:22  ktr
 * Initial revision
 *
 */
#ifndef _SAC_INTERFACEANALYSIS_H_
#define _SAC_INTERFACEANALYSIS_H_

#include "types.h"

/*****************************************************************************
 *
 * Interface analysis traversal (emia_tab)
 *
 * prefix: EMIA
 *
 ****************************************************************************/
extern node *EMIAdoInterfaceAnalysis (node *syntax_tree);

extern node *EMIAap (node *arg_node, info *arg_info);
extern node *EMIAarg (node *arg_node, info *arg_info);
extern node *EMIAassign (node *arg_node, info *arg_info);
extern node *EMIAblock (node *arg_node, info *arg_info);
extern node *EMIAcond (node *arg_node, info *arg_info);
extern node *EMIAfold (node *arg_node, info *arg_info);
extern node *EMIAfuncond (node *arg_node, info *arg_info);
extern node *EMIAfundef (node *arg_node, info *arg_info);
extern node *EMIAgenarray (node *arg_node, info *arg_info);
extern node *EMIAid (node *arg_node, info *arg_info);
extern node *EMIAlet (node *arg_node, info *arg_info);
extern node *EMIAmodarray (node *arg_node, info *arg_info);
extern node *EMIAret (node *arg_node, info *arg_info);
extern node *EMIAreturn (node *arg_node, info *arg_info);
extern node *EMIAvardec (node *arg_node, info *arg_info);
extern node *EMIAwith (node *arg_node, info *arg_info);
extern node *EMIAwith2 (node *arg_node, info *arg_info);

/****************************************************************************
 *
 * Nodes which MUST NOT be traversed
 *
 * - N_prf
 * - N_icm
 * - N_array
 * - N_objdef
 *
 ****************************************************************************/

#endif /* _SAC_INTERFACEANALYSIS_H_ */
