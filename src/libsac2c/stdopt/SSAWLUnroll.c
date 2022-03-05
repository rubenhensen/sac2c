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

#define DBUG_PREFIX "WLUR"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "user_types.h"
#include "shape.h"
#include "math_utils.h"
#include "tree_utils.h"
#include "type_utils.h"
#include "DupTree.h"
#include "SSAWithloopFolding.h"
#include "indexvectorutils.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
CreateBodyCode (node *partn, node *index, ntype *def_type)
{
    node *res, *letn, *coden;
    node *_ids;
    node *cexpr_avis;

    DBUG_ENTER ();

    coden = PART_CODE (partn);

    res = DUPdoDupTree (BLOCK_ASSIGNS (CODE_CBLOCK (coden)));

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

    if (def_type != NULL) {
        /*
         * we insert a type conversion to the default type at the very end.
         */
        cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (coden)));
        res
          = TCappendAssign (res,
                            TBmakeAssign (TBmakeLet (TBmakeIds (cexpr_avis, NULL),
                                                     TCmakePrf2 (F_type_conv,
                                                                 TBmakeType (
                                                                   TYeliminateAKV (
                                                                     def_type)),
                                                                 TBmakeId (cexpr_avis))),
                                          NULL));
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    /*
     * special handling of
     *       acc = accu(iv,n);
     *       b = <body>
     *       cexpr = op( acc, b);
     * needed.
     * acc = accu(iv,n)  -> acc = LHS of current WL(;
     * append new last assignment: LHS of current WL = cexpr;
     */

    DBUG_ASSERT (bodycode != NULL, "BLOCK_ASSIGNS is empty!");

    tmp = bodycode;
    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {
            ASSIGN_RHS (tmp) = FREEdoFreeNode (ASSIGN_RHS (tmp));
            ASSIGN_RHS (tmp) = DUPdupIdsId (lhs);
            F_accu_found = TRUE;
        }

        if (ASSIGN_NEXT (tmp) == NULL) {
            DBUG_ASSERT (F_accu_found, "No F_accu found!");

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

    DBUG_ENTER ();

    DBUG_ASSERT (bodycode != NULL, "BLOCK_ASSIGNS is empty!");

    /*
     * remove affected objects from the expression
     *
     *    a', ...  = prop_obj(iv, a, ...)
     */

    tmp = bodycode;
    tmp_prev = NULL;

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

    DBUG_ENTER ();

    withop = WITH_WITHOP (wln);
    cexpr = CODE_CEXPRS (PART_CODE (partn));
    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    while (withop != NULL) {
        switch (NODE_TYPE (withop)) {
        case N_genarray:
            DBUG_PRINT ("withop: genarray");
            break;
        case N_modarray:
            DBUG_PRINT ("withop: modarray");
            break;
        case N_fold:
            DBUG_PRINT ("withop: fold");
            break;
        case N_break:
            DBUG_PRINT ("withop: break");
            break;
        case N_propagate:
            DBUG_PRINT ("withop: propagate");
            break;
        default:
            DBUG_UNREACHABLE ("unhandled withop");
        }

        DBUG_PRINT ("cexpr: %s", ID_NAME (EXPRS_EXPR (cexpr)));
        DBUG_PRINT ("lhs: %s\n", IDS_NAME (lhs));

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
            DBUG_UNREACHABLE ("unhandled withop");
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
    ntype *def_type;

    DBUG_ENTER ();

    DBUG_ASSERT (maxdim > 0, "illegal max. dim found!");
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
            /* In case of genarray-WLs, we also pass along the type of the default
             * element as this can play a vital role for improved type knowledge
             * (cf bug 556).
             */
            if ((NODE_TYPE (WITH_WITHOP (wln)) == N_genarray)
                && (GENARRAY_DEFAULT (WITH_WITHOP (wln)) != NULL)) {
                def_type = ID_NTYPE (GENARRAY_DEFAULT (WITH_WITHOP (wln)));
            } else {
                def_type = NULL;
            }
            bodycode = CreateBodyCode (partn, index, def_type);
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
    ntype *def_type;

    DBUG_ENTER ();

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
        /* In case of genarray-WLs, we also pass along the type of the default
         * element as this can play a vital role for improved type knowledge
         * (cf bug 556).
         */
        if ((NODE_TYPE (WITH_WITHOP (wln)) == N_genarray)
            && (GENARRAY_DEFAULT (WITH_WITHOP (wln)) != NULL)) {
            def_type = ID_NTYPE (GENARRAY_DEFAULT (WITH_WITHOP (wln)));
        } else {
            def_type = NULL;
        }
        bodycode = CreateBodyCode (partn, index, def_type);
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

    DBUG_ENTER ();

    const_l = COaST2Constant (GENERATOR_BOUND1 (genn));
    l = (int *)COgetDataVec (const_l);
    DBUG_ASSERT (COgetDim (const_l) == 1, "inconsistant wl bounds found!");
    dim = SHgetExtent (COgetShape (const_l), 0);

    const_u = COaST2Constant (GENERATOR_BOUND2 (genn));
    u = (int *)COgetDataVec (const_u);
    DBUG_ASSERT (SHgetExtent (COgetShape (const_u), 0) == dim,
                 "inconsistant wl bounds found!");

    if (GENERATOR_STEP (genn) != NULL) {
        const_s = COaST2Constant (GENERATOR_STEP (genn));
        s = (int *)COgetDataVec (const_s);
        DBUG_ASSERT (SHgetExtent (COgetShape (const_s), 0) == dim,
                     "inconsistant wl bounds found!");
    } else {
        const_s = NULL;
        s = NULL;
    }

    if (GENERATOR_WIDTH (genn) != NULL) {
        const_w = COaST2Constant (GENERATOR_WIDTH (genn));
        w = (int *)COgetDataVec (const_w);
        DBUG_ASSERT (SHgetExtent (COgetShape (const_w), 0) == dim,
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
 * function: isWithVec()
 *
 * description:
 *   Predicate for showing that ivoroffset is derived from
 *   partn's WITHID_VEC.
 *
 ******************************************************************************/
static bool
isWithidVec (node *ivoroffset, node *partn)
{
    bool z;
    DBUG_ENTER ();

    z = NULL != IVUTfindIvWithid (ivoroffset, partn);

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   bool CheckUnrollModarray(node *wln, info *arg_info)
 *
 * description:
 *   Checks if this modarray-WL can be unrolled.
 *   In contrast to genarray, we neither need to know the shape of the result
 *   as we do have a template, nor do we require the total sum of all elements
 *   to be smaller than wlurnum. Here, it suffices if the sum of elements of
 *   all non-copy partitions are smaller than wlurnum.
 *   We mark all copy partitions as ISCOPY.
 *
 ******************************************************************************/

static bool
CheckUnrollModarray (node *wln, node *lhs, info *arg_info)
{
    bool ok;
    int elts;
    node *partn, *genn, *coden, *tmpn, *exprn;

    DBUG_ENTER ();

    /*
     * Check all N_parts:
     * Count the number of elements which do NOT just copy the original array
     * (e.g. body = {__flat = A[iv]} )
     */

    partn = WITH_PART (wln);
    elts = 0;

    ok = TRUE;

    while (ok && (partn != NULL)) {
        genn = PART_GENERATOR (partn);

        /*
         * Check if code is a copy of the original array and set NPART_COPY
         * for later usage in WLUDoUnrollModarray().
         *
         * B = new_with
         *       ([ 0 ] <= __flat_1_iv=[__flat_0_i] < [ 3 ]) {
         *         __wlt_4 = _sel_VxA_( __flat_1_iv, A );
         *       } : __wlt_4,
         *       ...more parts...
         *     modarray( A);
         *
         * We need DCR to be done before to detect identity written by the
         * programmer.
         */

        coden = PART_CODE (partn);

        if (NULL == BLOCK_ASSIGNS (CODE_CBLOCK (coden))) {
            PART_ISCOPY (partn) = FALSE;
        } else {
            tmpn = ASSIGN_STMT (BLOCK_ASSIGNS (CODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            PART_ISCOPY (partn)
              = ((N_let == NODE_TYPE (tmpn))
                 && (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (coden)))
                     == IDS_AVIS (LET_IDS (tmpn)))
                 && (N_prf == NODE_TYPE (exprn))
                 && ((F_sel_VxA == PRF_PRF (exprn)) || ((F_idx_sel == PRF_PRF (exprn))))
                 && (N_id == NODE_TYPE (PRF_ARG1 (exprn)))
                 && (isWithidVec (PRF_ARG1 (exprn), partn))
                 && (N_id == NODE_TYPE (PRF_ARG2 (exprn)))
                 && (ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (wln)))
                     == ID_AVIS (PRF_ARG2 (exprn))));
        }

        if (!PART_ISCOPY (partn)) {
            elts += CountElements (genn);
        }

        partn = PART_NEXT (partn);
    }

    DBUG_PRINT ("   modarray: %d indices in non-copy partitions; maxwlur %d",
                elts, global.wlunrnum);
    if (ok && (elts > global.wlunrnum)) {
        ok = FALSE;
        if (elts <= 32) {
            CTInote (EMPTY_LOC, "WLUR: -maxwlur %d would unroll modarray with-loop", elts);
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

    DBUG_ENTER ();

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
 *   This requires:
 *   a) the shape expression to be AKV
 *   b) the product of shape to be <= maxwlur
 *   c) a defaultexpression to exist
 *
 ******************************************************************************/

static bool
CheckUnrollGenarray (node *wln, node *lhs, info *arg_info)
{
    bool ok;
    long long length;
    constant *shp;
    shape *shshp;
    node *def;

    DBUG_ENTER ();

    ok = TRUE;
    shp = COaST2Constant (GENARRAY_SHAPE (WITH_WITHOP (wln)));

    if (shp!=NULL) {
        DBUG_PRINT ("   genarray: shape is const");
        shshp = COconstant2Shape (shp);
        length = SHgetUnrLen (shshp);
        shp = COfreeConstant (shp);
        shshp = SHfreeShape (shshp);

        DBUG_PRINT ("   genarray: %lld elements; maxwlur %d", length, global.wlunrnum);
        if (length > global.wlunrnum) {
            ok = FALSE;
            if (length <= 32) {
                CTInote (EMPTY_LOC, "WLUR: -maxwlur %lld would unroll genarray with-loop", length);
            }
        }
    } else {
        DBUG_PRINT ("   genarray: shape is not const");
        ok = FALSE;
    }
    def = GENARRAY_DEFAULT (WITH_WITHOP (wln));
    DBUG_PRINT ("   genarray: default expression %s",
                 def != NULL ? "exists" : "does not exist");

    ok = ok && (def != NULL);

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
 *   This indeed is more challenging than what one may think.
 *   The main challenge is that we want to be able to handle cases
 *   where the shape expression is constant, but the overall array
 *   may still be aud! This means that we have to create the "template"
 *   array from the default element and the shape expression.
 *   Assuming     genarray( [s0, ..., sn], def_id) ;
 *   we distinguish 3 cases:
 *
 *   1) the straight-forward case is an array with the frameshape
 *      [s0, ..., sn] whose elements are the default element, i.e.:
 *
 *         tmp1 = [def_id, ..., def_id];              // prod(si) many def_id
 *         <array> = _type_conv_( <lhs-type>, tmp1);
 *
 *      The type conv guarantees that we are not loosing potential type
 *      information gained from partitions.
 *
 *   2) if the shape (frameshape) contains a zero, we cannot use the same code
 *      as in 1), since we would loose the inner shape information
 *      completely in case def_id is not AKS. Instead, we generate
 *
 *         tmp1 = [:<basetype>];
 *         tmp2 = [s0, ..., sn];
 *         tmp3 = _shape_A_ (def_id);
 *         tmp4 = _cat_VxV_(tmp2, tmp3);
 *         tmp5 = _reshape_VxA_(tmp4, tmp1);
 *         <array> = _type_conv_( <lhs-type>, tmp5);
 *
 *   3) If the frameshape is [:int] we have the same problem. While we could
 *      use the same code as in 2), we can short-cut the code into:
 *         <array> = _type_conv_( <lhs-type>, def_id);
 *
 ******************************************************************************/

