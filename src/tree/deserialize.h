/*
 *
 * $Log$
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
#include "modulemanager.h"

extern void InitDeserialize (node *module);
extern void FinishDeserialize (node *module);

extern node *AddFunctionBodyToHead (node *fundef);
extern void AddSymbolToAst (const char *symbol, module_t *module);
extern node *DeserializeLookupFunction (const char *module, const char *symbol,
                                        info *info);

extern node *DSFundef (node *arg_node, info *arg_info);
extern node *DSReturn (node *arg_node, info *arg_info);
extern node *DSBlock (node *arg_node, info *arg_info);
extern node *DSArg (node *arg_node, info *arg_info);
extern node *DSVardec (node *arg_node, info *arg_info);
extern node *DSId (node *arg_node, info *arg_info);
extern node *DSLet (node *arg_node, info *arg_info);
extern node *DSNWithid (node *arg_node, info *arg_info);

#endif /* _DESERIALIZE_H */
