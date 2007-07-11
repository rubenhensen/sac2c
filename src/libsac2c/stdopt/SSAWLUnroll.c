/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "node_basic.h"
#include "globals.h"
#include "free.h"
#include "ctinfo.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "new_types.h"
#include "shape.h"
#include "math_utils.h"

#include "DupTree.h"
#include "SSAWithloopFolding.h"
#include "SSAWLUnroll.h"

/*
 * INFO structure
 */
struct INFO {
    node *assign;
    node *preassign;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_FUNDEF(n) ((n)->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CreateBodyCode
 *
 * description:
 *   Duplicate the code behind the N_part node and insert index variables.
 *
 ******************************************************************************/

static node *
CreateBodyCode (node *partn, node *index)
{
    node *res, *letn, *coden;
    node *_ids;

    DBUG_ENTER ("CreateBodyCode");

    coden = PART_CODE (partn);
    if (N_empty == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (coden)))) {
        res = NULL;
    } else {
        res = DUPdoDupTree (BLOCK_INSTR (CODE_CBLOCK (coden)));
    }

    /* index vector */
    letn = TBmakeLet (DUPdoDupNode (PART_VEC (partn)), index);
    res = TBmakeAssign (letn, res);

    /* index scalars */
    _ids = PART_IDS (partn);
    index = ARRAY_AELEMS (index);

