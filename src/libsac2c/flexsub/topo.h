
/*****************************************************************************
 *
 * file:   topo.h
 *
 * description:
 *   header file for topo.c
 *
 *****************************************************************************/

#ifndef _TOPO_H_
#define _TOPO_H_

#include "types.h"

extern node *TFTOPdoTopoSort (node *arg_node);
extern node *TFTOPtfdag (node *arg_node, info *arg_info);
extern node *TFTOPtfvertex (node *arg_node, info *arg_info);

#endif /* _TOPO_H_ */
