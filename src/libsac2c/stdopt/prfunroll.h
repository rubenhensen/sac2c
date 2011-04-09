/*
 * $Id$
 */
#ifndef _SAC_PRFUNROLL_H_
#define _SAC_PRFUNROLL_H_

#include "types.h"

/******************************************************************************
 *
 * PRF unroll traversal
 *
 * Prefix: UPRF
 *
 *****************************************************************************/
extern node *UPRFdoUnrollPRFs (node *fundef);
extern node *UPRFdoUnrollPRFsPrf (node *arg_node, node **vardecs, node **preassigns,
                                  node *lhs);
extern node *UPRFdoUnrollPRFsModule (node *syntax_tree);

extern node *UPRFfundef (node *arg_node, info *arg_info);
extern node *UPRFassign (node *arg_node, info *arg_info);
extern node *UPRFlet (node *arg_node, info *arg_info);
extern node *UPRFprf (node *arg_node, info *arg_info);

#endif /* _SAC_DATAREUSE_H_ */