    while (_ids) {
        letn = TBmakeLet (DUPdoDupNode (_ids), DUPdoDupTree (EXPRS_EXPR (index)));
        res = TBmakeAssign (letn, res);

        index = EXPRS_NEXT (index);
        _ids = IDS_NEXT (_ids);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *ApplyModGenarray(node *assignn, node *index)
 *
 * description:
 *   Modify unrolled body code for one index element to apply the modarray
 *   or genarray op.
 *
 ******************************************************************************/

static node *
ApplyModGenarray (node *bodycode, node *index, node *partn, node *cexpr, node *array)
{
    node *exprs, *letexpr, *tmpn;

    DBUG_ENTER ("ApplyModGenarray");

    /* create prf modarray */
    tmpn = TBmakeId (IDS_AVIS (array));

    exprs = TBmakeExprs (tmpn, TBmakeExprs (TBmakeId (IDS_AVIS (PART_VEC (partn))),
                                            TBmakeExprs (DUPdoDupTree (cexpr), NULL)));

    DBUG_ASSERT (NODE_TYPE (cexpr) == N_id, "WLunroll found cexpr that is no N_id");

    /* append to body code */
    if (TUisScalar (ID_NTYPE (cexpr))) {
        letexpr = TBmakePrf (F_modarray_AxVxS, exprs);
    } else {
        letexpr = TBmakePrf (F_modarray_AxVxA, exprs);
    }

    tmpn = TBmakeAssign (TBmakeLet (DUPdoDupNode (array), letexpr), NULL);
    bodycode = TCappendAssign (bodycode, tmpn);

    DBUG_RETURN (bodycode);
}

/******************************************************************************
 *
 * function:
 *   node *ApplyFold( node *assignn, node *index)
 *
 * description:
 *   Modify unrolled body code for one index element to apply the fold op.
 *
 ******************************************************************************/

static node *
ApplyFold (node *bodycode, node *index, node *partn, node *cexpr, node *lhs)
{
    node *tmp, *letn;
    bool F_accu_found = FALSE;

    DBUG_ENTER ("ApplyFold");

    /*
     * special handling of
     *       acc = accu(iv,n);
     *       b = <body>
     *       cexpr = op( acc, b);
     * needed.
     * acc = accu(iv,n)  -> acc = LHS of current WL(;
     * append new last assignment: LHS of current WL = cexpr;
     */

    DBUG_ASSERT ((NODE_TYPE (bodycode) != N_empty), "BLOCK_INSTR is empty!");

    tmp = bodycode;
    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {
            ASSIGN_RHS (tmp) = FREEdoFreeNode (ASSIGN_RHS (tmp));
            ASSIGN_RHS (tmp) = DUPdupIdsId (lhs);
            F_accu_found = TRUE;
        }

        if (ASSIGN_NEXT (tmp) == NULL) {
            DBUG_ASSERT ((F_accu_found), "No F_accu found!");

            /* Append new assign: lhs(wl) = cexpr; */
            letn = TBmakeLet (DUPdoDupNode (lhs), DUPdoDupNode (cexpr));
            ASSIGN_NEXT (tmp) = TBmakeAssign (letn, NULL);
            tmp = ASSIGN_NEXT (tmp);
        }

        tmp = ASSIGN_NEXT (tmp);
    }

    DBUG_RETURN (bodycode);
}

/******************************************************************************
 *
 * function:
 *   node *ApplyPropagate( node *assignn, node *index)
 *
 * description:
 *   Modify unrolled body code for one index element to apply the extract op.
 *
 ******************************************************************************/

static node *
ApplyPropagate (node *bodycode, node *index, node *partn, node *withop, node *cexpr)
{
    node *letn;
    node *tmp;
    node *tmp_prev;
    bool F_prop_obj_found;

    DBUG_ENTER ("ApplyPropagate");

    DBUG_ASSERT ((NODE_TYPE (bodycode) != N_empty), "BLOCK_INSTR is empty!");

    /*
     * remove affected objects from the expression
     *
     *    a', ...  = prop_obj(iv, a, ...)
     */

    tmp = bodycode;
    tmp_prev = NULL;
    F_prop_obj_found = FALSE;

    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_prop_obj_in)) {
            node *prf_arg = PRF_ARGS (ASSIGN_RHS (tmp));
            node *lhs = ASSIGN_LHS (tmp);
            node *prf_prv = NULL;
            node *lhs_prv = NULL;

            /* skip iv */
            prf_prv = prf_arg;
            prf_arg = EXPRS_NEXT (prf_arg);

            /* replace object in lhs and prf args with new assignment below it */
            while (prf_arg != NULL) {
                if (ID_AVIS (EXPRS_EXPR (prf_arg))
                    == ID_AVIS (PROPAGATE_DEFAULT (withop))) {
                    letn = TBmakeLet (TBmakeIds (IDS_AVIS (lhs), NULL),
                                      TBmakeId (ID_AVIS (EXPRS_EXPR (prf_arg))));
                    ASSIGN_NEXT (tmp) = TBmakeAssign (letn, ASSIGN_NEXT (tmp));
                    lhs = FREEdoFreeNode (lhs);
                    if (lhs_prv != NULL) {
                        IDS_NEXT (lhs_prv) = lhs;
                    } else {
                        ASSIGN_LHS (tmp) = lhs;
                    }
                    prf_arg = FREEdoFreeNode (prf_arg);
                    EXPRS_NEXT (prf_prv) = prf_arg;
                } else {
                    prf_prv = prf_arg;
                    prf_arg = EXPRS_NEXT (prf_arg);
                    lhs_prv = lhs;
                    lhs = IDS_NEXT (lhs);
                }
            }

            /* check whether we leave an empty F_prop_obj, and if so, delete */
            if (ASSIGN_LHS (tmp) == NULL) {
                tmp = FREEdoFreeNode (tmp);
                if (tmp_prev != NULL) {
                    ASSIGN_NEXT (tmp_prev) = tmp;
                } else {
                    bodycode = tmp;
                }
                continue;
            }

            F_prop_obj_found = TRUE;
        } else if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
                   && /* If the assignment is a prim. fct. and*/
                   (PRF_PRF (ASSIGN_RHS (tmp)) == F_prop_obj_out)) { /* it is a prop_out*/

            node *prf_arg = PRF_ARGS (ASSIGN_RHS (tmp));
            node *lhs = ASSIGN_LHS (tmp);
            node *prf_prv = NULL;
            node *lhs_prv = NULL;

            /* replace object in lhs and prf args with new assignment below it*/
            while (lhs != NULL) {

                if (IDS_AVIS (lhs) == ID_AVIS (EXPRS_EXPR (cexpr))) {
                    letn = TBmakeLet (TBmakeIds (IDS_AVIS (lhs), NULL),
                                      TBmakeId (ID_AVIS (EXPRS_EXPR (prf_arg))));
                    ASSIGN_NEXT (tmp) = TBmakeAssign (letn, ASSIGN_NEXT (tmp));

                    lhs = FREEdoFreeNode (lhs);
                    if (lhs_prv != NULL) {
                        IDS_NEXT (lhs_prv) = lhs;
                    } else {
                        ASSIGN_LHS (tmp) = lhs;
                    }
                    prf_arg = FREEdoFreeNode (prf_arg);
                    if (prf_prv != NULL) {
                        EXPRS_NEXT (prf_prv) = prf_arg;
                    } else {
                        PRF_ARGS (ASSIGN_RHS (tmp)) = prf_arg;
                    }
                } else {
                    lhs_prv = lhs;
                    lhs = IDS_NEXT (lhs);
                    prf_prv = prf_arg;
                    prf_arg = EXPRS_NEXT (prf_arg);
                }
            }

            if (ASSIGN_LHS (tmp) == NULL) {
                tmp = FREEdoFreeNode (tmp);
                if (tmp_prev != NULL) {
                    ASSIGN_NEXT (tmp_prev) = tmp;
                } else {
                    bodycode = tmp;
                }
                continue;
            }

            F_prop_obj_found = TRUE;
        }
        if (ASSIGN_NEXT (tmp) == NULL) {

            /* Assign the resulting object of the body to the ingoing object
             * (which incidentally is the default element so use that). */
            letn = TBmakeLet (TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (withop)), NULL),
                              DUPdoDupTree (EXPRS_EXPR (cexpr)));
            ASSIGN_NEXT (tmp) = TBmakeAssign (letn, NULL);
            tmp = ASSIGN_NEXT (tmp);
        }

        tmp_prev = tmp;
        tmp = ASSIGN_NEXT (tmp);
    }

    DBUG_RETURN (bodycode);
}

