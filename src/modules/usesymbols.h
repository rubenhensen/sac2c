/*
 *
 * $Log$
 * Revision 1.3  2004/11/11 14:29:40  sah
 * added some traversal functions for USS traversal
 *
 * Revision 1.2  2004/10/22 14:48:16  sah
 * fixed some typeos
 *
 * Revision 1.1  2004/10/22 13:50:49  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _USESYMBOLS_H
#define _USESYMBOLS_H

#include "types.h"

extern node *USSTypedef (node *arg_node, info *arg_info);
extern node *USSNWithOp (node *arg_node, info *arg_info);
extern node *USSArg (node *arg_node, info *arg_info);
extern node *USSVardec (node *arg_node, info *arg_info);
extern node *USSAp (node *arg_node, info *arg_info);
extern node *USSModul (node *arg_node, info *arg_info);
extern void DoUseSymbols (node *modul);

#endif /* _USESYMBOLS_H */
