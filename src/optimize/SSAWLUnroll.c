/*
 *
 * $Log$
 * Revision 1.16  2004/12/08 18:00:42  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.15  2004/11/26 18:26:53  mwe
 * bug fix
 *
 * Revision 1.13  2004/11/11 18:57:15  khf
 * CreateFold(): removed non emm part,
 *               add new assign instead of replace cexpr
 *
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

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "node_basic.h"
#include "globals.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "new_types.h"
#include "shape.h"

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
    node *_ids;

    DBUG_ENTER ("CreateBodyCode");

    coden = PART_CODE (partn);
    if (N_empty == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (coden))))
        res = NULL;
    else
        res = DUPdoDupTree (BLOCK_INSTR (CODE_CBLOCK (coden)));

    /* index vector */
    letn = TBmakeLet (DUPdoDupNode (PART_VEC (partn)), DUPdoDupTree (index));
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
    node *array;
    node *partn;

    DBUG_ENTER ("CreateModGenarray");

    partn = opfunarg[0]; /* (node*) */
    array = opfunarg[1]; /* (ids*) */

    bodyn = CreateBodyCode (partn, index);

    /* create prf modarray */
    cexpr = CODE_CEXPRS (PART_CODE (partn));
    tmpn = TBmakeId (IDS_AVIS (array));

    exprs
      = TBmakeExprs (tmpn, TBmakeExprs (index, TBmakeExprs (DUPdoDupTree (cexpr), NULL)));

    letexpr = TBmakePrf (F_modarray, exprs);

    assignn = TBmakeAssign (TBmakeLet (letexpr, DUPdoDupNode (array)), assignn);

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
    node *partn, *cexpr, *bodyn, *tmp, *letn;
    node *acc;
    bool F_accu_found = FALSE;

    DBUG_ENTER ("CreateFold");

    partn = opfunarg[0]; /* N_Npart */

    /*
     * create assign-chain for the code-body
     * and prepand it to assignn:
     */
    bodyn = CreateBodyCode (partn, index);

    /*
     * special handling of
     *       acc = accu(iv,n);
     *       b = <body>
     *       cexpr = op( acc, b);
     * needed.
     * acc = accu(iv,n)  -> acc = LHS of current WL(;
     * append new last assignment: LHS of current WL = cexpr;
     */

    acc = opfunarg[1];   /* ids* of current WL */
    cexpr = opfunarg[2]; /* (node*) */

    DBUG_ASSERT ((NODE_TYPE (bodyn) != N_empty), "BLOCK_INSTR is empty!");

    tmp = bodyn;
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

    if (bodyn != NULL) {
        assignn = TCappendAssign (bodyn, assignn);
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
            index = TCmakeFlatArray (index);

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

    maxdim = IDS_SHAPE (PART_VEC (partn), 0);
    l = u = s = w = NULL;
    res = NULL;

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
        index = TCmakeFlatArray (NULL);
        /* nums struct is freed inside MakeShpseg() */

        res = opfun (assignn, index);
    } else {
        res = ForEachElementHelp (l, u, s, w, 0, maxdim, assignn);
    }

    l = ILIBfree (l);
    u = ILIBfree (u);
    s = ILIBfree (s);
    w = ILIBfree (w);

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
WLUcheckUnrollModarray (node *wln)
{
    int ok, elts;
    node *partn, *genn, *coden, *tmpn, *exprn;

    DBUG_ENTER ("WLUcheckUnrollModarray");

    /* check all N_parts.
     All bounds (step, width) have to be constant. Count the number of
     elements which do NOT just copy the original array
     (e.g. body = {__flat = A[iv]} ) */

    partn = WITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = PART_GENERATOR (partn);
    ok = (COisConstant (GENERATOR_BOUND1 (genn)) && COisConstant (GENERATOR_BOUND2 (genn))
          && (!GENERATOR_STEP (genn) || COisConstant (GENERATOR_STEP (genn)))
          && (!GENERATOR_WIDTH (genn) || COisConstant (GENERATOR_WIDTH (genn))));

    while (ok && partn) {
        genn = PART_GENERATOR (partn);
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

        coden = PART_CODE (partn);
        if (N_empty == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (coden)))) {
            PART_ISCOPY (partn) = FALSE;
        } else {
            tmpn = ASSIGN_INSTR (BLOCK_INSTR (CODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            PART_ISCOPY (partn)
              = (N_let == NODE_TYPE (tmpn)
                 && !strcmp (ID_NAME (EXPRS_EXPR (CODE_CEXPRS (coden))),
                             IDS_NAME (LET_IDS (tmpn)))
                 && N_prf == NODE_TYPE (exprn) && F_sel == PRF_PRF (exprn)
                 && N_id == NODE_TYPE (PRF_ARG1 (exprn))
                 && !strcmp (IDS_NAME (PART_VEC (partn)), ID_NAME (PRF_ARG1 (exprn)))
                 && N_id == NODE_TYPE (PRF_ARG2 (exprn))
                 && !strcmp (ID_NAME (MODARRAY_ARRAY (WITH_WITHOP (wln))),
                             ID_NAME (PRF_ARG2 (exprn))));
        }

        if (!PART_ISCOPY (partn)) {
            elts += CountElements (genn);
        }

        partn = PART_NEXT (partn);
    }

    if (ok && (elts > global.wlunrnum)) {
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
WLUdoUnrollModarray (node *wln, info *arg_info)
{
    node *partn, *res;
    void *arg[2];
    node *letn;

    DBUG_ENTER ("SSADoUnrollModarray");

    partn = WITH_PART (wln);

    res = NULL;
    while (partn) {
        if (!PART_ISCOPY (partn)) {
            /* unroll this part */
            opfun = CreateModGenarray;
            arg[0] = partn;                                                  /* (node*) */
            arg[1] = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info))); /* (ids*) */
            opfunarg = arg;
            res = ForEachElement (partn, res);
        }

        partn = PART_NEXT (partn);
    }

    /* finally add Dupilcation of new array name */
    letn
      = TBmakeLet (DUPdoDupNode (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))),
                   DUPdoDupTree (MODARRAY_ARRAY (WITH_WITHOP (wln))));
    res = TBmakeAssign (letn, res);

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
WLUcheckUnrollGenarray (node *wln, info *arg_info)
{
    int ok, length;
    node *genn;

    DBUG_ENTER ("SSACheckUnrollGenarray");

    length = SHgetUnrLen (TYgetShape (
      AVIS_TYPE (IDS_AVIS (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))))));

    /*
     * Everything constant?
     * If the first part is constant, all others are constant, too.
     */
    genn = PART_GENERATOR (WITH_PART (wln));
    ok = ((length >= 0) && COisConstant (GENERATOR_BOUND1 (genn))
          && COisConstant (GENERATOR_BOUND2 (genn))
          && ((GENERATOR_STEP (genn) == NULL) || COisConstant (GENERATOR_STEP (genn)))
          && ((GENERATOR_WIDTH (genn) == NULL) || COisConstant (GENERATOR_WIDTH (genn))));

    if (ok && (length > global.wlunrnum)) {
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
WLUdoUnrollGenarray (node *wln, info *arg_info)
{
    node *partn, *res;
    node *letn, *let_expr;
    void *arg[2];
    node *arrayname;

    DBUG_ENTER ("WLUdoUnrollGenarray");

    partn = WITH_PART (wln);
    arrayname = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)));

    res = NULL;
    while (partn) {
        opfun = CreateModGenarray;
        arg[0] = partn;     /* (node*) */
        arg[1] = arrayname; /* (ids*) */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = PART_NEXT (partn);
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
    /*type = LET_TYPE( ASSIGN_INSTR( INFO_SSALUR_ASSIGN( arg_info)));*/
    let_expr = TCcreateZeroFromType (/*type*/
                                     TYtype2OldType (AVIS_TYPE (IDS_AVIS (LET_IDS (
                                       ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))))),
                                     TRUE, INFO_SSALUR_FUNDEF (arg_info));
