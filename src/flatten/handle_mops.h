/*
 *
 * $Log$
 * Revision 1.6  2002/09/11 23:22:42  dkr
 * HMAdjustFunNames() removed
 *
 * Revision 1.5  2002/08/14 11:51:22  sbs
 * HMAdjustFunNames debugged....
 *
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

#endif /* _handle_mops_h */
