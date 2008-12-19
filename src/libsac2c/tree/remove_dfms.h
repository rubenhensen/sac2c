/* $Id$ */

#ifndef _SAC_REMOVE_DFMS_H_
#define _SAC_REMOVE_DFMS_H_

#include "types.h"

extern node *RDFMSwith (node *arg_node, info *arg_info);
extern node *RDFMSwith2 (node *arg_node, info *arg_info);
extern node *RDFMSwith3 (node *arg_node, info *arg_info);
extern node *RDFMScond (node *arg_node, info *arg_info);
extern node *RDFMSdo (node *arg_node, info *arg_info);
extern node *RDFMSfundef (node *arg_node, info *arg_info);

extern node *RDFMSdoRemoveDfms (node *arg_node);

#endif /* _SAC_REMOVE_DFMS_H_ */
