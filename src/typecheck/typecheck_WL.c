/*
 *
 * $Log$
 * Revision 3.8  2004/07/14 23:25:53  sah
 * inlined some code from ConstantFolding.c as the old
 * constant folding has been removed. this is far from
 * beeing perfect, but hopefully the old typechecker will
 * be removed soon.
 *
 * Revision 3.7  2001/11/19 20:35:07  dkr
 * TI() renamed into TypeInference() in order to avoid linker warning
 *
 * Revision 3.6  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.5  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.4  2001/03/22 20:41:10  dkr
 * no changes done
 *
 * Revision 3.3  2001/03/05 17:02:39  sbs
 * switched from CF to CO for prf_add_AxA.
 *
 * Revision 3.2  2001/02/23 18:04:22  sbs
 * extended for negative take's and drop's in genarray
 *
 * Revision 3.1  2000/11/20 18:00:22  sacbase
 * new release made
 *
 * Revision 2.10  2000/05/11 10:38:28  dkr
 * Signature of ReduceGenarrayShape modified.
 * Bug in ReduceGenarrayShape fixed: arg_info is no longer dumped.
 *
 * Revision 2.9  2000/05/03 16:49:36  dkr
 * COFreeConstant returns NULL now
 *
 * Revision 2.8  2000/01/26 17:28:16  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.6  1999/11/11 20:05:32  dkr
 * signature and name of function IsConstantArray changed
 *
 * Revision 2.5  1999/10/22 14:14:40  sbs
 * now uses take, drop and friends from constants.c
 *
 * Revision 2.4  1999/10/19 12:55:26  sacbase
 * TCWLprf changed; now psi-applications are computed using constants!
 *
 * Revision 2.3  1999/07/15 20:40:44  sbs
 * in ReduceShape we now make sure that an appropriate CONSTVEC info will be
 * created.
 *
 * Revision 2.2  1999/05/31 16:56:45  sbs
 * constant-folding for wls extended
 * Now, with(...) genarray( [2+2,3],...);
 * works as well.
 *
 * Revision 2.1  1999/02/23 12:41:00  sacbase
 * new release made
 *
 * Revision 1.1  1998/04/28 15:48:09  srs
 * Initial revision
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "typecheck.h"
#include "DupTree.h"

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
        EXPRS_EXPR (tmpn) = Trav (EXPRS_EXPR (tmpn), arg_info);
        if (N_num != NODE_TYPE (EXPRS_EXPR (tmpn)))
            expr_ok = 0;
        tmpn = EXPRS_NEXT (tmpn);
    }

    ARRAY_TYPE (arg_node) = TI_array (arg_node, arg_info);

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
    constant *arg1, *arg2, *res;
    types *expr_type;

    DBUG_ENTER ("TCWLprf");

    if (F_shape == PRF_PRF (arg_node)) {
        if (N_id == NODE_TYPE (PRF_ARG1 (arg_node))) {
            /* constantfold prf shape() now. This code has been inlined
               from the old non-ssa constant folder. */
            DBUG_PRINT ("TYPE",
                        ("primitive function %s folded", mdb_prf[arg_node->info.prf]));
            node *tmp
              = Types2Array (ID_TYPE (PRF_ARG1 (arg_node)), INFO_CF_TYPE (arg_info));
            if (tmp != NULL) {
                /* Types2Array was successful */
                ARRAY_VECTYPE (tmp) = T_int;
                FreeTree (arg_node);
                arg_node = tmp;
            } else {
                /* not successful */
                expr_ok = 0;
            }
        } else {
            PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);

            if (PRF_ARG1 (arg_node)) {
                if (N_array == NODE_TYPE (PRF_ARG1 (arg_node))) {
                    /* this code has been inline from the old non-ssa constant folder
                       as it has been removed */
                    DBUG_PRINT ("TYPE", ("primitive function %s folded",
                                         mdb_prf[arg_node->info.prf]));

                    node *array = PRF_ARG1 (arg_node);

                    /* count number of elements */
                    int noofelems = CountExprs (ARRAY_AELEMS (array));

                    /* store result in this array (it is reused as shape) */
                    NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (array))) = noofelems;
                    NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (array))) = N_num;

                    /* free rest of array and prf node */
                    if (EXPRS_NEXT (ARRAY_AELEMS (array)) != NULL) {
                        EXPRS_NEXT (ARRAY_AELEMS (array))
                          = FreeTree (EXPRS_NEXT (ARRAY_AELEMS (array)));
                    }

                    /* give new array correct type */
                    ARRAY_TYPE (array) = FreeAllTypes (ARRAY_TYPE (array));
                    ARRAY_TYPE (array) = DupAllTypes (INFO_CF_TYPE (arg_info));
                    ARRAY_VECLEN (array) = 1;
                    ((int *)ARRAY_CONSTVEC (array))
                      = Array2IntVec (ARRAY_AELEMS (array), NULL);
                    ARRAY_VECTYPE (array) = T_int;

                    /* store result */
                    FreeNode (arg_node);
                    arg_node = array;
                } else if (N_num == NODE_TYPE (PRF_ARG1 (arg_node))) {
                    /* this code has been inlined from the old non-ssa
                       constant folder as it has been removed */

                    /* sah: I have not found any code dealing with
                       shape applied to a num, there has to be a reason
                       for this (isn't it always []?) */
                }

                if (N_prf == NODE_TYPE (arg_node)) /* not successful */
                    expr_ok = 0;
            }
        }
    } else {
        expr_type = TypeInference (PRF_ARG1 (arg_node), arg_info);
        INFO_CF_TYPE (arg_info) = expr_type;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        expr_type = TypeInference (PRF_ARG2 (arg_node), arg_info);
        INFO_CF_TYPE (arg_info) = expr_type;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);

        /* both arguments have to be a constant or an array. */
        if (PRF_ARG1 (arg_node) && PRF_ARG2 (arg_node)
            && (N_array == NODE_TYPE (PRF_ARG1 (arg_node))
                || N_num == NODE_TYPE (PRF_ARG1 (arg_node)))
            && (N_array == NODE_TYPE (PRF_ARG2 (arg_node))
                || N_num == NODE_TYPE (PRF_ARG2 (arg_node)))) {
            /* CF prf now. */
            if ((PRF_PRF (arg_node) == F_sel) || (PRF_PRF (arg_node) == F_reshape)
                || (PRF_PRF (arg_node) == F_take) || (PRF_PRF (arg_node) == F_drop)
                || (PRF_PRF (arg_node) == F_add_AxA)) {
                if (IsConstArray (PRF_ARG1 (arg_node))
                    && IsConstArray (PRF_ARG2 (arg_node))) {
                    arg1 = COMakeConstantFromArray (PRF_ARG1 (arg_node));
                    arg2 = COMakeConstantFromArray (PRF_ARG2 (arg_node));
                    if (PRF_PRF (arg_node) == F_sel) {
                        res = COSel (arg1, arg2);
                    } else if (PRF_PRF (arg_node) == F_reshape) {
                        res = COReshape (arg1, arg2);
                    } else if (PRF_PRF (arg_node) == F_take) {
                        res = COTake (arg1, arg2);
                    } else if (PRF_PRF (arg_node) == F_drop) {
                        res = CODrop (arg1, arg2);
                    } else {
                        res = COAdd (arg1, arg2);
                    }
                    arg1 = COFreeConstant (arg1);
                    arg2 = COFreeConstant (arg2);
                    arg_node = FreeTree (arg_node);
                    arg_node = COConstant2AST (res);
                    res = COFreeConstant (res);
                }
            } else {
                /* This is a direct call to CFprf. This may start a complete
                   traversal in constant folding mode, without setting the
                   appropriate cf_tab. I have no idea why this shall work?!?
                   Thus, i cannot inline the code here */
                /*
                   arg_node = CFprf(arg_node, arg_info);
                */
                if (N_prf == NODE_TYPE (arg_node)) /* not successful */
                    expr_ok = 0;
            }
        }
    }

    if ((NODE_TYPE (arg_node) == N_array) && (ARRAY_TYPE (arg_node) == NULL)) {
        ARRAY_TYPE (arg_node) = TI_array (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReduceGenarrayShape(node *arg_node, types *expr_type)
 *
 * description:
 *   CFs the given expression (see file description) and returns the
 *   folded result (the old expression has been set free) or returnes
 *   NULL if folding is not possible.
 *
 ******************************************************************************/

node *
ReduceGenarrayShape (node *arg_node, node *arg_info, types *expr_type)
/*
 * dkr: parameter 'arg_info' added (see remark below)
 */
{
    funtab *old_tab;
    node *infon;

    DBUG_ENTER ("ReduceGenarrayShape");

    old_tab = act_tab;
    act_tab = tcwl_tab;

    /*
     * remark (dkr):
     *
     * It is not a good idea to create a new (empty) arg_info-node here,
     * I'm afraid. All the arg_info-data is lost now although it is
     * obviously needed during the rest of the traversal!!
     * Why don't we hand over ReduceGenarrayShape() the current arg_info???
     * Possibly this will overwrite some arg_info-data still needed
     * after finishing the call of ReduceGenarrayShape() :-((
     * Therefore it is better just to copy the arg_info-data here ...
     */

    infon = MakeInfo ();
    infon->node[0] = MakeOk (); /* needed for INFO_TC_... macros */
    INFO_TC_STATUS (infon) = INFO_TC_STATUS (arg_info);
    INFO_TC_VARDEC (infon) = INFO_TC_VARDEC (arg_info);
    INFO_TC_FUNDEF (infon) = INFO_TC_FUNDEF (arg_info);
    INFO_CF_TYPE (infon) = expr_type;

    expr_ok = 1;
    arg_node = Trav (arg_node, infon);
    if (!expr_ok) {
        arg_node = NULL;
    } else {
        /*
         * The arg_node is a constant integer vector, so we have to attribute
         * the CONSTVEC form!!
         */

        ARRAY_ISCONST (arg_node) = TRUE;
        ARRAY_VECTYPE (arg_node) = T_int;
        ARRAY_CONSTVEC (arg_node)
          = Array2IntVec (ARRAY_AELEMS (arg_node), &ARRAY_VECLEN (arg_node));
    }

    act_tab = old_tab;
    infon = FreeTree (infon);

    DBUG_RETURN (arg_node);
}
