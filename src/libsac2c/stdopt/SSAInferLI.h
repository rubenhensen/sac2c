/******************************************************************************
 *
 * $Id$
 *
 * Infer Loop Invariants
 *
 * Prefix: ILI
 *
 * description:
 *
 *   This module infers the loop invariant args of do and while loops.
 *
 *****************************************************************************/
#ifndef _SAC_SSAINFERELI_H_
#define _SAC_SSAINFERELI_H_

#include "types.h"

extern node *ILIdoInferLoopInvariants (node *fundef);

extern node *ILIarg (node *arg_node, info *arg_info);
extern node *ILIfundef (node *arg_node, info *arg_info);
extern node *ILIap (node *arg_node, info *arg_info);

#endif /* _SAC_SSAINFERELI_H_ */
