#ifndef _SAC_MAP_AVIS_TRAV_H_
#define _SAC_MAP_AVIS_TRAV_H_

#include "types.h"

extern node *MATfundef (node *arg_node, info *arg_info);
extern node *MATavis (node *arg_node, info *arg_info);
extern node *MATblock (node *arg_node, info *arg_info);

extern node *MATdoMapAvisTrav (node *fundef, info *arg_info, travfun_p trav);

extern node *MATdoMapAvisTravOneFundef (node *fundef, info *arg_info, travfun_p trav);
#endif /* _SAC_MAP_FUN_AVISV_H_ */
