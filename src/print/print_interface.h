#ifndef _sac_print_interface_h
#define _sac_print_interface_h

#include "tree.h"
#include "globals.h"

extern node *PIHfundef (node *arg_node, node *arg_info);
extern node *PIHarg (node *arg_node, node *arg_info);

extern char *truncFunName (char *funname);

extern node *PrintInterface (node *syntax_tree);

#endif /* _sac_print_interface_h */
