/*
 * $Log$
 * Revision 1.3  2004/11/22 18:33:19  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.1  2001/05/30 13:48:36  nmw
 * Initial revision
 *
 *
 */

#ifndef _SAC_SSAINFERELI_H_
#define _SAC_SSAINFERELI_H_

#include "types.h"

/******************************************************************************
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
extern node *ILIdoInferLoopInvariants (node *fundef);

extern node *ILIarg (node *arg_node, info *arg_info);
extern node *ILIfundef (node *arg_node, info *arg_info);
extern node *ILIap (node *arg_node, info *arg_info);

#endif /* _SAC_SSAINFERELI_H_ */
