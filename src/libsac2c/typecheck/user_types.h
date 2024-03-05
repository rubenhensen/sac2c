#ifndef _SAC_USER_TYPES_H_
#define _SAC_USER_TYPES_H_

/*
 * The module "user_type" implements a repository for user defined types.
 * This file describes the interface to that module.
 *
 * The repository keeps entries of the following kind:
 *
 *  udt# | type-name | type-module | defining type | base type | line#
 *
 * All interfacing to that repository has to be made through the functions
 * defined in this module!
 *
 */

#include <stdio.h>
#include "types.h"

/*
 * special return value for "UTFindUserType"!
 * It indicates, that the repository does not contain an entry of the
 * given name.
 */
#define UT_NOT_DEFINED -1

extern usertype UTaddUserType (char *name, namespace_t *ns, ntype *type, ntype *base,
                               size_t lineno, node *tdef, bool nested, bool external);
extern usertype UTaddAlias (char *name, namespace_t *ns, usertype alias, size_t lineno,
                            node *tdef);
extern usertype UTfindUserType (const char *name, const namespace_t *ns);

extern int UTgetNumberOfUserTypes (void);

extern const namespace_t *UTgetNamespace (usertype t1);
extern char *UTgetName (usertype t1);
extern ntype *UTgetTypedef (usertype t1);
extern ntype *UTgetBaseType (usertype t1);
extern size_t UTgetLine (usertype t1);
extern node *UTgetTdef (usertype t1);
extern usertype UTgetAlias (usertype udt);
extern usertype UTgetUnAliasedType (usertype udt);

extern void UTsetTypedef (usertype t1, ntype *type);
extern void UTsetBaseType (usertype t1, ntype *type);
extern void UTsetName (usertype t1, const char *name);
extern void UTsetNamespace (usertype t1, namespace_t *ns);

extern bool UTeq (usertype udt1, usertype udt2);
extern bool UTisAlias (usertype udt);

extern bool UTisNested (usertype udt);
extern bool UTisExternal (usertype udt);
extern bool UTisStruct (usertype udt);

extern void UTprintRepository (FILE *outfile);

#endif /* _SAC_USER_TYPES_H_ */
