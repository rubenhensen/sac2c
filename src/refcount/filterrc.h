/*
 *
 * $Log$
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/11/02 14:34:59  ktr
 * Better support for conditionals.
 * AP_ARGS are now actually traversed.
 *
 * Revision 1.1  2004/10/12 10:08:05  ktr
 * Initial revision
 *
 */

#ifndef _SAC_FILTERRC_H_
#define _SAC_FILTERRC_H_

#include "types.h"

/******************************************************************************
 *
 * Filter reuse candidates traversal (emfrc_tab)
 *
 * prefix: EMFRC
 *
 *****************************************************************************/
extern node *EMFRCdoFilterReuseCandidates (node *syntax_tree);

extern node *EMFRCap (node *arg_node, info *arg_info);
extern node *EMFRCarg (node *arg_node, info *arg_info);
extern node *EMFRCassign (node *arg_node, info *arg_info);
extern node *EMFRCcond (node *arg_node, info *arg_info);
extern node *EMFRCfold (node *arg_node, info *arg_info);
extern node *EMFRCfuncond (node *arg_node, info *arg_info);
extern node *EMFRCfundef (node *arg_node, info *arg_info);
extern node *EMFRCgenarray (node *arg_node, info *arg_info);
extern node *EMFRCid (node *arg_node, info *arg_info);
extern node *EMFRCmodarray (node *arg_node, info *arg_info);
extern node *EMFRCprf (node *arg_node, info *arg_info);
extern node *EMFRCwith (node *arg_node, info *arg_info);
extern node *EMFRCwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_FILTERRC_H_ */
