#ifndef _SAC_PRA_H_
#define _SAC_PRA_H_

#include "types.h"

/******************************************************************************
 *
 * Polyhedral based WITH-loop reuse candidate analysis
 *
 * Prefix: PRA
 *
 *****************************************************************************/
extern node *PRAdoPolyhedralReuseAnalysis (node *with, node *fundef);
extern node *PRAprf (node *arg_node, info *arg_info);
extern node *PRApart (node *arg_node, info *arg_info);
extern node *PRAassign (node *arg_node, info *arg_info);
extern node *PRAwith (node *arg_node, info *arg_info);
extern node *PRAap (node *arg_node, info *arg_info);
extern node *PRAcond (node *arg_node, info *arg_info);
extern node *PRAfundef (node *arg_node, info *arg_info);

#endif /* _SAC_PRA_H_ */
