/*
 *
 * $Log$
 * Revision 1.4  1994/12/02 11:08:54  hw
 * inserted  char *type_string[]..
 * deleted  some preprosessor statemenmts
 *
 * Revision 1.3  1994/12/01  17:38:47  hw
 * changed parameters of NIF
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef DBUG_OFF /* All this stuff is only used in connection with DBUG */

/*
** global array used for DBUG purposes only
*/

#define NIF(n, s, f, p, t, z) s

char *mdb_nodetype[] = {
#include "node_info.mac"
};

#undef NIF

#define PRF_IF(n, s, x) s

char *mdb_prf[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

char *type_string[] = {"int", "float", "bool"}; /* used for DBUG_PRINT */

#endif /* DBUG_OFF */
