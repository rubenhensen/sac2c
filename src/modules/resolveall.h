/*
 *
 * $Log$
 * Revision 1.1  2004/10/21 17:19:00  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _RESOLVE_ALL_H
#define _RESOLVE_ALL_H

#include "types.h"

extern void ResolveAll (node *modul);

extern node *RSAUse (node *arg_node, info *arg_info);
extern node *RSAImport (node *arg_node, info *arg_info);
extern node *RSAProvide (node *arg_node, info *arg_info);
extern node *RSAExport (node *arg_node, info *arg_info);
extern node *RSAModul (node *arg_node, info *arg_info);

#endif /* _RESOLVE_ALL_H */
