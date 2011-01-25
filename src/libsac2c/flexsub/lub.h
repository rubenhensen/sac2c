/*****************************************************************************
 *
 * file:   lub.h
 *
 * description:
 *   header file for lub.c
 *
 *****************************************************************************/

#ifndef _LUB_H_
#define _LUB_H_

#include "types.h"

extern node *TFPLBdoLUBPreprocessing (node *arg_node);
extern node *TFPLBtfspec (node *arg_node, info *arg_info);
extern node *TFPLBtfvertex (node *arg_node, info *arg_info);

#endif /* _LUB_H_ */