/******************************************************************************
 *
 * function:
 *   node *ForEachElementWithop( node *bodycode, node *wln, node *partn,
 *                               node *index, info *arg_info)
 *
 * description:
 *   Applies the proper Create* function to the given unrolled body code
 *   for each withop in the with-loop.
 *
 ******************************************************************************/

static node *
ForEachElementWithop (node *bodycode, node *wln, node *partn, node *index, info *arg_info)
{
    node *withop;
    node *cexpr;
    node *lhs;

    DBUG_ENTER ("ForEachElementWithop");

    withop = WITH_WITHOP (wln);
    cexpr = CODE_CEXPRS (PART_CODE (partn));
    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    while (withop != NULL) {
        switch (NODE_TYPE (withop)) {
        case N_genarray:
            DBUG_PRINT ("WLUR", ("withop: genarray"));
            break;
        case N_modarray:
            DBUG_PRINT ("WLUR", ("withop: modarray"));
            break;
        case N_fold:
            DBUG_PRINT ("WLUR", ("withop: fold"));
            break;
        case N_break:
            DBUG_PRINT ("WLUR", ("withop: break"));
            break;
        case N_propagate:
            DBUG_PRINT ("WLUR", ("withop: propagate"));
            break;
        default:
            DBUG_ASSERT (0, "unhandled withop");
        }

        DBUG_PRINT ("WLUR", ("cexpr: %s", ID_NAME (EXPRS_EXPR (cexpr))));
        DBUG_PRINT ("WLUR", ("lhs: %s\n", IDS_NAME (lhs)));

        switch (NODE_TYPE (withop)) {
        case N_genarray:
            bodycode = ApplyModGenarray (bodycode, index, partn, EXPRS_EXPR (cexpr), lhs);
            break;
        case N_modarray:
            bodycode = ApplyModGenarray (bodycode, index, partn, EXPRS_EXPR (cexpr), lhs);
            break;
        case N_fold:
            bodycode = ApplyFold (bodycode, index, partn, EXPRS_EXPR (cexpr), lhs);
            break;
        case N_break:
            /* no-op */
            break;
        case N_propagate:
            bodycode = ApplyPropagate (bodycode, index, partn, withop, cexpr);
            break;
        default:
            DBUG_ASSERT (0, "unhandled withop");
        }
        withop = WITHOP_NEXT (withop);
        cexpr = EXPRS_NEXT (cexpr);
        lhs = IDS_NEXT (lhs);
    }

    DBUG_RETURN (bodycode);
}

/******************************************************************************
 *
 * function:
 *   node *ForEachElementHelp(...)
 *
 * description:
 *   Helper function for ForEachElement(), for multi-dimensional index vectors.
 *
 ******************************************************************************/

