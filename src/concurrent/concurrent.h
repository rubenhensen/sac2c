/*
 *
 * $Log$
 * Revision 3.3  2004/11/21 23:01:01  ktr
 * ISMOP 2004!!!!!!!
 *
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:25  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:05  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_CONCURRENT_H_
#define _SAC_CONCURRENT_H_

#include "types.h"

/*****************************************************************************
 *
 * Concurrent traversal (conc_tab)
 *
 * prefix: CONC
 *
 *****************************************************************************/
extern node *CONCdoBuildSpmdRegions (node *syntax_tree);

extern node *CONCmodule (node *arg_node, info *arg_info);
extern node *CONCfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CONCURRENT_H_ */
