/*
 *
 * $Log$
 * Revision 1.12  2004/11/10 18:27:29  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.11  2004/08/04 12:03:27  ktr
 * substituted eacc by emm
 *
 * Revision 1.10  2004/07/23 15:24:04  khf
 * changed flag for explicit accumulation from ktr to eacc
 *
 * Revision 1.9  2004/07/22 17:36:31  khf
 * support for explicit accumulate (only if ktr is activated) added
 *
 * Revision 1.8  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.7  2004/07/07 15:43:36  mwe
 * last changes undone (all changes connected to new type representation with ntype*)
 *
 * Revision 1.5  2003/06/17 13:36:42  dkr
 * bug in ForEachElement() fixed:
 * WLUR works for empty WL-shape as well now
 *
 * Revision 1.4  2003/06/11 21:47:29  ktr
 * Added support for multidimensional arrays.
 *
 * Revision 1.3  2002/10/10 23:55:46  dkr
 * another bug in CountElements() fixed ...
 *
 * Revision 1.2  2002/10/09 02:05:34  dkr
 * bug in CountElements() fixed
 *
 * Revision 1.1  2002/10/08 22:10:04  dkr
 * Initial revision
 *
 *
 * created from WLUnroll.c, Revision 3.11 on 2002/10/10 by dkr
 *
 */

/*******************************************************************************

  This file make the following functions available:
  - Check whether WL (genarray, modarray, fold) can be unrolled
  - Execution of WL unrolling (genarray, modarray, fold)

  Theses functions are called from SSALUR.c and SSAWLUnroll.c

 *******************************************************************************/

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"

#include "optimize.h"
#include "Inline.h"
#include "DupTree.h"
#include "SSAWithloopFolding.h"
#include "SSAWLUnroll.h"

/*
 * INFO structure and macros
 */

#include "SSALUR_info.h"

/* opfun is a higher oder function called from within ForEachElementHelp()
 * to create explicit code for one single array element. opfun have the
 * following values:
 *   - CreateModGenarray()
 *   - CreateFodl()
 *
 * The args of opfun are stored in opfunarg. Both variables are global to
 * reduce function arguments.
 *
 *
 * Structure of functions in this file:
 * ------------------------------------
 *
 * CheckUnrollFold               CheckUnrollGenarray               CheckUnrollModarray
 *       |                                 |                                |
 * DoUnrollFold                  DoUnrollGenarray                  DoUnrollModarray
 *                \                        |                      /
 *                 \----------------ForEachElement---------------/ | |
 * | ForEachElementHelp                                       \|/ /                \
 * ` CreateFold     CreateModGenarray      (higher order functions) \                /
 *                                 CreateBottomCode
 *
 */

typedef node *(*funp) (node *, node *);

funp opfun;
void **opfunarg;

/******************************************************************************
 *
 * function:
 *   node *CreateBodyCode
 *
 * description:
 *   Duplicate the code behind the N_Npart node and insert index variables.
 *
 ******************************************************************************/

