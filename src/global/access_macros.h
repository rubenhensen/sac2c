/*
 *
 * $Log$
 * Revision 1.2  1995/03/29 11:52:22  hw
 * macros for access to elements of struct info.ids inserted
 *
 * Revision 1.1  1995/03/28  12:01:50  hw
 * Initial revision
 *
 *
 */

#ifndef _access_macros_h

#define _access_macros_h

/* macros for access to elements of struct info.types */
#define TYPES info.types
#define SIMPLETYPE TYPES->simpletype
#define DIM TYPES->dim
#define ID TYPES->id
#define ID_MOD TYPES->id_mod
#define SHP TYPES->shpseg->shp
#define NAME TYPES->name
#define NAME_MOD TYPES->name_mod

/* macros used for N_ap nodes to get the function's name */
#define FUN_NAME info.fun_name.id
#define FUN_MOD_NAME info.fun_name.id_mod

/* macros for access to elements of struct info.ids */
#define IDS info.ids
#define IDS_ID IDS->id
#define IDS_NODE IDS->node

#endif /* _access_macros_h */
