/*
 *
 * $Log$
 * Revision 1.5  2004/11/14 15:25:38  sah
 * implemented support for udts
 * some cleanup
 *
 * Revision 1.4  2004/10/28 17:20:46  sah
 * now deserialize as an internal state
 *
 * Revision 1.3  2004/10/26 09:36:20  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.1  2004/09/23 20:44:53  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _DESERIALIZE_H
#define _DESERIALIZE_H

#include "types.h"
#include "new_types.h"
#include "modulemanager.h"

extern void InitDeserialize (node *module);
extern void FinishDeserialize (node *module);

extern node *AddSymbolByName (const char *symbol, STentrytype_t type, const char *module);
extern node *AddSymbolById (const char *symbid, const char *module);

/*
 * hooks for deserialization
 */
extern ntype *DeserializeLookupUserType (const char *mod, const char *name);
extern ntype *DeserializeLookupSymbolType (const char *mod, const char *name);
extern node *DeserializeLookupFunction (const char *module, const char *symbol);

/*
 * DS traversal
 */
extern node *AddFunctionBodyToHead (node *fundef);

extern node *DSFundef (node *arg_node, info *arg_info);
extern node *DSReturn (node *arg_node, info *arg_info);
extern node *DSBlock (node *arg_node, info *arg_info);
extern node *DSArg (node *arg_node, info *arg_info);
extern node *DSVardec (node *arg_node, info *arg_info);
extern node *DSId (node *arg_node, info *arg_info);
extern node *DSLet (node *arg_node, info *arg_info);
extern node *DSNWithid (node *arg_node, info *arg_info);

#endif /* _DESERIALIZE_H */
