/*
 *
 * $Log$
 * Revision 1.2  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.1  2004/10/28 17:40:11  sah
 * Initial revision
 *
 *
 *
 */

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
extern node *PPImodule (node *arg_node, info *arg_info);

#endif /* _SAC_PREPARE_INLINE_H_ */