static node *
FinalizeGenarray (node *bodycode, node *withop, node *lhs, info *arg_info)
{
    node *shp, *def, *expr, *avis, *vardecs=NULL, *assigns=NULL, *res;
    node *elems, *tmp1, *tmp2, *tmp3, *tmp4;
    ntype *def_type, *lhs_type, *expr_type;
    constant *shp_co;
    shape *shp_sh;
    long long num_elems;
    int i;

    DBUG_ENTER ();

    shp = GENARRAY_SHAPE (withop);
    shp_co = COaST2Constant (GENARRAY_SHAPE (withop));
    shp_sh = COconstant2Shape (shp_co);
    def = GENARRAY_DEFAULT (withop);
    def_type = ID_NTYPE (def);
    num_elems = SHgetUnrLen (shp_sh);
    lhs_type = IDS_NTYPE (lhs);

    /*
     * NB: WLSIMP elides all generators and even WLs with empty shape.
     *     This means that case 2) in principle is redundant!
     *     However, WLSIMP can be turned off and WLT does not like
     *     empty WLs. WLUR can not be turned off entirely just
     *     to ensure WLT compatability! Therefore, we leave case 2)
     *     in here.
     */
    if (num_elems == 0) {
        DBUG_PRINT ("frameshape contains 0 components: building variant 2");
        /*
         * Version 2:
         *    tmp1 = [:<basetype>];
         *    tmp2 = [s0, ..., sn];
         *    tmp3 = _shape_A_ (def_id);
         *    tmp4 = _cat_VxV_(tmp2, tmp3);
         *    tmp5 = _reshape_VxA_(tmp4, tmp1);
         *    <array> = _type_conv_( <lhs-type>, tmp5);
         */
        expr = TBmakeArray (TYmakeAKS (
                                TYmakeSimpleType (TUgetBaseSimpleType (def_type)),
                                SHmakeShape (0)),
                            SHcreateShape (1, 0),
                            NULL);
        avis = TBmakeAvis (TRAVtmpVar (), NTCnewTypeCheck_Expr (expr));
        vardecs = TBmakeVardec (avis, vardecs);
        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), expr), NULL);
        tmp1 = TBmakeId (avis);

        expr = DUPdoDupTree (shp);
        avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKV (TYmakeSimpleType (T_int), shp_co));
        vardecs = TBmakeVardec (avis, vardecs);
        assigns = TCappendAssign (
                      assigns,
                      TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), expr), NULL));
        tmp2 = TBmakeId (avis);

        expr = TCmakePrf1 (F_shape_A, DUPdoDupNode (def));
        expr_type = NTCnewTypeCheck_Expr (expr);
        avis = TBmakeAvis (TRAVtmpVar (), TYgetProductMember (expr_type, 0));
        expr_type = TYfreeTypeConstructor (expr_type);
        vardecs = TBmakeVardec (avis, vardecs);
        assigns = TCappendAssign (
                      assigns,
                      TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), expr), NULL));
        tmp3 = TBmakeId (avis);

        expr = TCmakePrf2 (F_cat_VxV, tmp2, tmp3);
        expr_type = NTCnewTypeCheck_Expr (expr);
        avis = TBmakeAvis (TRAVtmpVar (), TYgetProductMember (expr_type, 0));
        expr_type = TYfreeTypeConstructor (expr_type);
        vardecs = TBmakeVardec (avis, vardecs);
        assigns = TCappendAssign (
                      assigns,
                      TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), expr), NULL));
        tmp4 = TBmakeId (avis);

        expr = TCmakePrf2 (F_reshape_VxA, tmp4, tmp1);
        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (lhs_type));
        vardecs = TBmakeVardec (avis, vardecs);
        assigns = TCappendAssign (
                      assigns,
                      TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), expr), NULL));
        res = TBmakeId (avis);
        shp_sh = SHfreeShape (shp_sh);

    } else if (SHgetDim (shp_sh) > 0) {
        DBUG_PRINT ("frameshape contains %lld components: building variant 1", num_elems);
        /*
         * Version 1:
         *    tmp1 = [def_id, ..., def_id];
         *    <array> = _type_conv_( <lhs-type>, tmp1);
         */

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (lhs_type));
        vardecs = TBmakeVardec (avis, vardecs);

        elems = NULL;
        for (i = 0; i < num_elems; i++) {
            elems = TBmakeExprs ( DUPdoDupNode (def), elems);
        }
        assigns = TBmakeAssign (
                      TBmakeLet (
                          TBmakeIds (avis, NULL),
                          TBmakeArray ( TYeliminateAKV (def_type), shp_sh, elems)),
                      NULL);
        res = TBmakeId (avis);
        shp_co = COfreeConstant (shp_co);

    } else {
        DBUG_PRINT ("frameshape is [:int]: building variant 3");
        /*
         * Version 3:
         *    <array> = _type_conv_( <lhs-type>, def_id);
         */
        assigns = NULL;
        res = DUPdoDupNode (def);
        shp_sh = SHfreeShape (shp_sh);
        shp_co = COfreeConstant (shp_co);
    }

    res = TBmakeAssign (
              TBmakeLet (
                  DUPdoDupNode (lhs),
                  TCmakePrf2 (F_type_conv, TBmakeType (TYcopyType (lhs_type)), res)),
              bodycode);
    res = TCappendAssign (assigns, res);

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

    DBUG_ENTER ();

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
            CTInote (EMPTY_LOC, "WLUR: -maxwlur %d would unroll fold with-loop", elements);
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

    DBUG_ENTER ();

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
            CTInote (EMPTY_LOC, "WLUR: -maxwlur %d would unroll propagate with-loop", elements);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
            DBUG_UNREACHABLE ("unhandled with-op");
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
    bool ok = TRUE;
    bool b1, b2, idx;
    node *partn;
    node *genn;
    node *op;
    node *lhs;
