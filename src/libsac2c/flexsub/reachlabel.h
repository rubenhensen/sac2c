/*****************************************************************************
 *
 * file:   reachlabel.h
 *
 * description:
 *   header file for reachlabel.c
 *
 *****************************************************************************/

#ifndef _REACHLABEL_H_
#define _REACHLABEL_H_

#include "types.h"

extern node *TFRCHdoReachabilityAnalysis (node *arg_node);
extern node *TFRCHtfspec (node *arg_node, info *arg_info);
extern node *TFRCHtfvertex (node *arg_node, info *arg_info);

#endif /* _REACHLABEL_H_ */
