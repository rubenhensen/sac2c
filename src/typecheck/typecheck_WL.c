/*      $Id$
 * $Log$
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

    arg_node = Trav (arg_node, infon);

    act_tab = old_tab;

    FreeTree (infon);

    DBUG_RETURN (arg_node);
}
