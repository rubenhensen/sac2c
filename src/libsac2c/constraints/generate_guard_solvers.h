/*
 * $Id$
 *
 */

#ifndef _SAC_GENERATE_GUARD_SOLVERS_H_
#define _SAC_GENERATE_GUARD_SOLVERS_H_

#include "types.h"

extern node *GGSdoGenerateGuardSolvers (node *arg_node);
extern node *GGSdoRemoveGuardSolvers (node *arg_node);

extern node *GGSfundef (node *arg_node, info *arg_info);
extern node *GGSassign (node *arg_node, info *arg_info);
extern node *GGSprf (node *arg_node, info *arg_info);

#endif _SAC_GENERATE_GUARD_SOLVERS_H_
