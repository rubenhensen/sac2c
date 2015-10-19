#ifndef _SAC_CREATE_F_WRAPPER_HEADER_H_
#define _SAC_CREATE_F_WRAPPER_HEADER_H_

#include "types.h"

typedef struct HOLDER holder;

extern node *CFWHdoCreateFWrapperHeader (node *syntax_tree);
extern node *CFWHdoCreateCWrapperHeader (node *syntax_tree);

extern node *CFWHfunbundle (node *arg_node, info *arg_info);
extern node *CFWHfundef (node *arg_node, info *arg_info);
extern node *CFWHarg (node *arg_node, info *arg_info);
extern node *CFWHret (node *arg_node, info *arg_info);
extern node *CFWHtypedef (node *arg_node, info *arg_info);
extern node *CFWHmodule (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_F_WRAPPER_HEADER_H_ */
