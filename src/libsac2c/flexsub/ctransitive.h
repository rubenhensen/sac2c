/*****************************************************************************
 *
 * file:   ctransitive.h
 *
 * description:
 *   header file for ctransitive.c
 *
 *****************************************************************************/

#ifndef __CTRANSITIVE_H__
#define __CTRANSITIVE_H__

#include "types.h"

extern node *TFCTRdoCrossClosure (node *arg_node);
extern node *TFCTRtfspec (node *arg_node, info *arg_info);
extern node *TFCTRtfvertex (node *arg_node, info *arg_info);

#endif
