/*
 *
 * $Log$
 * Revision 1.8  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.7  2005/06/28 16:23:57  sah
 * cleanup
 *
 * Revision 1.6  2005/06/18 18:07:51  sah
 * added DSdispatchFunCall
 *
 * Revision 1.5  2005/05/25 20:26:35  sah
 * FUNDEF_EXT_ASSIGN is restored now
 * during deserialisation
 *
 * Revision 1.4  2005/05/22 19:45:53  sah
 * added first implementation steps for import
 *
 * Revision 1.3  2005/05/19 11:10:44  sah
 * added special import mode
 *
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

/*
 * init/finish functions
 */
extern void DSinitDeserialize (node *module);
extern void DSfinishDeserialize (node *module);

/*
 * functions used by entire compiler
 */
extern node *DSdispatchFunCall (const namespace_t *ns, const char *name, node *args);

/*
 * functions used by module system
 */
extern node *DSaddSymbolByName (const char *symbol, stentrytype_t type,
                                const char *module);
extern node *DSaddSymbolById (const char *symbid, const char *module);
extern void DSimportInstancesByName (const char *name, const char *module);

/*
 * hooks for deserialization
 */
extern ntype *DSloadUserType (const char *name, const namespace_t *ns);
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
extern node *DSassign (node *arg_node, info *arg_info);
extern node *DSap (node *arg_node, info *arg_info);

#endif /* _SAC_DESERIALIZE_H_ */
