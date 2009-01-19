/* $Id$ */

#ifndef _SAC_INFER_DFMS_H_
#define _SAC_INFER_DFMS_H_

#include "types.h"

/*
 * bit field to steer hiding of local vars
 */

/*
 * bit definitions
 */
#define HIDE_LOCALS_DO 1
#define HIDE_LOCALS_WHILE 2
#define HIDE_LOCALS_COND 4
#define HIDE_LOCALS_WITH 8
#define HIDE_LOCALS_WITH2 16
#define HIDE_LOCALS_WITH3 32
#define HIDE_LOCALS_BLOCK 64

/*
 * pre-defined bit field values
 */

#define HIDE_LOCALS_NEVER 0
#define HIDE_LOCALS_LAC (HIDE_LOCALS_DO | HIDE_LOCALS_WHILE | HIDE_LOCALS_COND)

extern node *INFDFMSfundef (node *arg_node, info *arg_info);
extern node *INFDFMSarg (node *arg_node, info *arg_info);
extern node *INFDFMSassign (node *arg_node, info *arg_info);
extern node *INFDFMSlet (node *arg_node, info *arg_info);
extern node *INFDFMSap (node *arg_node, info *arg_info);
extern node *INFDFMSid (node *arg_node, info *arg_info);
extern node *INFDFMSids (node *arg_node, info *arg_info);
extern node *INFDFMScode (node *arg_node, info *arg_info);
extern node *INFDFMSrange (node *arg_node, info *arg_info);
extern node *INFDFMSwith (node *arg_node, info *arg_info);
extern node *INFDFMScond (node *arg_node, info *arg_info);
extern node *INFDFMSwith2 (node *arg_node, info *arg_info);
extern node *INFDFMSwith3 (node *arg_node, info *arg_info);
extern node *INFDFMSdo (node *arg_node, info *arg_info);
extern node *INFDFMSblock (node *arg_node, info *arg_info);

extern node *INFDFMSdoInferDfms (node *syntax_tree, int hide_locals);
extern dfmask_t *INFDFMSdoInferInDfmAssignChain (node *assign, node *fundef);

#endif /* _SAC_INFER_DFMS_H_ */
