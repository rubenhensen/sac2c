#ifndef _SET_EXPRESSSION_RANGE_INFERENCE_H_
#define _SET_EXPRESSSION_RANGE_INFERENCE_H_

#include "types.h"

extern node *SERIdoInferRanges (node *arg_node);
extern node *SERIprf (node *arg_node, info *arg_info);
extern node *SERIspap (node *arg_node, info *arg_info);
extern node *SERIwith (node *arg_node, info *arg_info);
extern node *SERIsetwl (node *arg_node, info *arg_info);
extern node *SERIgenerator (node *arg_node, info *arg_info);

#endif /* _SET_EXPRESSSION_RANGE_INFERENCE_H_ */
