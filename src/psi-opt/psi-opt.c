/*
 *
 * $Log$
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
    DBUG_ENTER ("PsiOpt");

    NOTE (("Optimizing arrays: ...\n"));

    if (opt_ive) {
        ive_expr = 0;
        ive_op = 0;

        act_tab = idx_tab;
        arg_node = Trav (arg_node, NULL);

        NOTE (("  %d index-vector(s) eliminated", ive_expr));
        NOTE (("  %d index-vector-operation(s) eliminated", ive_op));
    }

    DBUG_RETURN (arg_node);
}
