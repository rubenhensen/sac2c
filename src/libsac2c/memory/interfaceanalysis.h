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

extern node *SHALprintPreFun (node *arg_node, info *arg_info);

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
extern node *EMIAprf (node *arg_node, info *arg_info);
extern node *EMIAret (node *arg_node, info *arg_info);
extern node *EMIAreturn (node *arg_node, info *arg_info);
extern node *EMIAvardec (node *arg_node, info *arg_info);
extern node *EMIAwith (node *arg_node, info *arg_info);
extern node *EMIAwith2 (node *arg_node, info *arg_info);

/****************************************************************************
 *
 * Nodes which MUST NOT be traversed
 *
 * - N_icm
 * - N_array
 * - N_objdef
 *
 ****************************************************************************/

#endif /* _SAC_INTERFACEANALYSIS_H_ */
