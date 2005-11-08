/*
 *
 * $Log$
 * Revision 1.1  2005/09/12 13:56:20  ktr
 * Initial revision
 *
 */
#ifndef _SAC_WLSIMPLIFICATION_H_
#define _SAC_WLSIMPLIFICATION_H_

#include "types.h"

/******************************************************************************
 *
 * With-loop simplification traversal ( wlsimp_tab)
 *
 * Prefix: WLSIMP
 *
 *****************************************************************************/
extern node *WLSIMPdoWithloopSimplification (node *syntax_tree);

extern node *WLSIMPfundef (node *arg_node, info *arg_info);
extern node *WLSIMPassign (node *arg_node, info *arg_info);
extern node *WLSIMPwith (node *arg_node, info *arg_info);
extern node *WLSIMPcode (node *arg_node, info *arg_info);
extern node *WLSIMPpart (node *arg_node, info *arg_info);
extern node *WLSIMPgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITCOPY_H_ */
