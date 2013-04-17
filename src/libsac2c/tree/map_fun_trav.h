#ifndef _SAC_MAP_FUN_TRAV_H_
#define _SAC_MAP_FUN_TRAV_H_

#include "types.h"

extern node *MFTfundef (node *arg_node, info *arg_info);

extern node *MFTdoMapFunTrav (node *fundef, info *arg_info, travfun_p trav);

#endif /* _SAC_MAP_FUN_TRAV_H_ */
