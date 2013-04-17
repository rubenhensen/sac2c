#ifndef _SAC_PREPARE_INLINE_H_
#define _SAC_PREPARE_INLINE_H_

#include "types.h"

/******************************************************************************
 *
 * Prepareinline traversal ( ppi_tab)
 *
 * Prefix: PPI
 *
 *****************************************************************************/
extern node *PPIdoPrepareInline (node *syntax_tree);

extern node *PPIfundef (node *arg_node, info *arg_info);
extern node *PPIap (node *arg_node, info *arg_info);
extern node *PPIfold (node *arg_node, info *arg_info);
extern node *PPImodule (node *arg_node, info *arg_info);

#endif /* _SAC_PREPARE_INLINE_H_ */
