/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 3.2  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 17:59:22  sacbase
 * new release made
 *
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_FUN2LAC_H_
#define _SAC_FUN2LAC_H_

#include "types.h"

/*****************************************************************************
 *
 * Fun2lac traversal ( fun2lac_tab)
 *
 * prefix: F2L
 *
 * description:
 *
 *   header file for fun2lac.c.
 *
 *****************************************************************************/
extern node *F2LdoFun2Lac (node *syntax_tree);

extern node *F2Lmodule (node *arg_node, info *arg_info);
extern node *F2Lfundef (node *arg_node, info *arg_info);
extern node *F2Lblock (node *arg_node, info *arg_info);
extern node *F2Lassign (node *arg_node, info *arg_info);
extern node *F2Llet (node *arg_node, info *arg_info);
extern node *F2Lap (node *arg_node, info *arg_info);

#endif /* _SAC_FUN2LAC_H_ */
