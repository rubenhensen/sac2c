/*
 * $Log$
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

extern node *SSAILIarg (node *arg_node, node *arg_info);
extern node *SSAILIfundef (node *arg_node, node *arg_info);
extern node *SSAILIap (node *arg_node, node *arg_info);
#endif /* SAC_SSAINFERELI_H */
