

#ifndef _SAC_MINIMIZE_BLOCK_TRANSFERS2_H_
#define _SAC_MINIMIZE_BLOCK_TRANSFERS2_H_

#include "types.h"

extern node *MBTRAN2doMinimizeBlockTransfers (node *arg_node);
extern node *MBTRAN2prf (node *arg_node, info *arg_info);
extern node *MBTRAN2block (node *arg_node, info *arg_info);
extern node *MBTRAN2assign (node *arg_node, info *arg_info);

#endif
