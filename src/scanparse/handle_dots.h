/*
 *
 * $Log$
 * Revision 1.3  2002/08/13 09:54:52  sah
 * added several traversalfunctions.
 *
 * Revision 1.2  2002/07/19 13:24:49  sah
 * added functions for traversal.
 *
 * Revision 1.1  2002/07/09 12:54:26  sbs
 * Initial revision
 *
 *
 */

#ifndef _handle_dots_h
#define _handle_dots_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *EliminateSelDots (node *arg_node);
extern node *HDwith (node *arg_node, node *arg_info);
extern node *HDwithop (node *arg_node, node *arg_info);
extern node *HDgenerator (node *arg_node, node *arg_info);
extern node *HDdot (node *arg_node, node *arg_info);
extern node *HDap (node *arg_node, node *arg_info);
extern node *HDprf (node *arg_node, node *arg_info);

#endif /* _handle_dots_h */
