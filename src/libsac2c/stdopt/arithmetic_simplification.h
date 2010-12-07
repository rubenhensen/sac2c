/*
 * $Id$
 */
#ifndef _SAC_ARITHMETIC_SIMPLIFICATION_H_
#define _SAC_ARITHMETIC_SIMPLIFICATION_H_

#include "types.h"

/******************************************************************************
 *
 * arithmetic simplification
 *
 * prefix: AS
 *
 *****************************************************************************/
extern node *ASdoArithmeticSimplification (node *arg_node);
extern node *ASdoArithmeticSimplificationOneFundefAnon (node *arg_node, info *arg_info);
extern node *ASdoArithmeticSimplificationModule (node *arg_node);

extern node *ASfundef (node *arg_node, info *arg_info);
extern node *ASassign (node *arg_node, info *arg_info);
extern node *ASprf (node *arg_node, info *arg_info);

#endif /* _SAC_ARITHMETIC_SIMPLIFICATION_H_ */
