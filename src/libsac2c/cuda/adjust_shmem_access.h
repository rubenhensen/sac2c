

#ifndef _ADJUST_SHMEM_ACCESS_H_
#define _ADJUST_SHMEM_ACCESS_H_

#include "types.h"

extern node *ASHAdoAdjustShmemAccess (node *syntax_tree);
extern node *ASHAlet (node *arg_node, info *arg_info);
extern node *ASHAwith (node *arg_node, info *arg_info);
extern node *ASHApart (node *arg_node, info *arg_info);
extern node *ASHAcode (node *arg_node, info *arg_info);
extern node *ASHAprf (node *arg_node, info *arg_info);

#endif
