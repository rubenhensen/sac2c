/*
 *
 * $Log$
 * Revision 1.6  1995/07/10 16:20:23  asi
 * added IDS_DEF
 *
 * Revision 1.5  1995/07/04  16:36:24  asi
 * added macros - IDS_VARNO, ARG1 ...
 *
 * Revision 1.4  1995/04/11  15:10:24  hw
 * added macro IDS_NEXT
 *
 * Revision 1.3  1995/03/31  15:46:33  hw
 * added macro IDS_REFCNT
 *
 * Revision 1.2  1995/03/29  11:52:22  hw
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
#define IDS_DEF IDS->def
#define IDS_VARNO IDS->node->varno
#define IDS_REFCNT IDS->refcnt
#define IDS_NEXT IDS->next

/* macros for access arguments of a ap or prf - node */
#define ARG1 node[0]->node[0]
#define ARG2 node[0]->node[1]->node[0]
#define ARG3 node[0]->node[1]->node[1]->node[0]

#endif /* _access_macros_h */
