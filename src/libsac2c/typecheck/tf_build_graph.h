/*****************************************************************************
 *
 * file:   tf_build_graph.h
 *
 * description:
 *   header file for tf_build_graph.c
 *
 *****************************************************************************/

#ifndef _TF_BUILD_GRAPH_H_
#define _TF_BUILD_GRAPH_H_

#include "types.h"

extern node *TFBDGdoBuildTFGraph (node *arg_node);
extern node *TFBDGtfspec (node *arg_node, info *arg_info);
extern node *TFBDGtfdef (node *arg_node, info *arg_info);

#endif /* _TF_BUILD_GRAPH_H_ */
