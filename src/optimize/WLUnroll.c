/*         $Id$
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:39  sacbase
 * new release made
 *
 * Revision 1.7  1998/11/19 12:53:45  srs
 * WLUnrolling now works with the new WL-body-syntax (empty
 * bodies contain N_empty nodes).
 *
 * Revision 1.6  1998/07/16 17:22:44  sbs
 * unrolling of WL-fold-loops now inlines the pseudo-funs generated
 * by the typechecker!
 *
 * Revision 1.5  1998/07/16 11:39:13  sbs
 * tried to tackle the inlining of pseudo-fold-funs for unrolled WLs
 * commented it out again since problems concerning the masks
 * arose....8-(((((
 *
 * Revision 1.4  1998/06/02 17:09:17  sbs
 * some comments added
 *
 * Revision 1.3  1998/05/30 19:44:46  dkr
 * fixed a bug in CreateFold:
 *   funname is no longer shared
 *
 * Revision 1.2  1998/05/15 14:43:16  srs
 * functions for WL unrolling
 *
 * Revision 1.1  1998/05/13 13:47:44  srs
 * Initial revision
 *
 *
 */

/*******************************************************************************

  This file make the following functions available:
  - Check whether WL (genarray, modarray, fold) can be unrolled
  - Execution of WL unrolling (genarray, modarray, fold)

  Theses functions are called from Unroll.c

 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "typecheck.h"
#include "compile.h"

#include "optimize.h"
#include "Inline.h"
#include "DupTree.h"
#include "WithloopFolding.h"

/* opfun is a higher oder function called from within ForEachElementHelp()
   to create explicit code for one single array element. opfun have the
   following values:
     - CreateModGenarray()
     - CreateFodl()

   The args of opfun are stored in opfunarg. Both variables are global to
   reduce function arguments.


   Structure of functions in this file:
   ------------------------------------

   CheckUnrollFold               CHeckUnrollGenarray               CheckUnrollModarray
         |                                 |                                |
   DoUnrollFold                  DoUnrollGenarray                  DoUnrollModarray
                  \                        |                      /
                   \----------------ForEachElement---------------/ | |
   | ForEachElementHelp                                       \|/ /                \
   ` CreateFold     CreateModGenarray      (higher order functions) \                /
                                   CreateBottomCode

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
 *
 ******************************************************************************/

node *
CreateBodyCode (node *partn, node *index)
{
    node *res, *letn, *coden;
    ids *_ids;

    DBUG_ENTER ("CreateBodyCode");

    coden = NPART_CODE (partn);
    if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden))))
        res = NULL;
    else
        res = DupTree (BLOCK_INSTR (NCODE_CBLOCK (coden)), NULL);

    /* index vector */
    if (coden->mask[1][IDS_VARNO (NPART_VEC (partn))]) {
        letn = MakeLet (DupTree (index, NULL), DupOneIds (NPART_VEC (partn), NULL));
        res = MakeAssign (letn, res);
    }

    /* index scalars */
    _ids = NPART_IDS (partn);
    index = ARRAY_AELEMS (index);

    while (_ids) {
        if (coden->mask[1][IDS_VARNO (_ids)]) {
            letn = MakeLet (DupTree (EXPRS_EXPR (index), NULL), DupOneIds (_ids, NULL));
            res = MakeAssign (letn, res);
        }

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

node *
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
    tmpn
      = MakeId (StringCopy (IDS_NAME (array)), StringCopy (IDS_MOD (array)), ST_regular);
    ID_VARDEC (tmpn) = IDS_VARDEC (array);
    exprs = MakeExprs (tmpn, MakeExprs (index, MakeExprs (DupTree (cexpr, NULL), NULL)));

    letexpr = MakePrf (F_modarray, exprs);

    assignn = MakeAssign (MakeLet (letexpr, DupOneIds (array, NULL)), assignn);

    /* append assignn to bodyn */
    if (bodyn) {
        tmpn = bodyn;
        while (ASSIGN_NEXT (tmpn))
            tmpn = ASSIGN_NEXT (tmpn);
        ASSIGN_NEXT (tmpn) = assignn;
    } else
        bodyn = assignn;

    DBUG_RETURN (bodyn);
}