static node *
CreateBodyCode (node *partn, node *index)
{
    node *res, *letn, *coden;
    ids *_ids;

    DBUG_ENTER ("CreateBodyCode");

    coden = NPART_CODE (partn);
    if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden))))
        res = NULL;
    else
        res = DupTree (BLOCK_INSTR (NCODE_CBLOCK (coden)));

    /* index vector */
    letn = MakeLet (DupTree (index), DupOneIds (NPART_VEC (partn)));
    res = MakeAssign (letn, res);

    /* index scalars */
    _ids = NPART_IDS (partn);
    index = ARRAY_AELEMS (index);

    while (_ids) {
        letn = MakeLet (DupTree (EXPRS_EXPR (index)), DupOneIds (_ids));
        res = MakeAssign (letn, res);

        index = EXPRS_NEXT (index);
        _ids = IDS_NEXT (_ids);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *CreateModGenarray(node *assignn, node *index)
 *
 * description:
 *   Create unrolled code for one index element.
 *   This is an opfun function. Further parameters in opfunarg.
 *
 ******************************************************************************/

static node *
CreateModGenarray (node *assignn, node *index)
{
    node *exprs, *letexpr, *cexpr, *tmpn, *bodyn;
    ids *array;
    node *partn;

    DBUG_ENTER ("CreateModGenarray");

    partn = opfunarg[0]; /* (node*) */
    array = opfunarg[1]; /* (ids*) */

    bodyn = CreateBodyCode (partn, index);

    /* create prf modarray */
    cexpr = NCODE_CEXPR (NPART_CODE (partn));
    tmpn = MakeId (StringCopy (IDS_NAME (array)), IDS_MOD (array), ST_regular);
    ID_VARDEC (tmpn) = IDS_VARDEC (array);
    exprs = MakeExprs (tmpn, MakeExprs (index, MakeExprs (DupTree (cexpr), NULL)));

    letexpr = MakePrf (F_modarray, exprs);

    assignn = MakeAssign (MakeLet (letexpr, DupOneIds (array)), assignn);

    /* append assignn to bodyn */
    if (bodyn) {
        tmpn = bodyn;
        while (ASSIGN_NEXT (tmpn)) {
            tmpn = ASSIGN_NEXT (tmpn);
        }
        ASSIGN_NEXT (tmpn) = assignn;
    } else
        bodyn = assignn;

    DBUG_RETURN (bodyn);
}

/******************************************************************************
 *
 * function:
 *   node *CreateFold( node *assignn, node *index)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
CreateFold (node *assignn, node *index)
{
    node *partn, *cexpr, *withop, *fundef, *accvar, *funap, *assigns, *bodyn, *tmp;
    ids *acc;
    bool F_accu_found = FALSE;

    DBUG_ENTER ("CreateFold");

    partn = opfunarg[0]; /* N_Npart */

    if (!emm) {

        acc = opfunarg[1];    /* (ids*) */
        cexpr = opfunarg[2];  /* (node*) */
        withop = opfunarg[3]; /* N_withop */
        fundef = opfunarg[4]; /* N_fundef */

        /*
         * create assign-chain for the folding-operation
         * and prepand it to assignn:
         */

        /*
         * first, we create a function application of the form:
         *    <acc> = <fun>( <acc>, <cexpr>);
         * where
         *    <acc>   is the accumulator variable (can be found in 'opfunarg[1]')
         *    <fun>   is the name of the (artificially introduced) folding-fun
         *              (can be found via 'NWITHOP_FUN( opfunarg[3])')
         *    <cexpr> is the expression in the operation part
         *              (can be found in 'opfunarg[2]')
         */

        accvar = MakeId (StringCopy (IDS_NAME (acc)), IDS_MOD (acc), ST_regular);
        ID_VARDEC (accvar) = IDS_VARDEC (acc);

        funap = MakeAp (StringCopy (NWITHOP_FUN (withop)), NWITHOP_MOD (withop),
                        MakeExprs (accvar, MakeExprs (DupTree (cexpr), NULL)));
        AP_FUNDEF (funap) = NWITHOP_FUNDEF (withop);

        assigns = MakeAssign (MakeLet (funap, DupOneIds (acc)), NULL);

        /*
         * The following code is ment to inline the pseudo-funs for
         * fold-WLs generated by the typechecker in order to ease the
         * compilation process.
         */

        assigns = InlineSingleApplication (ASSIGN_INSTR (assigns), fundef);

        /*
         * Now, we prepand it to assignn:
         */
        assignn = AppendAssign (assigns, assignn);
    }

    /*
     * create assign-chain for the code-body
     * and prepand it to assignn:
     */
    bodyn = CreateBodyCode (partn, index);

    if (emm) {
        /*
         * case emm -> ExplicitAccumulation() was applied.
         * special handling of
         *       acc = accu(iv,n);
         *       b = <body>
         *       cexpr = op( acc, b);
         * needed.
         * acc = accu(iv,n)      -> acc = LHS of current WL(;
         * cexpr = op( acc, b)   -> LHS of current WL = op( acc, b);
         */

        acc = opfunarg[1];   /* ids* of current WL */
        cexpr = opfunarg[2]; /* (node*) */

        DBUG_ASSERT ((NODE_TYPE (bodyn) != N_empty), "BLOCK_INSTR is empty!");

        tmp = bodyn;
        while (tmp != NULL) {
            if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
                && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {
                ASSIGN_RHS (tmp) = FreeNode (ASSIGN_RHS (tmp));
                ASSIGN_RHS (tmp) = DupIds_Id (acc);
                F_accu_found = TRUE;
            }
            if ((ID_AVIS (cexpr) == IDS_AVIS (ASSIGN_LHS (tmp)))
                && (F_accu_found == TRUE)) {
                ASSIGN_LHS (tmp) = FreeOneIds (ASSIGN_LHS (tmp));
                ASSIGN_LHS (tmp) = DupOneIds (acc);
            }
            tmp = ASSIGN_NEXT (tmp);
        }

        DBUG_ASSERT ((F_accu_found), "F_accu not found!");
    }

    if (bodyn != NULL) {
        assignn = AppendAssign (bodyn, assignn);
    }

    DBUG_RETURN (assignn);
}

