/*
 * $Log$
 * Revision 3.6  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.5  2004/11/27 02:12:28  sah
 * ...
 *
 * Revision 3.4  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 3.3  2004/11/17 19:46:19  sah
 * changed arguments from char to const char
 *
 * Revision 3.2  2002/10/18 14:29:31  sbs
 * made the type definition node part of the repository record
 *
 * Revision 3.1  2000/11/20 18:00:23  sacbase
 * new release made
 *
 * Revision 1.1  1999/10/12 15:38:15  sbs
 * Initial revision
 *
 *
 */

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
                               int lineno, node *tdef);
extern usertype UTfindUserType (const char *name, const namespace_t *ns);

extern int UTgetNumberOfUserTypes ();

extern const namespace_t *UTgetNamespace (usertype t1);
extern char *UTgetName (usertype t1);
extern ntype *UTgetTypedef (usertype t1);
extern ntype *UTgetBaseType (usertype t1);
extern int UTgetLine (usertype t1);
extern node *UTgetTdef (usertype t1);

extern void UTsetTypedef (usertype t1, ntype *type);
extern void UTsetBaseType (usertype t1, ntype *type);
extern void UTsetName (usertype t1, const char *name);
extern void UTsetNamespace (usertype t1, const namespace_t *ns);

extern void UTprintRepository (FILE *outfile);

#endif /* _SAC_USER_TYPES_H_ */
