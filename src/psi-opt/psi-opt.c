/*
 *
 * $Log$
 * Revision 2.4  2000/01/26 17:26:31  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.3  1999/07/14 12:10:47  sbs
 * N_info node allocated for IVE!
 *
 * Revision 2.2  1999/05/12 14:35:16  cg
 * Optimizations are now triggered by bit field optimize instead
 * of single individual int variables.
 *
 * Revision 2.1  1999/02/23 12:43:15  sacbase
 * new release made
 *
 * Revision 1.6  1998/02/25 09:22:16  cg
 * adjusted to new set of global variable in globals.[ch]
 *
 * Revision 1.5  1996/02/13 10:15:11  sbs
 * counting of eliminations inserted.
 *
 * Revision 1.4  1996/01/17  14:19:11  asi
 * added globals.h and removed optimize.h
 *
 * Revision 1.3  1995/10/05  14:55:52  sbs
 * some bug fixes.
 *
 * Revision 1.2  1995/06/06  15:19:13  sbs
 * first usable version ; does not include conditional stuff
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "psi-opt.h"
#include "globals.h"

int ive_expr, ive_op;

/*
 *
 *  functionname  : PsiOpt
 *  arguments     : 1) ptr to root of the syntaxtree
 *                  R) ptr to root of the optimized syntaxtree
 *  description   : Optimizes the intermediate sac-code at psi level
 *  global vars   : syntax_tree, opt_cf, opt_dcr
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       : Optimizations supported: - index
 *
 */
node *
PsiOpt (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("PsiOpt");

    NOTE (("Optimizing arrays: ...\n"));

    if (optimize & OPT_IVE) {
        ive_expr = 0;
        ive_op = 0;

        tmp_tab = act_tab;
        act_tab = idx_tab;

        info_node = MakeNode (N_info);
        arg_node = Trav (arg_node, info_node);

        FreeNode (info_node);
        act_tab = tmp_tab;

        NOTE (("  %d index-vector(s) eliminated", ive_expr));
        NOTE (("  %d index-vector-operation(s) eliminated", ive_op));
    }

    DBUG_RETURN (arg_node);
}
