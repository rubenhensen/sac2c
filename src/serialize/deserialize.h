/*
 *
 * $Log$
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

/*
 * DS traversal
 */
extern node *DSdoDeserialize (node *fundef);

extern node *DSFundef (node *arg_node, info *arg_info);
extern node *DSReturn (node *arg_node, info *arg_info);
extern node *DSBlock (node *arg_node, info *arg_info);
extern node *DSArg (node *arg_node, info *arg_info);
extern node *DSVardec (node *arg_node, info *arg_info);
extern node *DSId (node *arg_node, info *arg_info);
extern node *DSLet (node *arg_node, info *arg_info);
extern node *DSNWithid (node *arg_node, info *arg_info);

#endif /* _SAC_DESERIALIZE_H_ */
