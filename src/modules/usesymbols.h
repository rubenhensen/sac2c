/*
 *
 * $Log$
 * Revision 1.1  2004/10/22 13:50:49  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _USESYMBOLS_H
#define _USESYMBOLS_H

#include "types.h"

extern node *USSAp (node *arg_node, info *arg_info);
extern node *USSModul (node *arg_node, info *arg_info);
extern void DoUseSymbols (arg_node *modul);

#endif /* _USESYMBOLS_H */