/******************************************************************************
 *
 * function:
 *   node *CreateFold(node *assignn, node *index)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
CreateFold (node *assignn, node *index)
{
    node *wln, *partn, *assigns, *bodyn, *cexpr, *fun, *funap, *accvar;
    node *fundef;
    ids *acc;

    DBUG_ENTER ("CreateFold");

    wln = opfunarg[0];    /* (node*) */
    acc = opfunarg[1];    /* (ids*) */
    fundef = opfunarg[2]; /* N_fundef */

    DBUG_ASSERT ((NWITH_TYPE (wln) == WO_foldfun), "WO_foldfun expected!");

    /*
     * create assign-chain for the folding-operation
     * and prepand it to assignn:
     */
    /* foldfundef = NWITHOP_FUNDEF( NWITH_WITHOP( wln)); */
    /* assigns = GetFoldCode( foldfundef); */
    /* vardecs = GetFoldVardecs( foldfundef); */

    /*
     * first, we create a function application of the form:
     *    <acc> = <fun>( <acc>, <cexpr>);
     * where
     *    <acc> is the accumulator variable & can be found in opfunarg[1]
     *    <fun> is the name of the (artificially introduced) folding-fun
     *          & can be found via NWITH_WITHOP( wln)
     *    <cexpr> is the expression in the operation part
     *            & can be found via NCODE_CEXPR( NPART_CODE( NWITH_PART(wln)))
     */

    fun = NWITH_WITHOP (wln);
    cexpr = NCODE_CEXPR (NPART_CODE (NWITH_PART (wln)));

    accvar = MakeId (StringCopy (IDS_NAME (acc)), StringCopy (IDS_MOD (acc)), ST_regular);
    ID_VARDEC (accvar) = IDS_VARDEC (acc);

    funap = MakeAp (StringCopy (NWITHOP_FUN (fun)), StringCopy (NWITHOP_MOD (fun)),
                    MakeExprs (accvar, MakeExprs (DupTree (cexpr, NULL), NULL)));
    AP_FUNDEF (funap) = NWITHOP_FUNDEF (fun);

    assigns = MakeAssign (MakeLet (funap, DupOneIds (acc, NULL)), NULL);

    /*
     * The following code is ment to inline the pseudo-funs for
     * fold-WLs generated by the typechecker in order to ease the
     * compilation process.
     */

    inl_fun--; /* Do not count this inlining! */

    assigns = InlineSingleApplication (ASSIGN_INSTR (assigns), fundef);

    /*
     * Now, we prepand it to assignn:
     */

    assignn = AppendAssign (assigns, assignn);

    /*
     * create assign-chain for the code-body
     * and prepand it to assignn:
     */
    partn = NWITH_PART (wln);
    bodyn = CreateBodyCode (partn, index);

    if (bodyn)
        assignn = AppendAssign (bodyn, assignn);

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
 *
 ******************************************************************************/

node *
ForEachElementHelp (int *l, int *u, int *s, int *w, int dim, int maxdim, node *assignn)
{
    int count, act_w, i;
    static int ind[SHP_SEG_SIZE];
    node *index;
    types *type;
    shpseg *shpseg;

    DBUG_ENTER ("ForEachElementHelp");

    count = l[dim];
    act_w = 0;
    while (count + act_w < u[dim]) {
        ind[dim] = count + act_w;
        if (dim + 1 == maxdim) {
            /* create index */
            index = NULL;
            for (i = maxdim; i > 0; i--)
                index = MakeExprs (MakeNum (ind[i - 1]), index);
            index = MakeArray (index);
            /* nums struct is freed inside MakeShpseg. */
            shpseg = MakeShpseg (MakeNums (maxdim, NULL));
            type = MakeType (T_int, 1, shpseg, NULL, NULL);
            ARRAY_TYPE (index) = type;
            assignn = opfun (assignn, index);
        } else
            assignn = ForEachElementHelp (l, u, s, w, dim + 1, maxdim, assignn);

        /* advance to next element */
        if (w && act_w + 1 < w[dim])
            act_w++;
        else {
            act_w = 0;
            count += s ? s[dim] : 1;
        }
    }

    DBUG_RETURN (assignn);
}

