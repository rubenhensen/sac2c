/*
 *
 * $Log$
 * Revision 1.1  2004/10/28 17:40:11  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _PREPARE_INLINE_H
#define _PREPARE_INLINE_H

#include "types.h"

extern node *PPIFundef (node *arg_node, info *arg_info);
extern node *PPIModul (node *arg_node, info *arg_info);
extern void PrepareInline (node *syntax_tree);

#endif