/******************************************************************************
 *
 * function:
 *   node *ForEachElementHelp()
 *
 * description:
 *   See ForEachElement().
 *
 ******************************************************************************/

static node *
ForEachElementHelp (int *l, int *u, int *s, int *w, int dim, int maxdim, node *assignn)
{
    int count, act_w, i;
    static int ind[SHP_SEG_SIZE];
    node *index;
    types *type;
    shpseg *shpseg;

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
                index = MakeExprs (MakeNum (ind[i - 1]), index);
            }
            index = MakeFlatArray (index);
            /* nums struct is freed inside MakeShpseg() */
#ifdef MWE_NTYPE_READY
            ARRAY_NTYPE (index)
              = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, maxdim));
#else
            shpseg = MakeShpseg (MakeNums (maxdim, NULL));
            type = MakeTypes (T_int, 1, shpseg, NULL, NULL);
            ARRAY_TYPE (index) = type;
#endif
            assignn = opfun (assignn, index);
        } else {
            assignn = ForEachElementHelp (l, u, s, w, dim + 1, maxdim, assignn);
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
 *   node *ForEachElement(node *partn, node *assignn)
 *
 * description:
 *   Calls function opfun for every index of the generator given in partn.
 *
 ******************************************************************************/

static node *
ForEachElement (node *partn, node *assignn)
{
    node *res;
    node *index;
    int maxdim, *l, *u, *s, *w;

    DBUG_ENTER ("ForEachElement");

    maxdim = IDS_SHAPE (NPART_VEC (partn), 0);
    l = u = s = w = NULL;
    res = NULL;

    SSAArrayST2ArrayInt (NGEN_BOUND1 (NPART_GEN (partn)), &l, maxdim);
    SSAArrayST2ArrayInt (NGEN_BOUND2 (NPART_GEN (partn)), &u, maxdim);
    if (NGEN_STEP (NPART_GEN (partn))) {
        SSAArrayST2ArrayInt (NGEN_STEP (NPART_GEN (partn)), &s, maxdim);
    }
    if (NGEN_WIDTH (NPART_GEN (partn))) {
        SSAArrayST2ArrayInt (NGEN_WIDTH (NPART_GEN (partn)), &w, maxdim);
    }

    if (maxdim == 0) {
        /* create index */
        index = MakeFlatArray (NULL);
        /* nums struct is freed inside MakeShpseg() */
#ifdef MWE_NTYPE_READY
        ARRAY_NTYPE (index)
          = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, maxdim));
#else
        ARRAY_TYPE (index)
          = MakeTypes (T_int, 1, MakeShpseg (MakeNums (maxdim, NULL)), NULL, NULL);
#endif

        res = opfun (assignn, index);
    } else {
        res = ForEachElementHelp (l, u, s, w, 0, maxdim, assignn);
    }

    l = Free (l);
    u = Free (u);
    s = Free (s);
    w = Free (w);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int CountElements(node *genn)
 *
 * description:
 *   counts number of specified elements by generator node genn.
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

    const_l = COAST2Constant (NGEN_BOUND1 (genn));
    l = COGetDataVec (const_l);
    DBUG_ASSERT ((COGetDim (const_l) == 1), "inconsistant wl bounds found!");
    dim = SHGetExtent (COGetShape (const_l), 0);

    const_u = COAST2Constant (NGEN_BOUND2 (genn));
    u = COGetDataVec (const_u);
    DBUG_ASSERT ((SHGetExtent (COGetShape (const_u), 0) == dim),
                 "inconsistant wl bounds found!");

    if (NGEN_STEP (genn) != NULL) {
        const_s = COAST2Constant (NGEN_STEP (genn));
        s = COGetDataVec (const_s);
        DBUG_ASSERT ((SHGetExtent (COGetShape (const_s), 0) == dim),
                     "inconsistant wl bounds found!");
    } else {
        const_s = NULL;
        s = NULL;
    }

    if (NGEN_WIDTH (genn) != NULL) {
        const_w = COAST2Constant (NGEN_WIDTH (genn));
        w = COGetDataVec (const_w);
        DBUG_ASSERT ((SHGetExtent (COGetShape (const_w), 0) == dim),
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
            elts = wlunrnum + 1;
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

    const_l = COFreeConstant (const_l);
    const_u = COFreeConstant (const_u);
    if (const_s != NULL) {
        const_s = COFreeConstant (const_s);
    }
    if (const_w != NULL) {
        const_w = COFreeConstant (const_w);
    }

    DBUG_RETURN (elts);
}

/******************************************************************************
 *
 * function:
 *   int SSACheckUnrollModarray(node *wln)
 *
 * description:
 *   Checks if this modarray WL can be unrolled.
 *   Multiple N_Npart nodes, which are not the identity of the base array,
 *   may be unrolled simultaneously. These N_Npart nodes are marked in
 *   NPART_COPY
 *
 ******************************************************************************/

int
SSACheckUnrollModarray (node *wln)
{
    int ok, elts;
    node *partn, *genn, *coden, *tmpn, *exprn;

    DBUG_ENTER ("SSACheckUnrollModarray");

    /* check all N_parts.
     All bounds (step, width) have to be constant. Count the number of
     elements which do NOT just copy the original array
     (e.g. body = {__flat = A[iv]} ) */

    partn = NWITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = NPART_GEN (partn);
    ok = (COIsConstant (NGEN_BOUND1 (genn)) && COIsConstant (NGEN_BOUND2 (genn))
          && (!NGEN_STEP (genn) || COIsConstant (NGEN_STEP (genn)))
          && (!NGEN_WIDTH (genn) || COIsConstant (NGEN_WIDTH (genn))));

    while (ok && partn) {
        genn = NPART_GEN (partn);
        /* check if code is a copy of the original array and set NPART_COPY
           for later usage in SSADoUnrollModarray().

             B = new_with
               ([ 0 ] <= __flat_1_iv=[__flat_0_i] < [ 3 ]) {
                  __wlt_4 = sel( __flat_1_iv, A );
               } : __wlt_4,
               ...more parts...
             modarray( A);

           We need DCR to be done before to detect identity written by the
           programmer. */

        coden = NPART_CODE (partn);
        if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden)))) {
            NPART_COPY (partn) = FALSE;
        } else {
            tmpn = ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            NPART_COPY (partn)
              = (N_let == NODE_TYPE (tmpn)
                 && !strcmp (ID_NAME (NCODE_CEXPR (coden)), IDS_NAME (LET_IDS (tmpn)))
                 && N_prf == NODE_TYPE (exprn) && F_sel == PRF_PRF (exprn)
                 && N_id == NODE_TYPE (PRF_ARG1 (exprn))
                 && !strcmp (IDS_NAME (NPART_VEC (partn)), ID_NAME (PRF_ARG1 (exprn)))
                 && N_id == NODE_TYPE (PRF_ARG2 (exprn))
                 && !strcmp (ID_NAME (NWITH_ARRAY (wln)), ID_NAME (PRF_ARG2 (exprn))));
        }

        if (!NPART_COPY (partn)) {
            elts += CountElements (genn);
        }

        partn = NPART_NEXT (partn);
    }

    if (ok && (elts > wlunrnum)) {
        ok = 0;
        if (elts <= 32) {
            /*
             * Most with-loops can easily be unrolled.
             * So, we only want to see a warning for small ones.
             */
            NOTE (("WLUR: -maxwlur %d would unroll fold with-loop", elts));
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *SSADoUnrollModarray(node *wln, info *arg_info)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
 *
 ******************************************************************************/

node *
SSADoUnrollModarray (node *wln, info *arg_info)
{
    node *partn, *res;
    void *arg[2];
    node *letn;

    DBUG_ENTER ("SSADoUnrollModarray");

    partn = NWITH_PART (wln);

    res = NULL;
    while (partn) {
        if (!NPART_COPY (partn)) {
            /* unroll this part */
            opfun = CreateModGenarray;
            arg[0] = partn;                                                  /* (node*) */
            arg[1] = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info))); /* (ids*) */
            opfunarg = arg;
            res = ForEachElement (partn, res);
        }

        partn = NPART_NEXT (partn);
    }

    /* finally add Dupilcation of new array name */
    letn = MakeLet (DupTree (NWITH_ARRAY (wln)),
                    DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int SSACheckUnrollGenarray( node *wln, info *arg_info)
 *
 * description:
 *   Unrolling of arrays is done if number of array elements is smaller
 *   than wlunrnum.
 *
 ******************************************************************************/

int
SSACheckUnrollGenarray (node *wln, info *arg_info)
{
    int ok, length;
    node *genn;
    types *type;

    DBUG_ENTER ("SSACheckUnrollGenarray");

#ifdef MWE_NTYPE_READY
    length = SHGetUnrLen (TYGetShape (
      AVIS_TYPE (IDS_AVIS (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))))));