static node *
ForEachElementHelp (int *l, int *u, int *s, int *w, int dim, int maxdim, node *assignn,
                    node *wln, node *partn, info *arg_info)
{
    int count, act_w, i;
    static int ind[SHP_SEG_SIZE];
    node *index, *bodycode;

    DBUG_ENTER ("ForEachElementHelp");

    DBUG_ASSERT ((maxdim > 0), "illegal max. dim found!");
    count = l[dim];
    act_w = 0;
    while (count + act_w < u[dim]) {
        ind[dim] = count + act_w;
        if (dim + 1 == maxdim) {
            /* create index */
            index = NULL;
            for (i = maxdim; i > 0; i--) {
                index = TBmakeExprs (TBmakeNum (ind[i - 1]), index);
            }
            index = TCmakeIntVector (index);

            /* Create a fresh copy of the bodycode, then hand it over to
             * each of the withops to mutate. Finally, append it to the
             * assign chain. */
            bodycode = CreateBodyCode (partn, index);
            bodycode = ForEachElementWithop (bodycode, wln, partn, index, arg_info);
            assignn = TCappendAssign (assignn, bodycode);
        } else {
            assignn = ForEachElementHelp (l, u, s, w, dim + 1, maxdim, assignn, wln,
                                          partn, arg_info);
        }

        /* advance to next element */
        if (w && (act_w + 1 < w[dim])) {
            act_w++;
        } else {
            act_w = 0;
            count += s ? s[dim] : 1;
        }
    }

    DBUG_RETURN (assignn);
}

/******************************************************************************
 *
 * function:
 *   node *ForEachElement(node *assignn, node *wln, node *partn, info *arg_info)
 *
 * description:
 *   Creates a copy of the with-loop body code, and calls ForEachElementWithop
 *   on the body code for every index element of the given generator in partn.
 *
 ******************************************************************************/

static node *
ForEachElement (node *assignn, node *wln, node *partn, info *arg_info)
{
    node *index, *bodycode;
    int maxdim, *l, *u, *s, *w;

    DBUG_ENTER ("ForEachElement");

    maxdim = SHgetExtent (TYgetShape (IDS_NTYPE (PART_VEC (partn))), 0);

    l = u = s = w = NULL;

    WLFarrayST2ArrayInt (GENERATOR_BOUND1 (PART_GENERATOR (partn)), &l, maxdim);
    WLFarrayST2ArrayInt (GENERATOR_BOUND2 (PART_GENERATOR (partn)), &u, maxdim);
    if (GENERATOR_STEP (PART_GENERATOR (partn))) {
        WLFarrayST2ArrayInt (GENERATOR_STEP (PART_GENERATOR (partn)), &s, maxdim);
    }
    if (GENERATOR_WIDTH (PART_GENERATOR (partn))) {
        WLFarrayST2ArrayInt (GENERATOR_WIDTH (PART_GENERATOR (partn)), &w, maxdim);
    }

    if (maxdim == 0) {
        /* create index */
        index = TCmakeIntVector (NULL);
        /* nums struct is freed inside MakeShpseg() */

        /* Create a fresh copy of the bodycode, then hand it over to
         * each of the withops to mutate. Finally, append it to the
         * assign chain. */
        bodycode = CreateBodyCode (partn, index);
        bodycode = ForEachElementWithop (bodycode, wln, partn, index, arg_info);
        assignn = TCappendAssign (assignn, bodycode);
    } else {
        assignn
          = ForEachElementHelp (l, u, s, w, 0, maxdim, assignn, wln, partn, arg_info);
    }

    l = MEMfree (l);
    u = MEMfree (u);
    s = MEMfree (s);
    w = MEMfree (w);

    DBUG_RETURN (assignn);
}

/******************************************************************************
 *
 * function:
 *   int CountElements(node *genn)
 *
 * description:
 *   Counts number of specified elements by generator node genn.
 *   Supports grids.
 *
 ******************************************************************************/

