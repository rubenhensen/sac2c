/**
 * @file resolvesymboltypes.h
 * @brief this phase resolves all symbol-types to the
 *        corresponding user-defined-types or
 *        generates an error message otherwise
 * @author Stephan Herhut
 * @date 2005-05-31
 */

#ifndef _SAC_RESOLVESYMBOLTABLES_H_
#define _SAC_RESOLVESYMBOLTABLES_H_

#include "types.h"

extern node *RSTmodule (node *arg_node, info *arg_info);
extern node *RSTtypedef (node *arg_node, info *arg_info);
extern node *RSTobjdef (node *arg_node, info *arg_info);
extern node *RSTfundef (node *arg_node, info *arg_info);
extern node *RSTarg (node *arg_node, info *arg_info);
extern node *RSTret (node *arg_node, info *arg_info);
extern node *RSTavis (node *arg_node, info *arg_info);
extern node *RSTarray (node *arg_node, info *arg_info);
extern node *RSTcast (node *arg_node, info *arg_info);
extern node *RSTtype (node *arg_node, info *arg_info);
extern node *RSTstructelem (node *arg_node, info *arg_info);

extern node *RSTdoResolveSymbolTypes (node *syntax_tree);
#endif