#else
    type = IDS_TYPE (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info))));
    length = GetTypesLength (type);
#endif
    /*
     * Everything constant?
     * If the first part is constant, all others are constant, too.
     */
    genn = NPART_GEN (NWITH_PART (wln));
    ok = ((length >= 0) && COIsConstant (NGEN_BOUND1 (genn))
          && COIsConstant (NGEN_BOUND2 (genn))
          && ((NGEN_STEP (genn) == NULL) || COIsConstant (NGEN_STEP (genn)))
          && ((NGEN_WIDTH (genn) == NULL) || COIsConstant (NGEN_WIDTH (genn))));

    if (ok && (length > wlunrnum)) {
        ok = 0;
        if (length <= 32) {
            NOTE (("WLUR: -maxwlur %d would unroll genarray with-loop", length));
            /*
             * Most with-loops can easily be unrolled.
             * So, we only want to see a warning for small ones.
             */
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *SSADoUnrollGenarray(node *wln, info *arg_info)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
 *
 ******************************************************************************/

node *
SSADoUnrollGenarray (node *wln, info *arg_info)
{
    node *partn, *res;
    node *letn, *let_expr;
    void *arg[2];
    ids *arrayname;
    types *type;

    DBUG_ENTER ("SSADoUnrollGenarray");

    partn = NWITH_PART (wln);
    arrayname = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)));

    res = NULL;
    while (partn) {
        opfun = CreateModGenarray;
        arg[0] = partn;     /* (node*) */
        arg[1] = arrayname; /* (ids*) */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = NPART_NEXT (partn);
    }

    /*
     * finally add   arrayname = reshape( ..., [0,...,0])
     */
