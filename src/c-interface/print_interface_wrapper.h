/*
 * $Log$
 * Revision 1.1  2000/08/02 14:25:07  nmw
 * Initial revision
 *
 */

#ifndef _sac_print_interface_wrapper_h
#define _sac_print_interface_wrapper_h

#include "tree.h"
#include "globals.h"

extern node *PIWmodul (node *arg_node, node *arg_info);
extern node *PIWcwrapper (node *arg_node, node *arg_info);
extern node *PIWfundef (node *arg_node, node *arg_info);
extern node *PIWarg (node *arg_node, node *arg_info);
extern node *PIWobjdef (node *arg_node, node *arg_info);

#endif /* _sac_print_interface_wrapper_h */
