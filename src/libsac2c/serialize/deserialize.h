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
extern void DSimportInstancesByName (const char *name, const char *module);
extern void DSimportTypedefByName (const char *name, const char *module);
extern void DSimportObjdefByName (const char *name, const char *module);
extern node *DSloadFunctionBody (node *fundef);

extern void DSaddAliasing (const char *symbol, node *target);
extern void DSremoveAliasing (const char *symbol);

/*
 * hooks for deserialization
 */
extern usertype DSloadUserType (const char *symid, const namespace_t *ns);
extern node *DSlookupFunction (const char *module, const char *symbol);
extern node *DSlookupObject (const char *module, const char *symbol);
extern node *DSfetchArgAvis (int pos);

/*
 * deserialize helpers
 */
extern double DShex2Double (const char *string);
extern float DShex2Float (const char *string);

#endif /* _SAC_DESERIALIZE_H_ */
