/*
 *
 * $Log$
 * Revision 1.24  1998/06/18 13:40:07  cg
 * macro NIF used in node_info.mac enlarged,
 * new traversal function tables added.
 *
 * Revision 1.23  1998/04/29 17:11:50  dkr
 * changed usage of NIF
 *
 * Revision 1.22  1998/04/23 18:57:34  dkr
 * changed usage of NIF
 *
 * Revision 1.21  1998/04/02 16:06:40  dkr
 * changed signature of NIF
 *
 * Revision 1.20  1997/11/05 09:33:26  dkr
 * usage of NIF-macro has changed
 *
 * Revision 1.19  1996/01/16 16:45:45  cg
 * extended macro TYP_IF to 5 positions
 *
 * Revision 1.18  1996/01/02  15:48:32  cg
 * macro NIF extended.
 *
 * Revision 1.17  1995/11/16  19:37:21  cg
 * NIF macro extended by 4 new parameters.
 *
 * Revision 1.16  1995/10/20  09:24:17  cg
 * added 4 new items for macro NIF
 *
 * Revision 1.15  1995/08/03  14:51:40  cg
 * Macro NIF adjusted to 26 parameters.
 *
 * Revision 1.14  1995/07/07  14:27:54  hw
 * enlarged macro PRF_IF( there are 4 args now)
 *
 * Revision 1.13  1995/06/23  12:18:07  hw
 * enlarged macro TYP_IF
 *
 * Revision 1.12  1995/06/02  12:13:08  sbs
 * NIF macro prolongated
 *
 * Revision 1.11  1995/04/11  15:57:47  asi
 * NIF macro enlarged
 *
 * Revision 1.10  1995/01/31  14:59:33  asi
 * opt4_tab inserted and NIF macro enlarged
 *
 * Revision 1.9  1995/01/05  12:37:02  sbs
 * third component for type_info.mac inserted
 *
 * Revision 1.8  1994/12/30  16:57:01  sbs
 * commented out #ifndef DBUG_OFF
 *
 * Revision 1.7  1994/12/21  11:33:29  hw
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

/* #ifndef DBUG_OFF    All this stuff is only used in connection with DBUG */
#if 0
#include "tree.h"    /* to have TYPE types */
#include "convert.h" /* to have type_string[] */
#endif

/*
** global array used for DBUG purposes only
*/

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, nn)                                                                \
    s

char *mdb_nodetype[] = {
#include "node_info.mac"
};
#undef NIF

#define PRF_IF(n, s, x, y) s

char *mdb_prf[] = {
#include "prf_node_info.mac"
};
#undef PRF_IF

#define TYP_IF(n, s, p, f, sz) s

char *mdb_type[] = {
#include "type_info.mac"
};
#undef TYP_IF

/* #endif  DBUG_OFF */
