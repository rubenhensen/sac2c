/*
 *
 * $Log$
 * Revision 3.2  2001/03/22 18:55:32  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:03:44  sacbase
 * new release made
 *
 * Revision 1.1  2000/08/02 14:22:47  nmw
 * Initial revision
 *
 */

#ifndef _sac_print_interfaceheader_h
#define _sac_print_interfaceheader_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"

extern node *PIHmodul (node *arg_node, node *arg_info);
extern node *PIHcwrapper (node *arg_node, node *arg_info);
extern node *PIHfundef (node *arg_node, node *arg_info);
extern node *PIHarg (node *arg_node, node *arg_info);

node *PIHcwrapperPrototype (node *wrapper, node *arg_info);

#endif /* _sac_print_interfaceheader_h */
