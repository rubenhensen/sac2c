#ifndef _sac_print_interface_h
#define _sac_print_interface_h

#include "tree.h"
#include "globals.h"

/*extern node *PrintFundef(node *arg_node, node *arg_info);
extern node *PrintTypedef(node *arg_node, node *arg_info);
extern node *PrintObjdef(node *arg_node, node *arg_info);
extern void PrintFunctionHeader(node *arg_node, node *arg_info);
*/

extern node *PrintInterface (node *syntax_tree);

#endif /* _sac_print_interface_h */
