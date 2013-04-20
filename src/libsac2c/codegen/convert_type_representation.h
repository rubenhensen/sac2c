#ifndef _SAC_CONVERT_TYPE_REPRESENTATION_H_
#define _SAC_CONVERT_TYPE_REPRESENTATION_H_

#include "types.h"

extern node *CTRdoConvertToOldTypes (node *syntax_tree);

extern node *CTRfundef (node *arg_node, info *arg_info);
extern node *CTRarg (node *arg_node, info *arg_info);
extern node *CTRblock (node *arg_node, info *arg_info);
extern node *CTRvardec (node *arg_node, info *arg_info);
extern node *CTRret (node *arg_node, info *arg_info);

#endif /* _SAC_CONVERT_TYPE_REPRESENTATION_H_ */
