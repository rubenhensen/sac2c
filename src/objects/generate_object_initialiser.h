/* $Id$ */

#ifndef _SAC_GENERATE_OBJECT_INITIALISER_H_
#define _SAC_GENERATE_OBJECT_INITIALISER_H_

#include "types.h"

extern node *GOIfundef (node *arg_node, info *arg_info);
extern node *GOImodule (node *arg_node, info *arg_info);

extern node *GOIdoGenerateObjectInitialiser (node *syntax_tree);

#endif /* _SAC_GENERATE_OBJECT_INITIALISER_H_ */
