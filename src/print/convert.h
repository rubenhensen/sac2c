/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:40:22  sacbase
 * new release made
 *
 * Revision 1.10  1998/06/03 14:32:41  cg
 * implementation streamlined
 *
 * Revision 1.9  1998/04/25 15:56:55  sbs
 * tree.h included!
 *
 * Revision 1.8  1996/02/06 16:10:20  sbs
 * Double2String and Float2String inserted.
 *
 * Revision 1.7  1995/06/30  11:58:25  hw
 * mmoved macro MOD to tree.h
 *
 * Revision 1.6  1995/06/23  12:32:27  hw
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

#ifndef _convert_h

#define _convert_h

#include "types.h"

extern char *type_string[];

extern char *Type2String (types *, int);
extern char *Double2String (double);
extern char *Float2String (float);

#endif /* _convert_h */