#ifdef MWE_NTYPE_READY
    let_expr = CreateZeroFromType (NULL,
                                   AVIS_TYPE (LET_AVIS (
                                     ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))),
                                   TRUE, INFO_SSALUR_FUNDEF (arg_info));
#else
    type = LET_TYPE (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)));
    let_expr = CreateZeroFromType (type, TRUE, INFO_SSALUR_FUNDEF (arg_info));
#endif
    letn = MakeLet (let_expr, DupOneIds (arrayname));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int SSACheckUnrollFold( node *wln)
 *
 * description:
 *   Unrolling of fold-WLs is done if the total number of function calls is
 *   less or equal wlunrnum.
 *
 ******************************************************************************/

int
SSACheckUnrollFold (node *wln)
{
    int ok, elts;
    node *partn, *genn;

    DBUG_ENTER ("SSACheckUnrollFold");

    /* check all N_parts.
       All bounds (step, width) have to be constant. */

    partn = NWITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = NPART_GEN (partn);
    ok = (COIsConstant (NGEN_BOUND1 (genn)) && COIsConstant (NGEN_BOUND2 (genn))
          && (!NGEN_STEP (genn) || COIsConstant (NGEN_STEP (genn)))
          && (!NGEN_WIDTH (genn) || COIsConstant (NGEN_WIDTH (genn))));

    while (ok && (partn != NULL)) {
        elts += CountElements (NPART_GEN (partn));
        partn = NPART_NEXT (partn);
    }

    if (ok && (elts > wlunrnum)) {
        ok = 0;
        if (elts <= 32) {
            NOTE (("WLUR: -maxwlur %d would unroll fold with-loop", elts));
            /*
             * Most with-loops can easily be unrolled.
             * So, we only want to see a warning for small ones.
             */
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *SSADoUnrollFold(node *wln, info *arg_info)
 *
 * description:
 *   INFO_SSALUR_FUNDEF( arg_info) contains the pointer to the N_fundef node
 *     where this WL is situated in.
 *
 *   Unroll fold WL:
 *     res = neutral;
 *     wl_expr = ...            \  repeat for
 *     res = f(res, wl_expr);   /  every element
 *
 ******************************************************************************/

node *
SSADoUnrollFold (node *wln, info *arg_info)
{
    node *partn, *res;
    void *arg[5];
    node *letn;

    DBUG_ENTER ("SSADoUnrollFold");

    partn = NWITH_PART (wln);

    res = NULL;
    while (partn != NULL) {
        /* unroll this part */
        opfun = CreateFold;
        arg[0] = partn; /* N_Npart node */
        arg[1] = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info))); /* (ids*)  */
        arg[2] = NCODE_CEXPR (NPART_CODE (partn));                       /* (node*) */
        arg[3] = NWITH_WITHOP (wln);            /* N_Nwithop node */
        arg[4] = INFO_SSALUR_FUNDEF (arg_info); /* N_fundef node */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = NPART_NEXT (partn);
    }

    /* finally add initialisation of accumulator with neutral element. */
    letn = MakeLet (DupTree (NWITH_NEUTRAL (wln)),
                    DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))));

    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}
