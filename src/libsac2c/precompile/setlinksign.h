#ifndef _SAC_SETLINKSIGN_H_
#define _SAC_SETLINKSIGN_H_

#include "types.h"

extern node *SLSdoSetLinksign (node *tree);

extern node *SLSret (node *arg_node, info *arg_info);
extern node *SLSarg (node *arg_node, info *arg_info);
extern node *SLSfundef (node *arg_node, info *arg_info);
extern node *SLSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_SETLINKSIGN_H_ */