#ifndef DBUG_OFF
    int p=0;
#endif

    DBUG_ENTER ();

    partn = WITH_PART (wln);

    /* Everything constant? */
    while (ok && (partn != NULL)) {
        genn = PART_GENERATOR (partn);
        DBUG_ASSERT (NODE_TYPE (genn) == N_generator, "non N_generator partition found!");
        b1 = COisConstant (GENERATOR_BOUND1 (genn));
        b2 = COisConstant (GENERATOR_BOUND2 (genn));
        idx = TYisAKS (IDS_NTYPE (PART_VEC (partn)));
        DBUG_PRINT ("   Bound1: %s", (b1 ? "is const" : "not const"));
        DBUG_PRINT ("   Bound2: %s", (b2 ? "is const" : "not const"));
        DBUG_PRINT ("   idx-vec: %s", (idx ? "is AKS" : "is not AKS"));
        ok = (b1 && b2 && idx
              && ((GENERATOR_STEP (genn) == NULL) || COisConstant (GENERATOR_STEP (genn)))
              && ((GENERATOR_WIDTH (genn) == NULL) || COisConstant (GENERATOR_WIDTH (genn))));
        DBUG_PRINT ("   => partition %d: %s", p++, (ok? "ok" : "not ok"));
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
            DBUG_UNREACHABLE ("unhandled with-op");
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
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    /* traverse block of fundef */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
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

    DBUG_ENTER ();

    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "missing fundef in ap-node");

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("traverse in special fundef %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = LacFundef (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("traversal of special fundef %s finished\n",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("do not traverse in normal fundef %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
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

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_STMT (arg_node) != NULL, "assign node without instruction");

    /* stack actual assign */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    pre_assigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /* restore stacked assign */
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* integrate pre_assignments in assignment chain and remove this assign */
    if (pre_assigns != NULL) {
        tmp = arg_node;
        TUclearSsaAssign (arg_node);
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
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at N_fundef for %s", FUNDEF_NAME( arg_node));
    if (!FUNDEF_ISLACFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        /*
         * New code is created in non-SSA form and later on transformed into
         * SSA form using the standard transformation module
         * ssa_transform. Therefore, we clear a global control flag, to force
         * SSAT to run.
         *
         * NB. I don't like this: We should only clear ssaform if WLUR actually
         *     changes any code.
         */
        global.valid_ssaform = FALSE;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
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
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at WL for %s",
                AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_ASSIGN (arg_info))))));
    /* traverse the N_Nwithop node */
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    /* traverse all generators */
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* traverse bodies */
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    /* can this WL be unrolled? */
    if (CheckUnrollWithloop (arg_node, arg_info)) {
        DBUG_PRINT ("Unrolling WL for %s", AVIS_NAME (IDS_AVIS (LET_IDS (
                                             ASSIGN_STMT (INFO_ASSIGN (arg_info))))));
        global.optcounters.wlunr_expr++;
        INFO_PREASSIGN (arg_info) = DoUnrollWithloop (arg_node, arg_info);
    } else {
        DBUG_PRINT ("Cannot unroll WL for %s", AVIS_NAME (IDS_AVIS (LET_IDS (
                                                 ASSIGN_STMT (INFO_ASSIGN (arg_info))))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURdoWithloopUnrollingModule( node *arg_node)
 *
 * description:
 *   Starts the with-loop unrolling traversal for the N_module arg_node
 *
 *   This is called only in the post-optimization stage of compilation.
 *   NB. If the compiler is running with limited optimizations, such
 *   as -noopt, WLUR may leave the ast in a less-than-optimal
 *   state, due to removing a one-trip WL that would otherwise crash WLT,
 *   as described in gitlab Issue #2280.
 *
 *   If this becomes problematic, then fix the WLT problem.
 *
 ******************************************************************************/

node *
WLURdoWithloopUnrollingModule (node *arg_node)
{
    info *myinfo;

    DBUG_ENTER ();

    TRAVpush (TR_wlur);
    myinfo = MakeInfo ();

    if (MODULE_FUNS (arg_node) != NULL)
        MODULE_FUNS (arg_node) = TRAVsons (MODULE_FUNS (arg_node), myinfo);

    FreeInfo (myinfo);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLURdoWithloopUnrollingFundef( node *arg_node, info *arg_info)
 *
 * description:
 *   Starts the with-loop unrolling traversal for the N_fundef arg_node
 *
 *   This is called only during the optimization stage of compilation.
 *
 ******************************************************************************/

node *
WLURdoWithloopUnrollingFundef (node *arg_node, info *arg_info)
{
    info *myinfo;

    DBUG_ENTER ();

    TRAVpush (TR_wlur);
    myinfo = MakeInfo ();

    arg_node = WLURfundef(arg_node, myinfo);

    FreeInfo (myinfo);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
