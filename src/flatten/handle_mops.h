/*
 *
 * $Log$
 * Revision 1.4  2002/08/13 17:14:34  sbs
 * HMfundef changed into HMAdjustFundef
 *
 * Revision 1.3  2002/08/13 16:34:08  sbs
 * HMfundef added.
 *
 * Revision 1.2  2002/08/13 14:41:00  sbs
 * HMNwithop added.
 * ./
 *
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

extern node *HMmop (node *arg_node, node *arg_info);
extern node *HMap (node *arg_node, node *arg_info);
extern node *HMNwithop (node *arg_node, node *arg_info);

extern node *HMAdjustFundef (node *fundef);

#endif /* _handle_mops_h */
