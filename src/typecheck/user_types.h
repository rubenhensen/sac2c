/*
 * $Log$
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

#ifndef _user_types_h
#define _user_types_h

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
#include "types.h" /* needed for usertype! */

#include "new_types.h"

/*
 * special return value for "UTFindUserType"!
 * It indicates, that the repository does not contain an entry of the
 * given name.
 */
#define UT_NOT_DEFINED -1

extern usertype UTAddUserType (char *name, char *mod, ntype *type, ntype *base,
                               int lineno, node *tdef);
extern usertype UTFindUserType (char *name, char *mod);

extern int UTGetNumberOfUserTypes ();

extern char *UTGetMod (usertype t1);
extern char *UTGetName (usertype t1);
extern ntype *UTGetTypedef (usertype t1);
extern ntype *UTGetBaseType (usertype t1);
extern int UTGetLine (usertype t1);
extern node *UTGetTdef (usertype t1);

extern void UTSetTypedef (usertype t1, ntype *type);
extern void UTSetBaseType (usertype t1, ntype *type);

extern void UTPrintRepository (FILE *outfile);

#endif /* _user_types_h */
