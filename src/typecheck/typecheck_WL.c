/*      $Id$
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:00  sacbase
 * new release made
 *
 * Revision 1.2  1998/04/29 08:51:47  srs
 * *** empty log message ***
 *
 * Revision 1.1  1998/04/28 15:48:09  srs
 * Initial revision
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "tree.h"
#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "free.h"
#include "internal_lib.h"
#include "globals.h"
#include "ConstantFolding.h"

/*
 * This files exports a function ReduceGenarrayShape() which tries to
 * constantfold the expression in the genarray operator.
 * Foldable expressions only contain
 *  - shape(Id)   (not shape(Id*2))
 *  - prf +,-,*,/ with only shape() or constants as arguments.
 * Examples for valid expressions:
 * 1) shape(A)*2+3;
 * 2) (shape(A)+2)*(shape(A)*2)
 *
 * Why is CF restricted that way?
 * CF does not need masks (USE, DEF, MRD) for these expressions except
 * for the Id inside shape(). The shape()-evaluation is patched so it can be
 * called from TC.
 *
 */

/* To avaoid conflicts with usage of arg_info in CF we better use
   a global var to store our result*/
int expr_ok;

/******************************************************************************
 *
 * function:
 *   node *TCWLnull(node *arg_node, node *arg_info)
 *
 * description:
 *   This is an invalid expression and NULL is returned.
 *
 *
 ******************************************************************************/

node *
TCWLnull (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCWLnull");
    expr_ok = 0;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCWLarray(node *arg_node, node *arg_info)
 *
 * description:
 *   if an array appears, it has to be constant.
 *
 *
 ******************************************************************************/

node *
TCWLarray (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("TCWLarray");

    tmpn = ARRAY_AELEMS (arg_node);
    while (tmpn) {
        if (N_num != NODE_TYPE (EXPRS_EXPR (tmpn)))
            expr_ok = 0;
        tmpn = EXPRS_NEXT (tmpn);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCWLprf(node *arg_node, node *arg_info)
 *
 * description:
 *   Check if all arguments are constants (maybe fold them), then fold
 *   this prf.
 *
 ******************************************************************************/

node *
TCWLprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCWLprf");

    if (F_shape == PRF_PRF (arg_node)) {
        if (N_id == NODE_TYPE (PRF_ARG1 (arg_node))) {
            /* constantfold prf shape() now. We can be sure that no other
               CF-functions are called (else there would be more problems
               with masks) and therefor do not have to chance act_tab. */
            arg_node = CFprf (arg_node, arg_info);
            if (N_prf == NODE_TYPE (arg_node)) /* not successful */
                expr_ok = 0;
        } else
            expr_ok = 0;
    } else {
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);

        /* both arguments have to be a constant or an array. */
        if (PRF_ARG1 (arg_node) && PRF_ARG2 (arg_node)
            && (N_array == NODE_TYPE (PRF_ARG1 (arg_node))
                || N_num == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (N_array == NODE_TYPE (PRF_ARG2 (arg_node))
                || N_num == NODE_TYPE (PRF_ARG2 (arg_node)))) {
            /* CF prf now. */
            arg_node = CFprf (arg_node, arg_info);
            if (N_prf == NODE_TYPE (arg_node)) /* not successful */
                expr_ok = 0;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReduceGenarrayShape(node *arg_node)
 *
 * description:
 *   CFs the given expression (see file description) and returns the
 *   folded result (the old expression has been set free) or returnes
 *   NULL if folding is not possible.
 *
 ******************************************************************************/

node *
ReduceGenarrayShape (node *arg_node, types *expr_type)
{
    funptr *old_tab;
    node *infon;

    DBUG_ENTER ("ReduceGenarrayShape");

    old_tab = act_tab;
    act_tab = tcwl_tab;

    infon = MakeInfo ();
    INFO_CF_TYPE (infon) = expr_type;

    expr_ok = 1;
    arg_node = Trav (arg_node, infon);
    if (!expr_ok)
        arg_node = NULL;

    act_tab = old_tab;

    FreeTree (infon);

    DBUG_RETURN (arg_node);
}
