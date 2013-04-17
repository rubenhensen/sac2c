#ifndef _SAC_INSERT_VARDEC_H_
#define _SAC_INSERT_VARDEC_H_

#include "types.h"

/******************************************************************************
 *
 * Insert Vardec traversal ( insvd_tab)
 *
 * Prefix: INSVD
 *
 *****************************************************************************/
extern node *INSVDdoInsertVardec (node *syntaxtree);

extern node *INSVDmodule (node *arg_node, info *arg_info);
extern node *INSVDfundef (node *arg_node, info *arg_info);
extern node *INSVDspap (node *arg_node, info *arg_info);
extern node *INSVDspfold (node *arg_node, info *arg_info);
extern node *INSVDspid (node *arg_node, info *arg_info);
extern node *INSVDspids (node *arg_node, info *arg_info);
extern node *INSVDwith (node *arg_node, info *arg_info);
extern node *INSVDpart (node *arg_node, info *arg_info);
extern node *INSVDcode (node *arg_node, info *arg_info);
extern node *INSVDlet (node *arg_node, info *arg_info);
extern node *INSVDdo (node *arg_node, info *arg_info);
extern node *INSVDspfold (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_VARDEC_H_ */
