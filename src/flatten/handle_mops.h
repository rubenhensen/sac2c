/*
 *
 * $Log$
 * Revision 1.1  2002/08/13 10:22:40  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _handle_mops_h
#define _handle_mops_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *HandleMops (node *arg_node);

extern node *HMap (node *arg_node, node *arg_info);
extern node *HMmop (node *arg_node, node *arg_info);

#endif /* _handle_mops_h */
