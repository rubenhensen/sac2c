/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef DBUG_OFF /* All this stuff is only used in connection with DBUG */

#ifndef _my_debug_h

#define _my_debug_h

/*
** global array used for DBUG purposes only
*/

#define NIF(n, s, x, y, z) s

char *mdb_nodetype[] = {
#include "node_info.mac"
};

#undef NIF

#define PRF_IF(n, s, x) s

char *mdb_prf[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

#endif /* _my_debug_h */

#endif /* DBUG_OFF */
