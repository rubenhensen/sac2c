/*****************************************************************************
 *
 * file:   dfwalk.h
 *
 * description:
 *   header file for dfwalk.c
 *
 *****************************************************************************/

#ifndef _DFWALK_H_
#define _DFWALK_H_

#include "types.h"

extern node *TFDFWdoDFWalk (node *arg_node);
extern node *TFDFWtfspec (node *arg_node, info *arg_info);
extern node *TFDFWtfvertex (node *arg_node, info *arg_info);

#endif /* _DFWALK_H_ */
