#ifndef _sac_print_interface_h
#define _sac_print_interface_h

#include "tree.h"
#include "globals.h"
extern node *PIHfundef (node *arg_node, node *arg_info);
extern node *PIHarg (node *arg_node, node *arg_info);

extern node *PIWfundef (node *arg_node, node *arg_info);
extern node *PIWarg (node *arg_node, node *arg_info);

extern char *Type2CTypeString (types *type, int flag);
extern char *truncFunName (char *funname);
extern char *truncArgName (char *argname);
extern node *PrintInterface (node *syntax_tree);

#endif /* _sac_print_interface_h */