#endif
    letn = TBmakeLet (DUPdoDupNode (arrayname), let_expr);
    res = TBmakeAssign (letn, res);

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
WLUcheckUnrollFold (node *wln)
{
    int ok, elts;
    node *partn, *genn;

    DBUG_ENTER ("SSACheckUnrollFold");

    /* check all N_parts.
       All bounds (step, width) have to be constant. */

    partn = WITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = PART_GENERATOR (partn);
    ok = (COisConstant (GENERATOR_BOUND1 (genn)) && COisConstant (GENERATOR_BOUND2 (genn))
          && ((GENERATOR_STEP (genn) != 0) || COisConstant (GENERATOR_STEP (genn)))
          && ((GENERATOR_WIDTH (genn) != 0) || COisConstant (GENERATOR_WIDTH (genn))));

    while (ok && (partn != NULL)) {
        elts += CountElements (PART_GENERATOR (partn));
        partn = PART_NEXT (partn);
    }

    if (ok && (elts > global.wlunrnum)) {
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
 *   node *WLUdoUnrollFold(node *wln, info *arg_info)
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
WLUdoUnrollFold (node *wln, info *arg_info)
{
    node *partn, *res;
    void *arg[5];
    node *letn;

    DBUG_ENTER ("WLUdoUnrollFold");

    partn = WITH_PART (wln);

    res = NULL;
    while (partn != NULL) {
        /* unroll this part */
        opfun = CreateFold;
        arg[0] = partn; /* N_Npart node */
        arg[1] = LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info))); /* (ids*)  */
        arg[2] = CODE_CEXPRS (PART_CODE (partn));                        /* (node*) */
        arg[3] = WITH_WITHOP (wln);             /* N_Nwithop node */
        arg[4] = INFO_SSALUR_FUNDEF (arg_info); /* N_fundef node */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = PART_NEXT (partn);
    }

    /* finally add initialisation of accumulator with neutral element. */
    letn
      = TBmakeLet (DUPdoDupNode (LET_IDS (ASSIGN_INSTR (INFO_SSALUR_ASSIGN (arg_info)))),
                   DUPdoDupTree (FOLD_NEUTRAL (WITH_WITHOP (wln))));
    res = TBmakeAssign (letn, res);

    DBUG_RETURN (res);
}
