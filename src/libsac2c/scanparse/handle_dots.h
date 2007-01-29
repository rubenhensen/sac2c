/*
 *
 * $Log$
 * Revision 1.10  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.9  2004/11/25 22:26:47  sah
 * COMPILES!
 *
 *
 * Revision 1.1  2002/07/09 12:54:26  sbs
 * Initial revision
 *
 */

#ifndef _HANDLE_DOTS_H_
#define _HANDLE_DOTS_H_

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *HDdoEliminateSelDots (node *arg_node);
extern node *HDwith (node *arg_node, info *arg_info);
extern node *HDgenarray (node *arg_node, info *arg_info);
extern node *HDmodarray (node *arg_node, info *arg_info);
extern node *HDfold (node *arg_node, info *arg_info);
extern node *HDpart (node *arg_node, info *arg_info);
extern node *HDgenerator (node *arg_node, info *arg_info);
extern node *HDdot (node *arg_node, info *arg_info);
extern node *HDspap (node *arg_node, info *arg_info);
extern node *HDprf (node *arg_node, info *arg_info);
extern node *HDassign (node *arg_node, info *arg_info);
extern node *HDsetwl (node *arg_node, info *arg_info);
extern node *HDspid (node *arg_node, info *arg_info);

#endif /* _HANDLE_DOTS_H_ */
