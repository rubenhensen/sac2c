/*
 * $Log$
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

/*****************************************************************************
 *
 * file:   SSAInferLI.h
 *
 * prefix: SSAILI
 *
 * description:
 *
 *   This module infers the loop invariant args of do and while loops.
 *
 *
 *****************************************************************************/

#ifndef SAC_SSAINFERELI_H

#define SAC_SSAINFERELI_H

extern node *SSAInferLoopInvariants (node *fundef);

extern node *SSAILIarg (node *arg_node, info *arg_info);
extern node *SSAILIfundef (node *arg_node, info *arg_info);
extern node *SSAILIap (node *arg_node, info *arg_info);
#endif /* SAC_SSAINFERELI_H */
