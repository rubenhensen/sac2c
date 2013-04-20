/******************************************************************************
 *
 * Iteration Invariant Parameter Inference
 *
 * Prefix: IIPI
 *
 * description:
 *
 *   This module identifies parameters of loop functions that are passed on
 *   directly to the recursive call. Typically, these are a result of a
 *   previously successfull application of LIR.
 *
 *****************************************************************************/
#ifndef _SAC_ITERATION_INVARIANT_PARAMETER_INFERENCE_H_
#define _SAC_ITERATION_INVARIANT_PARAMETER_INFERENCE_H_

#include "types.h"

extern node *IIPIdoIterationInvariantParameterInference (node *fundef);

extern node *IIPIarg (node *arg_node, info *arg_info);
extern node *IIPIfundef (node *arg_node, info *arg_info);
extern node *IIPIap (node *arg_node, info *arg_info);

#endif /* _SAC_ITERATION_INVARIANT_PARAMETER_INFERENCE_H_ */
