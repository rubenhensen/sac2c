/*
 *
 * $Log$
 * Revision 1.6  1995/06/23 12:32:27  hw
 * macro SIMPLE4FUN_RENAME inserted
 * "extern char *rename_type[];" inserted
 *
 * Revision 1.5  1995/01/06  19:25:20  sbs
 * "__" inserted between modul and function name
 *
 * Revision 1.4  1995/01/05  11:51:25  sbs
 * MOD_NAME_CON macro inserted for mod-name generation for
 * types and functions.
 *
 * Revision 1.3  1994/12/14  16:35:39  sbs
 * userdef types integrated
 *
 * Revision 1.2  1994/12/05  13:26:31  hw
 * *** empty log message ***
 *
 *
 */

/* NOTE: you must include "tree.h before this !!!
 */

#ifndef _convert_h

#define _convert_h

#define SIMPLE2STR(type)                                                                 \
    ((type->name != NULL) ? type->name : type_string[type->simpletype])
#define SIMPLE4FUN_RENAME(type)                                                          \
    ((type->name != NULL) ? type->name : rename_type[type->simpletype])
#define MOD_NAME_CON "__"

extern char *type_string[];
extern char *rename_type[];

extern char *Type2String (types *, int);

#endif /* _convert_h */
