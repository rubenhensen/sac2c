/*
 *
 * $Log$
 * Revision 1.8  2004/11/23 21:20:48  sah
 * *** empty log message ***
 *
 * Revision 1.7  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 1.6  2004/11/17 19:50:04  sah
 * interface changes
 *
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
