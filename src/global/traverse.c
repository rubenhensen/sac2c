/*
 *
 * $Log$
 * Revision 1.9  1995/01/16 10:54:50  asi
 * added opt3_tab for loop independent removal
 * and free_tree for deletion of a syntax(sub)-tree
 *
 * Revision 1.8  1995/01/11  13:25:14  sbs
 * traverse.c:145: warning: unsigned value >= 0 is always 1 fixed
 *
 * Revision 1.7  1995/01/02  16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.6  1994/12/31  14:09:00  sbs
 * DBUG_ASSERT inserted checking the range of node-types!
 *
 * Revision 1.5  1994/12/16  14:22:10  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.4  1994/12/09  10:13:19  sbs
 * optimize inserted
 *
 * Revision 1.3  1994/12/01  17:41:40  hw
 * added funptr type_tab[]
 * changed parameters of NIF
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "optimize.h"
#include "import.h"

#include "traverse.h"

funptr *act_tab;

/*
**
**  global definitions of all funtabs needed for Trav
**
**  1) flat_tab
**
*/

#define NIF(n, s, i, f, p, t, o, x, y, z) f

funptr flat_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
**  2) print_tab
*/

#define NIF(n, s, i, f, p, t, o, x, y, z) p

funptr print_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 3) type_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) t

funptr type_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 4) opt1_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) o

funptr opt1_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 5) imp_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) i

funptr imp_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 6) opt2_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) x

funptr opt2_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 7) opt3_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) y

funptr opt3_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 8) free_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) z

funptr free_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
**
**  functionname  : Trav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : higher order traverse function; selects particular
**                  function from the globaly available funtab
**                  pointed to by "act_tab" with respect to the
**                  actual node-type.
**  global vars   : act_tab
**  internal funs : ---
**  external funs : all funs from flatten.c
**
**  remarks       : could be done as macro as well!!
**
*/

node *
Trav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("Trav");
    DBUG_ASSERT ((NULL != arg_node), "wrong argument:NULL pointer");
    DBUG_ASSERT ((arg_node->nodetype <= N_ok), "wrong argument: Type-tag out of range!");
    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x number of nodes: %d",
                         mdb_nodetype[arg_node->nodetype], arg_node, arg_node->nnode));

    DBUG_RETURN ((*act_tab[arg_node->nodetype]) (arg_node, arg_info));
}

/*
**  dummy function for funtab entries not yet done
**  recursively invoces Trav on all subnodes
*/

node *
DummyFun (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("DummyFun");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    DBUG_RETURN (arg_node);
}
