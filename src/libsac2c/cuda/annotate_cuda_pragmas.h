#ifndef _SAC_ANNOTATE_CUDA_PRAGMAS_H_
#define _SAC_ANNOTATE_CUDA_PRAGMAS_H_

#include "types.h"

extern node *ACPdoAnnotateCUDAPragmas (node *arg_node);
extern node *ACPwith (node *arg_node, info *arg_info);
extern node *ACPpart (node *arg_node, info *arg_info);
extern node *ACPgenerator (node *arg_node, info *arg_info);

#endif
