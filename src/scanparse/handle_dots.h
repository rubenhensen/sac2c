/*
 *
 * $Log$
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

extern node *HDprf (node *arg_node, node *arg_info);

#endif /* _handle_dots_h */
