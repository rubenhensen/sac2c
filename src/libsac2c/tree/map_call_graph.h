#ifndef _SAC_MAP_CALL_GRAPH_H_
#define _SAC_MAP_CALL_GRAPH_H_

#include "types.h"

extern node *MCGfundef (node *arg_node, info *arg_info);
extern node *MCGap (node *arg_node, info *arg_info);

extern bool MCGcontNever (node *arg_node, info *arg_info);
extern bool MCGcontAlways (node *arg_node, info *arg_info);
extern bool MCGcontLacFun (node *arg_node, info *arg_info);
extern bool MCGcontOneLevel (node *arg_node, info *arg_info);
extern bool MCGcontLacFunAndOneLevel (node *arg_node, info *arg_info);

extern info *MCGdoMapCallGraph (node *fundef, travfun_p mapfundown, travfun_p mapfunup,
                                bool (*contfun) (node *, info *), info *info);

#endif /* _SAC_MAP_CALL_GRAPH_H */
