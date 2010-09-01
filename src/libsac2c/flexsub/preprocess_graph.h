/*****************************************************************************
 *
 * file:   tf_preprocess_graph.h
 *
 * description:
 *   header file for tf_preprocess_graph.c
 *
 *****************************************************************************/

#ifndef _TF_PREPROCESS_GRAPH_H_
#define _TF_PREPROCESS_GRAPH_H_

#include "types.h"

extern node *TFPPGdoPreprocessTFGraph (node *arg_node);
extern node *TFPPGtfspec (node *arg_node, info *arg_info);
extern node *TFPPGtfdef (node *arg_node, info *arg_info);

#endif /* _TF_PREPROCESS_GRAPH_H_ */
