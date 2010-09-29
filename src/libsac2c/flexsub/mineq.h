/*****************************************************************************
 *
 * file:   mineq.h
 *
 * description:
 *   header file for mineq.c
 *
 *****************************************************************************/

#ifndef _MINEQ_H_
#define _MINEQ_H_

#include "types.h"

extern node *TFMINdoReduceTFGraph (node *arg_node);
extern node *TFMINtfspec (node *arg_node, info *arg_info);
extern node *TFMINtfvertex (node *arg_node, info *arg_info);

#endif /* _MINEQ_H_ */
