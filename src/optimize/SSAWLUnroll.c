/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "node_basic.h"
#include "globals.h"
#include "free.h"
#include "ctinfo.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "new_types.h"
#include "shape.h"

#include "optimize.h"
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

    result = ILIBmalloc (sizeof (info));

    INFO_ASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
ApplyModGenarray (node *bodycode, node *index, node *partn, node *array)
{
    node *exprs, *letexpr, *cexpr, *tmpn;

    DBUG_ENTER ("ApplyModGenarray");

    /* create prf modarray */
    cexpr = EXPRS_EXPR (CODE_CEXPRS (PART_CODE (partn)));
    tmpn = TBmakeId (IDS_AVIS (array));

    exprs = TBmakeExprs (tmpn, TBmakeExprs (TBmakeId (IDS_AVIS (PART_VEC (partn))),
                                            TBmakeExprs (DUPdoDupTree (cexpr), NULL)));

    letexpr = TBmakePrf (F_modarray, exprs);

    /* append to body code */
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
ApplyFold (node *bodycode, node *index, node *partn, node *acc, node *cexpr)
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
            ASSIGN_RHS (tmp) = DUPdupIdsId (acc);
            F_accu_found = TRUE;
        }

        if (ASSIGN_NEXT (tmp) == NULL) {
            DBUG_ASSERT ((F_accu_found), "No F_accu found!");

            /* Append new assign: lhs(wl) = cexpr; */
            letn = TBmakeLet (DUPdoDupNode (acc), DUPdoDupNode (cexpr));
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
    node *arrayname;
    node *acc;
    node *cexpr;

    DBUG_ENTER ("ForEachElementWithop");

    withop = WITH_WITHOP (wln);
    while (withop != NULL) {
        switch (NODE_TYPE (withop)) {
        case N_genarray:
            arrayname = LET_IDS (ASSIGN_INSTR (INFO_ASSIGN (arg_info)));
            bodycode = ApplyModGenarray (bodycode, index, partn, arrayname);
            break;
        case N_modarray:
            arrayname = ASSIGN_LHS (INFO_ASSIGN (arg_info));
            bodycode = ApplyModGenarray (bodycode, index, partn, arrayname);
            break;
        case N_fold:
            acc = LET_IDS (ASSIGN_INSTR (INFO_ASSIGN (arg_info)));
            cexpr = EXPRS_EXPR (CODE_CEXPRS (PART_CODE (partn)));
            bodycode = ApplyFold (bodycode, index, partn, acc, cexpr);
            break;
        case N_break:
            /* no-op */
            break;
        default:
            DBUG_ASSERT (0, "unhandled withop");
        }
        withop = WITHOP_NEXT (withop);
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

    l = ILIBfree (l);
    u = ILIBfree (u);
    s = ILIBfree (s);
    w = ILIBfree (w);

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
                tmp = tmp + (w ? (MIN (m, w[i])) : 1);
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
CheckUnrollModarray (node *wln, info *arg_info)
{
    bool ok;
    int elts;
    node *partn, *genn, *coden, *tmpn, *exprn, *lhs;

    DBUG_ENTER ("CheckUnrollModarray");

    /*
     * Check all N_parts:
     * Count the number of elements which do NOT just copy the original array
     * (e.g. body = {__flat = A[iv]} )
     */

    partn = WITH_PART (wln);
    elts = 0;

    lhs = LET_IDS (ASSIGN_INSTR (INFO_ASSIGN (arg_info)));

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
                 && (N_prf == NODE_TYPE (exprn)) && (F_sel == PRF_PRF (exprn))
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
FinalizeModarray (node *bodycode, node *withop, info *arg_info)
{
    node *letn;
    node *res;

    DBUG_ENTER ("FinalizeModarray");

    /* Finally add duplication of new array name */
    letn = TBmakeLet (DUPdoDupNode (ASSIGN_LHS (INFO_ASSIGN (arg_info))),
                      DUPdoDupTree (MODARRAY_ARRAY (withop)));
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
CheckUnrollGenarray (node *wln, info *arg_info)
{
    bool ok;
    int length;
    node *lhs;

    DBUG_ENTER ("WLUCheckUnrollGenarray");

    lhs = LET_IDS (ASSIGN_INSTR (INFO_ASSIGN (arg_info)));

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
FinalizeGenarray (node *bodycode, node *withop, info *arg_info)
{
    ntype *type;
    simpletype btype;
    int length;
    node *shp, *shpavis;
    node *vect, *vectavis;
    node *vardecs = NULL;
    node *arrayname;
    node *reshape;
    node *res;

    DBUG_ENTER ("FinalizeGenarray");

    arrayname = LET_IDS (ASSIGN_INSTR (INFO_ASSIGN (arg_info)));

    /*
     * Prepend:
     *
     * <tmp1> = <shape>;
     * <tmp2> = [0,...,0];
     * <array> = _reshape_( <tmp1>, <tmp2>)
     */
    type = IDS_NTYPE (arrayname);
    btype = TYgetSimpleType (TYgetScalar (type));
    length = SHgetUnrLen (TYgetShape (type));

    shp = SHshape2Array (TYgetShape (type));
    shpavis = TBmakeAvis (ILIBtmpVar (),
                          TYmakeAKS (TYmakeSimpleType (T_int),
                                     SHcreateShape (1, SHgetDim (TYgetShape (type)))));
    vardecs = TBmakeVardec (shpavis, vardecs);

    vect = TCcreateZeroVector (length, btype);
    vectavis = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (btype),
                                                     SHcreateShape (1, length)));
    vardecs = TBmakeVardec (vectavis, vardecs);

    reshape = TCmakePrf2 (F_reshape, TBmakeId (shpavis), TBmakeId (vectavis));

    if (TYisAKV (type)) {
        IDS_NTYPE (arrayname) = TYeliminateAKV (type);
        type = TYfreeType (type);
    }

    res = TBmakeAssign (TBmakeLet (DUPdoDupNode (arrayname), reshape), bodycode);
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
 *   node *FinalizeFold( node *bodycode, node *withop,
 *                                info *arg_info)
 *
 * description:
 *   Adds final code to the unrolled with-loop for the fold op.
 *
 ******************************************************************************/

static node *
FinalizeFold (node *bodycode, node *withop, info *arg_info)
{
    node *letn;
    node *res;

    DBUG_ENTER ("FinalizeFold");

    /* add initialisation of accumulator with neutral element. */
    letn = TBmakeLet (DUPdoDupNode (ASSIGN_LHS (INFO_ASSIGN (arg_info))),
                      DUPdoDupTree (FOLD_NEUTRAL (withop)));
    res = TBmakeAssign (letn, bodycode);

    DBUG_RETURN (res);
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
    node *partn, *res, *withop;

    DBUG_ENTER ("DoUnrollWithloop");

    partn = WITH_PART (wln);
    withop = WITH_WITHOP (wln);
    res = NULL;

    /* Go over every partition of the with loop, copy the body code
     * and apply each withop. */
    while (partn != NULL) {
        res = ForEachElement (res, wln, partn, arg_info);
        partn = PART_NEXT (partn);
    }

    /* Finalize the unrolling for every withop. */
    withop = WITH_WITHOP (wln);
    while (withop != NULL) {
        switch (NODE_TYPE (withop)) {
        case N_genarray:
            res = FinalizeGenarray (res, withop, arg_info);
            break;
        case N_modarray:
            res = FinalizeModarray (res, withop, arg_info);
            break;
        case N_fold:
            res = FinalizeFold (res, withop, arg_info);
            break;
        case N_break:
            /* no-op */
            break;
        default:
            DBUG_ASSERT (0, "unhandled with-op");
        }
        withop = WITHOP_NEXT (withop);
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
    node *partn, *genn, *op;

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
    while (ok && op != NULL) {
        switch (NODE_TYPE (op)) {
        case N_genarray:
            ok = CheckUnrollGenarray (wln, arg_info);
            break;
        case N_modarray:
            ok = CheckUnrollModarray (wln, arg_info);
            break;
        case N_fold:
            ok = CheckUnrollFold (wln);
            break;
        case N_break:
            /* no-op */
            break;
        default:
            DBUG_ASSERT (0, "unhandled with-op");
        }
        op = WITHOP_NEXT (op);
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

    if (global.optimize.dowlur) {
        TRAVpush (TR_wlur);

        info = MakeInfo ();

        syntax_tree = TRAVdo (syntax_tree, info);

        FreeInfo (info);

        TRAVpop ();
    }

    DBUG_RETURN (syntax_tree);
}
