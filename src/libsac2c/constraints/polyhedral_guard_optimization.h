/*
 * $Id$
 *
 */

#ifndef _SAC_POLYHEDRAL_GUARD_OPTIMIZATION_H_
#define _SAC_POLYHEDRAL_GUARD_OPTIMIZATION_H_

#include "types.h"

extern node *POGOdoPolyhedralGuardOptimization (node *arg_node);

extern node *POGOfundef (node *arg_node, info *arg_info);
extern node *POGOpart (node *arg_node, info *arg_info);
extern node *POGOwith (node *arg_node, info *arg_info);
extern node *POGOassign (node *arg_node, info *arg_info);
extern node *POGOlet (node *arg_node, info *arg_info);
extern node *POGOprf (node *arg_node, info *arg_info);
extern bool POGOisPogoPrf (prf nprf);

#endif // _SAC_POLYHEDRAL_GUARD_OPTIMIZATION_H_
