/*
 *
 * $Log$
 * Revision 1.2  2005/02/16 22:29:13  sah
 * changed link handling
 *
 * Revision 1.1  2004/11/23 22:40:40  sah
 * Initial revision
 *
 * Revision 1.8  2004/11/23 21:20:48  sah
 *
 *
 */

#ifndef _SAC_DESERIALIZE_H_
#define _SAC_DESERIALIZE_H_

#include "types.h"

extern void DSinitDeserialize (node *module);
extern void DSfinishDeserialize (node *module);

extern node *DSaddSymbolByName (const char *symbol, stentrytype_t type,
                                const char *module);
extern node *DSaddSymbolById (const char *symbid, const char *module);

/*
 * hooks for deserialization
 */
extern ntype *DSloadUserType (const char *mod, const char *name);
extern ntype *DSloadSymbolType (const char *mod, const char *name);
extern node *DSlookupFunction (const char *module, const char *symbol);
extern node *DSfetchArgAvis (int pos);

/*
 * DS traversal
 */
extern node *DSdoDeserialize (node *fundef);

extern node *DSfundef (node *arg_node, info *arg_info);
extern node *DSreturn (node *arg_node, info *arg_info);
extern node *DSblock (node *arg_node, info *arg_info);
extern node *DSarg (node *arg_node, info *arg_info);

#endif /* _SAC_DESERIALIZE_H_ */
