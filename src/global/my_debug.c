/*
 *
 * $Log$
 * Revision 1.7  1994/12/21 11:33:29  hw
 * added char *mdb_type[]
 *
 * Revision 1.6  1994/12/16  14:20:59  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.5  1994/12/05  13:02:03  hw
 * removed char *type_string[]
 * included "convert.h" to have char *type_string[]
 * included "tree.h" to have no problem with including "convert.h"
 *
 * Revision 1.4  1994/12/02  11:08:54  hw
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

#ifndef DBUG_OFF     /* All this stuff is only used in connection with DBUG */
#include "tree.h"    /* to have TYPE types */
#include "convert.h" /* to have type_string[] */

/*
** global array used for DBUG purposes only
*/

#define NIF(n, s, i, f, p, t, o, x, y, z) s

char *mdb_nodetype[] = {
#include "node_info.mac"
};

#undef NIF

#define PRF_IF(n, s, x) s

char *mdb_prf[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

#define TYP_IF(n, s) s

char *mdb_type[] = {
#include "type_info.mac"
};

#undef TYP_IF

#endif /* DBUG_OFF */