/******************************************************************************
 *
 * function:
 *   node *ForEachElement(node *partn, funp opfun)
 *
 * description:
 *   Calls function opfun for every index of the generator given in partn.
 *
 *
 ******************************************************************************/

node *
ForEachElement (node *partn, node *assignn)
{
    node *res;
    int maxdim, *l, *u, *s, *w;

    DBUG_ENTER ("ForEachElement");

    maxdim = IDS_SHAPE (NPART_VEC (partn), 0);
    l = u = s = w = NULL;
    res = NULL;

    ArrayST2ArrayInt (NGEN_BOUND1 (NPART_GEN (partn)), &l, maxdim);
    ArrayST2ArrayInt (NGEN_BOUND2 (NPART_GEN (partn)), &u, maxdim);
    if (NGEN_STEP (NPART_GEN (partn)))
        ArrayST2ArrayInt (NGEN_STEP (NPART_GEN (partn)), &s, maxdim);
    if (NGEN_WIDTH (NPART_GEN (partn)))
        ArrayST2ArrayInt (NGEN_WIDTH (NPART_GEN (partn)), &w, maxdim);

    res = ForEachElementHelp (l, u, s, w, 0, maxdim, assignn);

    FREE (l);
    FREE (u);
    FREE (s);
    FREE (w);

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

#define ELT(n) NUM_VAL (EXPRS_EXPR (n))
int
CountElements (node *genn)
{
    int elts, tmp, d, m;
    node *l, *u, *s, *w;

    DBUG_ENTER ("CountElements");
    l = ARRAY_AELEMS (NGEN_BOUND1 (genn));
    u = ARRAY_AELEMS (NGEN_BOUND2 (genn));
    s = NGEN_STEP (genn);
    if (s)
        s = ARRAY_AELEMS (s);
    w = NGEN_WIDTH (genn);
    if (w)
        w = ARRAY_AELEMS (w);
    elts = 1;

    while (l) {
        tmp = 0;

        /* check step/width */
        if ((w && !s) || (w && ELT (w) < 1) || (s && w && ELT (s) < ELT (w))) {
            /* illegal */
            elts = wlunrnum + 1;
            break;
        }

        /* counts elements in this dimension */
        tmp = ELT (u) - ELT (l);
        if (s) {
            d = tmp / ELT (s);
            m = tmp % ELT (s);
            tmp = w ? (d * ELT (w)) : d;
            if (m)
                tmp += w ? (m > ELT (w) ? ELT (w) : m) : 1;
        }

        /* summarise elements over all dimensions. */
        elts *= tmp;

        /* next dimension */
        l = EXPRS_NEXT (l);
        u = EXPRS_NEXT (u);
        if (s)
            s = EXPRS_NEXT (s);
        if (w)
            w = EXPRS_NEXT (w);
    }

    DBUG_RETURN (elts);
}
#undef ELT

/******************************************************************************
 *
 * function:
 *   int CheckUnrollModarray(node *wln)
 *
 * description:
 *   Checks if this modarray WL can be unrolled.
 *   Multiple N_Npart nodes, which are not the identity of the base array,
 *   may be unrolled simultaneously. These N_Npart nodes are marked in
 *   NPART_COPY
 *
 ******************************************************************************/

int
CheckUnrollModarray (node *wln)
{
    int ok, elts;
    node *partn, *genn, *coden, *tmpn, *exprn;

    DBUG_ENTER ("CheckUnrollModarray");

    /* check all N_parts.
     All bounds (step, width) have to be constant. Count the number of
     elements which do NOT just copy the original array
     (e.g. body = {__flat = A[iv]} ) */

    partn = NWITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = NPART_GEN (partn);
    ok = (IsConstantArray (NGEN_BOUND1 (genn), N_num)
          && IsConstantArray (NGEN_BOUND2 (genn), N_num)
          && (!NGEN_STEP (genn) || IsConstantArray (NGEN_STEP (genn), N_num))
          && (!NGEN_WIDTH (genn) || IsConstantArray (NGEN_WIDTH (genn), N_num)));

    while (partn && ok) {
        genn = NPART_GEN (partn);
        /* check if code is a copy of the original array and set NPART_COPY
           for later usage in DoUnrollModarray().

             B = new_with
               ([ 0 ] <= __flat_1_iv=[__flat_0_i] < [ 3 ]) {
                  __wlt_4 = psi( __flat_1_iv, A );
               } : __wlt_4,
               ...more parts...
             modarray( A);

           We need DCR to be done before to detect identity written by the
           programmer. */

        coden = NPART_CODE (partn);
        if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden))))
            NPART_COPY (partn) = 0;
        else {
            tmpn = ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            NPART_COPY (partn)
              = (N_let == NODE_TYPE (tmpn)
                 && !strcmp (ID_NAME (NCODE_CEXPR (coden)), IDS_NAME (LET_IDS (tmpn)))
                 && N_prf == NODE_TYPE (exprn) && F_psi == PRF_PRF (exprn)
                 && N_id == NODE_TYPE (PRF_ARG1 (exprn))
                 && !strcmp (IDS_NAME (NPART_VEC (partn)), ID_NAME (PRF_ARG1 (exprn)))
                 && N_id == NODE_TYPE (PRF_ARG2 (exprn))
                 && !strcmp (ID_NAME (NWITHOP_ARRAY (NWITH_WITHOP (wln))),
                             ID_NAME (PRF_ARG2 (exprn))));
        }

        if (!NPART_COPY (partn))
            ok = (elts += CountElements (genn)) <= wlunrnum;

        partn = NPART_NEXT (partn);
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *DoUnrollModarray(node *wln)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
 *
 *
 ******************************************************************************/

