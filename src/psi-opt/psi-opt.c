/*
 *
 * $Log$
 * Revision 1.3  1995/10/05 14:55:52  sbs
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
#include "optimize.h"

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
    if (psi_optimize && optimize) {
        NOTE (("Optimizing arrays: ...\n"));
        if (psi_opt_ive) {
            act_tab = idx_tab;
            arg_node = Trav (arg_node, NULL);
        }
    }
    DBUG_RETURN (arg_node);
}
