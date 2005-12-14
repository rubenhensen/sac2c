#ifndef _SAC_SPMD_EMM_H_
#define _SAC_SPMD_EMM_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id$
 *
 * Spmd EMM traversal (spmdemm_tab)
 *
 * prefix: SPMDEMM
 *
 *****************************************************************************/
extern node *SPMDEMMdoSpmdEmm (node *arg_node);

extern node *SPMDEMMassign (node *arg_node, info *arg_info);
extern node *SPMDEMMlet (node *arg_node, info *arg_info);
extern node *SPMDEMMprf (node *arg_node, info *arg_info);
extern node *SPMDEMMspmd (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_EMM_H_ */