node *
DoUnrollModarray (node *wln, node *arg_info)
{
    node *partn, *res;
    void *arg[2];
    node *letn;

    DBUG_ENTER ("DoUnrollModarray");

    partn = NWITH_PART (wln);

    res = NULL;
    while (partn) {
        if (!NPART_COPY (partn)) {
            /* unroll this part */
            opfun = CreateModGenarray;
            arg[0] = partn;                                               /* (node*) */
            arg[1] = LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))); /* (ids*) */
            opfunarg = arg;
            res = ForEachElement (partn, res);
        }

        partn = NPART_NEXT (partn);
    }

    /* finally add Dupilcation of new array name */
    letn
      = MakeLet (DupTree (NWITHOP_ARRAY (NWITH_WITHOP (wln)), NULL),
                 DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))), NULL));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int CheckUnrollGenarray(node *wln)
 *
 * description:
 *   Unrolling of arrays is done if number of array elements is smaller
 *   than wlunrnum.
 *
 ******************************************************************************/

int
CheckUnrollGenarray (node *wln, node *arg_info)
{
    int ok, length;
    node *genn;
    types *type;

    DBUG_ENTER ("CheckUnrollGenarray");

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = NPART_GEN (NWITH_PART (wln));
    ok = (IsConstantArray (NGEN_BOUND1 (genn), N_num)
          && IsConstantArray (NGEN_BOUND2 (genn), N_num)
          && (!NGEN_STEP (genn) || IsConstantArray (NGEN_STEP (genn), N_num))
          && (!NGEN_WIDTH (genn) || IsConstantArray (NGEN_WIDTH (genn), N_num)));

    type = IDS_TYPE (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))));
    GET_LENGTH (length, type);
    ok = ok && length <= wlunrnum;

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *DoUnrollGenarray(node *wln)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
 *
 *
 ******************************************************************************/

