/*
 *
 * $Log$
 * Revision 1.19  1995/04/07 10:16:26  hw
 * added function NoTrav
 *
 * Revision 1.18  1995/04/05  15:52:38  asi
 * loop invariant removal added
 *
 * Revision 1.17  1995/04/04  12:21:56  asi
 * added include files WorkReduction.h
 *
 * Revision 1.16  1995/03/29  12:00:31  hw
 * comp_tab inserted
 *
 * Revision 1.15  1995/03/17  17:41:48  asi
 * added work reduction
 *
 * Revision 1.14  1995/03/10  10:45:25  hw
 * refcnt_tab inserted
 *
 * Revision 1.13  1995/02/13  17:22:28  asi
 * added include files ConstantFolding.h and DeadCodeRemoval.h
 *
 * Revision 1.12  1995/02/07  10:59:23  asi
 * renamed opt1_tab -> opt_tab, opt2_tab -> dead_tab, opt3_tab -> lir.tab and
 * added functionlist cf_tab for constant folding
 *
 * Revision 1.11  1995/01/31  14:59:33  asi
 * opt4_tab inserted and NIF macro enlarged
 *
 * Revision 1.10  1995/01/18  17:37:16  asi
 * added include free.h
 *
 * Revision 1.9  1995/01/16  10:54:50  asi
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
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "optimize.h"
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "WorkReduction.h"
#include "LoopInvariantRemoval.h"
#include "import.h"
#include "refcount.h"
#include "compile.h"

#include "traverse.h"

funptr *act_tab;

/*
**
**  global definitions of all funtabs needed for Trav
**
**  1) flat_tab
**
*/

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) f

funptr flat_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
**  2) print_tab
*/

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) p

funptr print_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 3) type_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) t

funptr type_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 4) opt_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) o

funptr opt_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 5) imp_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) i

funptr imp_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 6) dead_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) x

funptr dead_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 7) wr_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) y

funptr wr_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 8) free_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) z

funptr free_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 9) cf_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) a

funptr cf_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 10) refcnt_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) b

funptr refcnt_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 11) comp_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) c

funptr comp_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
 * 12) lir_tab
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) d

funptr lir_tab[] = {
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

/*
 * dummy function for funtab entries where noting is to do
 *
 */
node *
NoTrav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DummyNoOp");
    DBUG_RETURN (arg_node);
}
