#ifndef _SAC_CREATE_WRAPPER_CODE_H_
#define _SAC_CREATE_WRAPPER_CODE_H_

#include "types.h"

extern node *CWCdoCreateWrapperCode (node *ast);

extern bool CWChasWrapperCode (node *fundef);
extern node *CWCfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WRAPPER_CODE_H_ */