node *
DoUnrollGenarray (node *wln, node *arg_info)
{
    node *partn, *res;
    node *letn, *args, *let_expr;
    void *arg[2];
    ids *arrayname;
    int elements, i;
    simpletype stype;
    types *type;

    DBUG_ENTER ("DoUnrollGenarray");

    partn = NWITH_PART (wln);
    arrayname = LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info)));

    res = NULL;
    while (partn) {
        opfun = CreateModGenarray;
        arg[0] = partn;     /* (node*) */
        arg[1] = arrayname; /* (ids*) */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = NPART_NEXT (partn);
    }

    /* finally add arrayname = reshape(...,[0,...,0]) */
    /*
     * attention: the above reshape() is correct. But it seems that it
     * is not necessary anymore after the TC. reshape() is ignored in
     * compile phase.
     * If WLT is deactivated, not all elements of the WL are rewritten by
     * prf modarray. CF cannot handle prf reshape() so unrolling might fail
     * here. If we drop the reshape, further compilation should(!!!)
     * work without problems and CF can fold all elements.
     */
    args = DupTree (NWITHOP_SHAPE (NWITH_WITHOP (wln)), NULL);

    type = IDS_TYPE (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))));
    stype = TYPES_BASETYPE (type);

    elements = 1;
    for (i = 0; i < TYPES_DIM (type); i++)
        elements *= TYPES_SHAPE (type, i);

    /* drop reshape() */
    /*   args = MakeExprs(args, MakeExprs(MakeNullVec(elements, stype), NULL)); */
    /*   let_expr = MakePrf(F_reshape,args); */
    let_expr = MakeNullVec (elements, stype);

    letn = MakeLet (let_expr, DupOneIds (arrayname, NULL));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int CheckUnrollFold(node *wln)
 *
 * description:
 *   Unrolling of fold-WLs is done if the total number of function calls is
 *   less or equal wlunrnum.
 *
 ******************************************************************************/

int
CheckUnrollFold (node *wln)
{
    int ok, elts;
    node *partn, *genn;

    DBUG_ENTER ("CheckUnrollFold");

    /* check all N_parts.
       All bounds (step, width) have to be constant. */

    partn = NWITH_PART (wln);
    elts = 0;

    /* everything constant? If the first part is constant, all others are
       constant, too. */
    genn = NPART_GEN (partn);
    ok = (IsConstantArray (NGEN_BOUND1 (genn), N_num)
          && IsConstantArray (NGEN_BOUND2 (genn), N_num)
          && (!NGEN_STEP (genn) || IsConstantArray (NGEN_STEP (genn), N_num))
          && (!NGEN_WIDTH (genn) || IsConstantArray (NGEN_WIDTH (genn), N_num)));

    while (partn && ok) {
        ok = (elts += CountElements (NPART_GEN (partn))) <= wlunrnum;
        partn = NPART_NEXT (partn);
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   node *DoUnrollFold(node *wln, node *arg_info)
 *
 * description:
 *   INFO_UNR_FUNDEF( arg_info) contains the pointer to the N_fundef node
 *     where this WL is situated in.
 *
 *   Unroll fold WL:
 *     res = neutral;
 *     wl_expr = ...            \  repeat for
 *     res = f(res, wl_expr);   /  every element
 *
 ******************************************************************************/

node *
DoUnrollFold (node *wln, node *arg_info)
{
    node *partn, *res;
    void *arg[3];
    node *letn;

    DBUG_ENTER ("DoUnrollFold");

    partn = NWITH_PART (wln);

    res = NULL;
    while (partn) {
        /* unroll this part */
        opfun = CreateFold;
        arg[0] = wln;                                                 /* (node*) */
        arg[1] = LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))); /* (ids*) */
        arg[2] = INFO_UNR_FUNDEF (arg_info);                          /* N_fundef node */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = NPART_NEXT (partn);
    }

    /* finally add initialisation of accumulator with neutral element. */
    letn
      = MakeLet (DupTree (NWITHOP_NEUTRAL (NWITH_WITHOP (wln)), NULL),
                 DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))), NULL));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}