static int
CountElements (node *genn)
{
    int elts, tmp, d, m, dim, i;
    constant *const_l, *const_u, *const_s, *const_w;
    int *l, *u, *s, *w;

    DBUG_ENTER ("CountElements");

    const_l = COaST2Constant (GENERATOR_BOUND1 (genn));
    l = COgetDataVec (const_l);
    DBUG_ASSERT ((COgetDim (const_l) == 1), "inconsistant wl bounds found!");
    dim = SHgetExtent (COgetShape (const_l), 0);

    const_u = COaST2Constant (GENERATOR_BOUND2 (genn));
    u = COgetDataVec (const_u);
    DBUG_ASSERT ((SHgetExtent (COgetShape (const_u), 0) == dim),
                 "inconsistant wl bounds found!");

    if (GENERATOR_STEP (genn) != NULL) {
        const_s = COaST2Constant (GENERATOR_STEP (genn));
        s = COgetDataVec (const_s);
        DBUG_ASSERT ((SHgetExtent (COgetShape (const_s), 0) == dim),
                     "inconsistant wl bounds found!");
    } else {
        const_s = NULL;
        s = NULL;
    }

    if (GENERATOR_WIDTH (genn) != NULL) {
        const_w = COaST2Constant (GENERATOR_WIDTH (genn));
        w = COgetDataVec (const_w);
        DBUG_ASSERT ((SHgetExtent (COgetShape (const_w), 0) == dim),
                     "inconsistant wl bounds found!");
    } else {
        const_w = NULL;
        w = NULL;
    }

    elts = 1;
    for (i = 0; i < dim; i++) {
        tmp = 0;

        /* check step/width */
        if ((w && !s) || (w && (w[i] < 1)) || (s && w && (s[i] < w[i]))) {
            /* illegal */
            elts = global.wlunrnum + 1;
            break;
        }

        /* counts elements in this dimension */
        tmp = u[i] - l[i];
        if (s != NULL) {
            d = tmp / s[i];
            m = tmp % s[i];
            tmp = (w != NULL) ? (d * w[i]) : d;
            if (m) {
                tmp = tmp + (w ? (MATHmin (m, w[i])) : 1);
            }
        }

        /* summarise elements over all dimensions. */
        elts *= tmp;
    }

    const_l = COfreeConstant (const_l);
    const_u = COfreeConstant (const_u);
    if (const_s != NULL) {
        const_s = COfreeConstant (const_s);
    }
    if (const_w != NULL) {
        const_w = COfreeConstant (const_w);
    }

    DBUG_RETURN (elts);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollModarray(node *wln, info *arg_info)
 *
 * description:
 *   Checks if this modarray-WL can be unrolled.
 *   Multiple N_Npart nodes, which are not the identity of the base array,
 *   may be unrolled simultaneously. These N_Npart nodes are marked in
 *   NPART_COPY
 *
 ******************************************************************************/

static bool
CheckUnrollModarray (node *wln, node *lhs, info *arg_info)
{
    bool ok;
    int elts;
    node *partn, *genn, *coden, *tmpn, *exprn;

    DBUG_ENTER ("CheckUnrollModarray");

    /*
     * Check all N_parts:
     * Count the number of elements which do NOT just copy the original array
     * (e.g. body = {__flat = A[iv]} )
     */

    partn = WITH_PART (wln);
    elts = 0;

    ok = (TYisAKS (IDS_NTYPE (lhs)) || TYisAKV (IDS_NTYPE (lhs)));

    while (ok && (partn != NULL)) {
        genn = PART_GENERATOR (partn);

        /*
         * Check if code is a copy of the original array and set NPART_COPY
         * for later usage in WLUDoUnrollModarray().
         *
         * B = new_with
         *       ([ 0 ] <= __flat_1_iv=[__flat_0_i] < [ 3 ]) {
         *         __wlt_4 = sel( __flat_1_iv, A );
         *       } : __wlt_4,
         *       ...more parts...
         *     modarray( A);
         *
         * We need DCR to be done before to detect identity written by the
         * programmer.
         */

        coden = PART_CODE (partn);

        if (N_empty == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (coden)))) {
            PART_ISCOPY (partn) = FALSE;
        } else {
            tmpn = ASSIGN_INSTR (BLOCK_INSTR (CODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            PART_ISCOPY (partn)
              = ((N_let == NODE_TYPE (tmpn))
                 && (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (coden)))
                     == IDS_AVIS (LET_IDS (tmpn)))
                 && (N_prf == NODE_TYPE (exprn)) && (F_sel_VxA == PRF_PRF (exprn))
                 && (N_id == NODE_TYPE (PRF_ARG1 (exprn)))
                 && (IDS_AVIS (PART_VEC (partn)) == ID_AVIS (PRF_ARG1 (exprn)))
                 && (N_id == NODE_TYPE (PRF_ARG2 (exprn)))
                 && (ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (wln)))
                     == ID_AVIS (PRF_ARG2 (exprn))));
        }

        if (!PART_ISCOPY (partn)) {
            elts += CountElements (genn);
        }

        partn = PART_NEXT (partn);
    }

    if (ok && (elts > global.wlunrnum)) {
        ok = FALSE;
        if (elts <= 32) {
            CTInote ("WLUR: -maxwlur %d would unroll fold with-loop", elts);
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *FinalizeModarray( node *bodycode, node *withop,
 *                                    info *arg_info)
 *
 * description:
 *   Adds final code to the unrolled with-loop for the modarray op.
 *
 ******************************************************************************/

static node *
FinalizeModarray (node *bodycode, node *withop, node *lhs, info *arg_info)
{
    node *letn;
    node *res;

    DBUG_ENTER ("FinalizeModarray");

    /* Finally add duplication of new array name */
    letn = TBmakeLet (DUPdoDupNode (lhs), DUPdoDupTree (MODARRAY_ARRAY (withop)));
    res = TBmakeAssign (letn, bodycode);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollGenarray( node *wln, info *arg_info)
 *
 * description:
 *   Checks whether the given genarray-wl can be unrolled.
 *   Unrolling of arrays is done if number of array elements is smaller
 *   than wlunrnum.
 *
 ******************************************************************************/

static bool
CheckUnrollGenarray (node *wln, node *lhs, info *arg_info)
{
    bool ok;
    int length;

    DBUG_ENTER ("WLUCheckUnrollGenarray");

    if (TYisAKS (IDS_NTYPE (lhs)) || TYisAKV (IDS_NTYPE (lhs))) {
        length = SHgetUnrLen (TYgetShape (IDS_NTYPE (lhs)));
    } else {
        length = -1;
    }

    /*
     * Everything constant?
     */
    ok = (length >= 0);

    if (ok && (length > global.wlunrnum)) {
        ok = FALSE;
        if (length <= 32) {
            CTInote ("WLUR: -maxwlur %d would unroll genarray with-loop", length);
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *FinalizeGenarray( node *bodycode, node *withop,
 *                                    info *arg_info)
 *
 * description:
 *   Adds final code to the unrolled with-loop for the genarray op.
 *
 ******************************************************************************/

static node *
FinalizeGenarray (node *bodycode, node *withop, node *lhs, info *arg_info)
{
    ntype *type;
    simpletype btype;
    int length;
    node *shp, *shpavis;
    node *vect, *vectavis;
    node *vardecs = NULL;
    node *reshape;
    node *res;

    DBUG_ENTER ("FinalizeGenarray");

    /*
     * Prepend:
     *
     * <tmp1> = <shape>;
     * <tmp2> = [0,...,0];
     * <array> = _reshape_( <tmp1>, <tmp2>)
     */
    type = IDS_NTYPE (lhs);
    btype = TYgetSimpleType (TYgetScalar (type));
    length = SHgetUnrLen (TYgetShape (type));

    shp = SHshape2Array (TYgetShape (type));
    shpavis = TBmakeAvis (TRAVtmpVar (),
                          TYmakeAKS (TYmakeSimpleType (T_int),
                                     SHcreateShape (1, SHgetDim (TYgetShape (type)))));
    vardecs = TBmakeVardec (shpavis, vardecs);

    vect = TCcreateZeroVector (length, btype);
    vectavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (btype),
                                                     SHcreateShape (1, length)));
    vardecs = TBmakeVardec (vectavis, vardecs);

    reshape = TCmakePrf2 (F_reshape_VxA, TBmakeId (shpavis), TBmakeId (vectavis));

    if (TYisAKV (type)) {
        IDS_NTYPE (lhs) = TYeliminateAKV (type);
        type = TYfreeType (type);
    }

    res = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs), reshape), bodycode);
    res = TBmakeAssign (TBmakeLet (TBmakeIds (vectavis, NULL), vect), res);
    res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), shp), res);

    INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), vardecs);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollFold( node *wln, info *arg_info)
 *
 * description:
 *   Checks whether the given fold-wl can be unrolled.
 *
 ******************************************************************************/

