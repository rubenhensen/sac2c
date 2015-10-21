#ifndef _SAC_AUTO_INLINING_H_
#define _SAC_AUTO_INLINING_H_

#include "types.h"

extern node *AINLdoAutoInlining (node *arg_node);

extern node *AINLap (node *arg_node, info *arg_info);
extern node *AINLprf (node *arg_node, info *arg_info);
extern node *AINLwith (node *arg_node, info *arg_info);
extern node *AINLpart (node *arg_node, info *arg_info);
extern node *AINLcode (node *arg_node, info *arg_info);
extern node *AINLfundef (node *arg_node, info *arg_info);

#endif /* _SAC_AUTO_INLINING_H_ */