static bool
CheckUnrollFold (node *wln)
{
    bool ok = TRUE;
    int elements;
    node *partn;
    node *genn;

    DBUG_ENTER ("CheckUnrollFold");

    /*
     * Loop through all N_parts, counting elements.
     * All bounds (low, high, step, width) have to be constant.
     */

    partn = WITH_PART (wln);
    elements = 0;

    while (partn != NULL) {
        genn = PART_GENERATOR (partn);
        elements += CountElements (genn);
        partn = PART_NEXT (partn);
    }

    if (elements > global.wlunrnum) {
        ok = FALSE;
        if (elements <= 32) {
            CTInote ("WLUR: -maxwlur %d would unroll fold with-loop", elements);
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollPropagate( node *wln, info *arg_info)
 *
 * description:
 *   Checks whether the given propagate-wl can be unrolled.
 *
 ******************************************************************************/

static bool
CheckUnrollPropagate (node *wln)
{
    bool ok = TRUE;
    int elements;
    node *partn;
    node *genn;

    DBUG_ENTER ("CheckUnrollPropagate");

    /*
     * Loop through all N_parts, counting elements.
     * All bounds (low, high, step, width) have to be constant.
     */

    partn = WITH_PART (wln);
    elements = 0;

    while (partn != NULL) {
        genn = PART_GENERATOR (partn);
        elements += CountElements (genn);
        partn = PART_NEXT (partn);
    }

    if (elements > global.wlunrnum) {
        ok = FALSE;
        if (elements <= 32) {
            CTInote ("WLUR: -maxwlur %d would unroll propagate with-loop", elements);
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *FinalizeFold( node *bodycode, node *withop,
 *                                info *arg_info)
 *
 * description:
 *   Adds final code to the unrolled with-loop for the fold op.
 *
 ******************************************************************************/

static node *
FinalizeFold (node *bodycode, node *withop, node *lhs, info *arg_info)
{
    node *letn;
    node *res;

    DBUG_ENTER ("FinalizeFold");

    /* add initialisation of accumulator with neutral element. */
    letn = TBmakeLet (DUPdoDupNode (lhs), DUPdoDupTree (FOLD_NEUTRAL (withop)));
    res = TBmakeAssign (letn, bodycode);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *FinalizePropagate( node *bodycode, node *withop, node *lhs
 *                                    info *arg_info)
 *
 * description:
 *   Adds final code to the unrolled with-loop for the extract op.
 *
 ******************************************************************************/

static node *
FinalizePropagate (node *bodycode, node *withop, node *lhs, info *arg_info)
{
    node *letn;
    node *assignn;

    DBUG_ENTER ("FinalizePropagate");

    /* Append final assign of the last resulting object to the loop's lhs. */
    letn = TBmakeLet (DUPdoDupNode (lhs), DUPdoDupTree (PROPAGATE_DEFAULT (withop)));
    assignn = TBmakeAssign (letn, NULL);
    bodycode = TCappendAssign (bodycode, assignn);

    DBUG_RETURN (bodycode);
}

/******************************************************************************
 *
 * function:
 *   node *DoUnrollWithloop(node *wln, info *arg_info)
 *
 * description:
 *   Unrolls the given with-loop.
 *
 ******************************************************************************/

static node *
DoUnrollWithloop (node *wln, info *arg_info)
{
    node *partn;
    node *res;
    node *withop;
    node *lhs;

    DBUG_ENTER ("DoUnrollWithloop");

    partn = WITH_PART (wln);
    withop = WITH_WITHOP (wln);
    res = NULL;
    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    /* Go over every partition of the with loop, copy the body code
     * and apply each withop. */
    while (partn != NULL) {

        /* If this part just copies the previous array, AND this is not
         * a multi-operator with-loop, skip it. */
        if (!PART_ISCOPY (partn) || WITHOP_NEXT (WITH_WITHOP (wln))) {
            res = ForEachElement (res, wln, partn, arg_info);
        }
        partn = PART_NEXT (partn);
    }

    /* Finalize the unrolling for every withop. */
    withop = WITH_WITHOP (wln);

    while (withop != NULL) {
        switch (NODE_TYPE (withop)) {
        case N_genarray:
            res = FinalizeGenarray (res, withop, lhs, arg_info);
            break;
        case N_modarray:
            res = FinalizeModarray (res, withop, lhs, arg_info);
            break;
        case N_fold:
            res = FinalizeFold (res, withop, lhs, arg_info);
            break;
        case N_break:
            /* no-op */
            break;
        case N_propagate:
            res = FinalizePropagate (res, withop, lhs, arg_info);
            break;
        default:
            DBUG_ASSERT (0, "unhandled with-op");
        }
        withop = WITHOP_NEXT (withop);
        lhs = IDS_NEXT (lhs);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollWithloop( node *wln)
 *
 * description:
 *   Checks whether the given with-loop may be unrolled. Returns true if this
 *   is the case, false otherwise. Checks if the bounds are constant, and if so,
 *   checks for each withop whether their restrictions are satisfied.
 *
 ******************************************************************************/

static bool
CheckUnrollWithloop (node *wln, info *arg_info)
{
    int ok = TRUE;
    node *partn;
    node *genn;
    node *op;
    node *lhs;

    DBUG_ENTER ("CheckUnrollWithloop");

    partn = WITH_PART (wln);

    /* Everything constant? */
    while (ok && (partn != NULL)) {
        genn = PART_GENERATOR (partn);
        ok = (NODE_TYPE (genn) == N_generator && COisConstant (GENERATOR_BOUND1 (genn))
              && COisConstant (GENERATOR_BOUND2 (genn))
              && ((GENERATOR_STEP (genn) == NULL) || COisConstant (GENERATOR_STEP (genn)))
              && ((GENERATOR_WIDTH (genn) == NULL)
                  || COisConstant (GENERATOR_WIDTH (genn))));
        partn = PART_NEXT (partn);
    }

    /* Check for every with-op */
    op = WITH_WITHOP (wln);
    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    while (ok && op != NULL) {
        switch (NODE_TYPE (op)) {
        case N_genarray:
            ok = CheckUnrollGenarray (wln, lhs, arg_info);
            break;
        case N_modarray:
            ok = CheckUnrollModarray (wln, lhs, arg_info);
            break;
        case N_fold:
            ok = CheckUnrollFold (wln);
            break;
        case N_break:
            /* no-op */
            break;
        case N_propagate:
            ok = CheckUnrollPropagate (wln);
            break;
        default:
            DBUG_ASSERT (0, "unhandled with-op");
        }
        op = WITHOP_NEXT (op);
        lhs = IDS_NEXT (lhs);
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *LacFundef( node *arg_node, info *arg_info)
 *
 * description:
 *   Special version of WLURfundef for LAC functions.
 *
 ******************************************************************************/

node *
LacFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLURfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }
    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*
 * Traversal functions
 */

/******************************************************************************
 *
 * function:
 *   node *WLURap( node *arg_node, info *arg_info)
 *
 * description:
 *   Checks whethers this is an appliance of a special loop or cond function,
 *   and recursively traverses it if so.
 *
 ******************************************************************************/

node *
WLURap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("WLURap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("WLUR", ("traverse in special fundef %s",
                             FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = LacFundef (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("WLUR", ("traversal of special fundef %s finished\n",
                             FUNDEF_NAME (AP_FUNDEF (arg_node))));
        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("WLUR", ("do not traverse in normal fundef %s",
                             FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
WLURassign (node *arg_node, info *arg_info)
{
    node *pre_assigns;
    node *tmp;
    node *old_assign;

    DBUG_ENTER ("WLURassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    /* stack actual assign */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    pre_assigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /* restore stacked assign */
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* integrate pre_assignments in assignment chain and remove this assign */
    if (pre_assigns != NULL) {
        tmp = arg_node;
        arg_node = TCappendAssign (pre_assigns, ASSIGN_NEXT (arg_node));
        tmp = FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
WLURfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLURfundef");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        if (FUNDEF_BODY (arg_node) != NULL) {
            /* traverse block of fundef */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
        INFO_FUNDEF (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURwith( node *arg_node, info *arg_info)
 *
 * description:
 *   Checks whether the given with-node can be unrolled, and if so,
 *   applies the unroll.
 *
 ******************************************************************************/

node *
WLURwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLURwith");

    /* traverse the N_Nwithop node */
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    /* traverse all generators */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    /* traverse bodies */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    /* can this WL be unrolled? */
    if (CheckUnrollWithloop (arg_node, arg_info)) {
        global.optcounters.wlunr_expr++;
        INFO_PREASSIGN (arg_info) = DoUnrollWithloop (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURdoWithloopUnrolling( node *syntax_tree)
 *
 * description:
 *   Starts the with-loop unrolling traversal for the given syntax tree.
 *
 ******************************************************************************/

node *
WLURdoWithloopUnrolling (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLURdoWithloopUnrolling");

    TRAVpush (TR_wlur);

    info = MakeInfo ();

    global.valid_ssaform = FALSE;
    /*
     * New code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation module
     * ssa_transform. Therefore, we adjust the global control flag.
     */

    syntax_tree = TRAVdo (syntax_tree, info);

    FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
