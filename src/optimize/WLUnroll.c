/*
 *
 * $Log$
 * Revision 3.3  2001/03/22 21:15:15  dkr
 * include of tree.h eliminated
 *
 * Revision 3.2  2001/02/02 09:55:52  dkr
 * superfluous include of compile.h removed
 *
 * Revision 3.1  2000/11/20 18:00:38  sacbase
 * new release made
 *
 * Revision 2.21  2000/10/31 23:02:37  dkr
 * local functions are static now
 *
 * Revision 2.20  2000/10/27 02:40:24  dkr
 * some function headers corrected
 *
 * Revision 2.19  2000/10/26 23:09:06  dkr
 * fixed a bug in DoUnrollGenarray:
 * function GetBasetype() used instead of TYPE_BASETYPE in order to get
 * correct results even for user-defined types.
 *
 * Revision 2.18  2000/10/26 17:39:42  dkr
 * CreateZeroVector moved to tree_compound.[ch]
 *
 * Revision 2.17  2000/10/26 12:44:35  dkr
 * signature of DupOneIds changed
 *
 * Revision 2.16  2000/10/24 11:48:31  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 2.15  2000/10/20 15:36:09  dkr
 * macro GET_LENGTH replaced by function GetLength
 *
 * Revision 2.14  2000/10/18 09:44:07  dkr
 * fixed a bug with user-defined types in DoUnrollGenarray
 *
 * Revision 2.13  2000/06/23 15:21:10  dkr
 * signature of function DupTree changed
 *
 * Revision 2.12  2000/06/23 14:10:45  dkr
 * nodetype N_with removed
 *
 * Revision 2.11  2000/05/26 21:57:44  dkr
 * minor changes in CreateFold() done
 *
 * Revision 2.10  2000/05/26 11:34:43  dkr
 * bug fixed: fold-with-loops are now folded correctly even if multiple
 * generators/code-blocks are present :-)
 *
 * Revision 2.9  2000/05/11 11:16:06  dkr
 * Function MakeNullVec renamed into CreateZeroVector
 *
 * Revision 2.8  2000/02/23 19:08:50  cg
 * The optimizations LUR and WLUR now issue a notification if
 * the setting of -maxlur or -maxwlur, respectively, prevents
 * the unrolling of an otherwise unrollable loop or with-loop.
 * Currently the upper boundary for issuing such a note is set to 32.
 *
 * Revision 2.7  1999/11/11 20:05:48  dkr
 * signature and name of function IsConstantArray changed
 *
 * Revision 2.6  1999/07/15 20:37:30  sbs
 * DBUG_ASSERTs in CountElements inserted.
 *
 * Revision 2.5  1999/07/14 16:11:37  bs
 * Bug fixed in CountElements.
 *
 * Revision 2.4  1999/05/12 09:56:54  jhs
 * Adjusted macros to access constant vectors.
 *
 * Revision 2.3  1999/04/21 15:36:00  jhs
 * DoUnrollGenarray now fit for emty arrays.
 *
 * Revision 2.2  1999/03/19 09:47:16  bs
 * Every Call of MakeArray will also create a compact vector propagation.
 *
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
 */

/*******************************************************************************

  This file make the following functions available:
  - Check whether WL (genarray, modarray, fold) can be unrolled
  - Execution of WL unrolling (genarray, modarray, fold)

  Theses functions are called from Unroll.c

 *******************************************************************************/

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
#include "typecheck.h"

#include "optimize.h"
#include "Inline.h"
#include "DupTree.h"
#include "WithloopFolding.h"

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
    if (coden->mask[1][IDS_VARNO (NPART_VEC (partn))]) {
        letn = MakeLet (DupTree (index), DupOneIds (NPART_VEC (partn)));
        res = MakeAssign (letn, res);
    }

    /* index scalars */
    _ids = NPART_IDS (partn);
    index = ARRAY_AELEMS (index);

    while (_ids) {
        if (coden->mask[1][IDS_VARNO (_ids)]) {
            letn = MakeLet (DupTree (EXPRS_EXPR (index)), DupOneIds (_ids));
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
    tmpn
      = MakeId (StringCopy (IDS_NAME (array)), StringCopy (IDS_MOD (array)), ST_regular);
    ID_VARDEC (tmpn) = IDS_VARDEC (array);
    exprs = MakeExprs (tmpn, MakeExprs (index, MakeExprs (DupTree (cexpr), NULL)));

    letexpr = MakePrf (F_modarray, exprs);

    assignn = MakeAssign (MakeLet (letexpr, DupOneIds (array)), assignn);

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
 *   node *CreateFold( node *assignn, node *index)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
CreateFold (node *assignn, node *index)
{
    node *partn, *cexpr, *withop, *fundef, *accvar, *funap, *assigns, *bodyn;
    ids *acc;

    DBUG_ENTER ("CreateFold");

    partn = opfunarg[0];  /* N_Npart */
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

    accvar = MakeId (StringCopy (IDS_NAME (acc)), StringCopy (IDS_MOD (acc)), ST_regular);
    ID_VARDEC (accvar) = IDS_VARDEC (acc);

    funap = MakeAp (StringCopy (NWITHOP_FUN (withop)), StringCopy (NWITHOP_MOD (withop)),
                    MakeExprs (accvar, MakeExprs (DupTree (cexpr), NULL)));
    AP_FUNDEF (funap) = NWITHOP_FUNDEF (withop);

    assigns = MakeAssign (MakeLet (funap, DupOneIds (acc)), NULL);

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
    bodyn = CreateBodyCode (partn, index);

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
            type = MakeTypes (T_int, 1, shpseg, NULL, NULL);
            ARRAY_TYPE (index) = type;
            ARRAY_VECTYPE (index) = T_int;
            ARRAY_VECLEN (index) = maxdim;
            ((int *)ARRAY_CONSTVEC (index)) = Array2IntVec (ARRAY_AELEMS (index), NULL);
            assignn = opfun (assignn, index);
        } else {
            assignn = ForEachElementHelp (l, u, s, w, dim + 1, maxdim, assignn);
        }

        /* advance to next element */
        if (w && act_w + 1 < w[dim]) {
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
 *
 ******************************************************************************/

static node *
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
    if (NGEN_STEP (NPART_GEN (partn))) {
        ArrayST2ArrayInt (NGEN_STEP (NPART_GEN (partn)), &s, maxdim);
    }
    if (NGEN_WIDTH (NPART_GEN (partn))) {
        ArrayST2ArrayInt (NGEN_WIDTH (NPART_GEN (partn)), &w, maxdim);
    }

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
static int
CountElements (node *genn)
{
    int elts, tmp, d, m, dim, i;
    int *l, *u, *s, *w;
    node *tmpn;

    DBUG_ENTER ("CountElements");

    tmpn = NGEN_BOUND1 (genn);
    DBUG_ASSERT ((NODE_TYPE (tmpn) == N_id || NODE_TYPE (tmpn) == N_array),
                 ("generator bounds corrupted!"));
    if (NODE_TYPE (tmpn) == N_id) {
        DBUG_ASSERT (ID_ISCONST (tmpn),
                     "CountElements called with non-constant lower bound");
        DBUG_ASSERT (ID_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on lower bound");
        dim = ID_VECLEN (tmpn);
        l = (int *)ID_CONSTVEC (tmpn);
    } else /* NODE_TYPE(tmpn) == N_array */ {
        DBUG_ASSERT (ARRAY_ISCONST (tmpn),
                     "CountElements called with non-constant lower bound");
        DBUG_ASSERT (ARRAY_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on lower bound");
        dim = ARRAY_VECLEN (tmpn);
        l = (int *)ARRAY_CONSTVEC (tmpn);
    }

    tmpn = NGEN_BOUND2 (genn);
    DBUG_ASSERT ((NODE_TYPE (tmpn) == N_id || NODE_TYPE (tmpn) == N_array),
                 ("generator bounds corrupted!"));
    if (NODE_TYPE (tmpn) == N_id) {
        DBUG_ASSERT (ID_ISCONST (tmpn),
                     "CountElements called with non-constant upper bound");
        DBUG_ASSERT (ID_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on upper bound");
        u = (int *)ID_CONSTVEC (tmpn);
    } else { /* NODE_TYPE(tmpn) == N_array */
        DBUG_ASSERT (ARRAY_ISCONST (tmpn),
                     "CountElements called with non-constant upper bound");
        DBUG_ASSERT (ARRAY_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on upper bound");
        u = (int *)ARRAY_CONSTVEC (tmpn);
    }

    tmpn = NGEN_STEP (genn);
    if ((tmpn != NULL) && (NODE_TYPE (tmpn) == N_id)) {
        DBUG_ASSERT (ID_ISCONST (tmpn), "CountElements called with non-constant step");
        DBUG_ASSERT (ID_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on step");
        s = (int *)ID_CONSTVEC (tmpn);
    } else if ((tmpn != NULL) && (NODE_TYPE (tmpn) == N_array)) {
        DBUG_ASSERT (ARRAY_ISCONST (tmpn), "CountElements called with non-constant step");
        DBUG_ASSERT (ARRAY_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on step");
        s = (int *)ARRAY_CONSTVEC (tmpn);
    } else {
        s = NULL;
    }

    tmpn = NGEN_WIDTH (genn);
    if ((tmpn != NULL) && (NODE_TYPE (tmpn) == N_id)) {
        DBUG_ASSERT (ID_ISCONST (tmpn), "CountElements called with non-constant width");
        DBUG_ASSERT (ID_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on width");
        w = (int *)ID_CONSTVEC (tmpn);
    } else if ((tmpn != NULL) && (NODE_TYPE (tmpn) == N_array)) {
        DBUG_ASSERT (ARRAY_ISCONST (tmpn),
                     "CountElements called with non-constant width");
        DBUG_ASSERT (ARRAY_CONSTVEC (tmpn),
                     "CountElements called with corrupted CONSTVEC on width");
        w = (int *)ARRAY_CONSTVEC (tmpn);
    } else {
        w = NULL;
    }

    elts = 1;

    for (i = 0; i < dim; i++) {
        tmp = 0;

        /* check step/width */
        if ((w && !s) || (w && w[i] < 1) || (s && w && s[i] < w[i])) {
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
            if (m)
                tmp = tmp + (w ? (MIN (m, w[i])) : 1);
        }

        /* summarise elements over all dimensions. */
        elts = elts * tmp;

        /* next dimension */
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
    ok = (IsConstArray (NGEN_BOUND1 (genn)) && IsConstArray (NGEN_BOUND2 (genn))
          && (!NGEN_STEP (genn) || IsConstArray (NGEN_STEP (genn)))
          && (!NGEN_WIDTH (genn) || IsConstArray (NGEN_WIDTH (genn))));

    while (ok && partn) {
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
        if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden)))) {
            NPART_COPY (partn) = FALSE;
        } else {
            tmpn = ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (coden)));
            exprn = LET_EXPR (tmpn);
            NPART_COPY (partn)
              = (N_let == NODE_TYPE (tmpn)
                 && !strcmp (ID_NAME (NCODE_CEXPR (coden)), IDS_NAME (LET_IDS (tmpn)))
                 && N_prf == NODE_TYPE (exprn) && F_psi == PRF_PRF (exprn)
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
 *   node *DoUnrollModarray(node *wln, node *arg_info)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
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
    letn = MakeLet (DupTree (NWITH_ARRAY (wln)),
                    DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info)))));
    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int CheckUnrollGenarray(node *wln, node *arg_info)
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
    ok = (IsConstArray (NGEN_BOUND1 (genn)) && IsConstArray (NGEN_BOUND2 (genn))
          && (!NGEN_STEP (genn) || IsConstArray (NGEN_STEP (genn)))
          && (!NGEN_WIDTH (genn) || IsConstArray (NGEN_WIDTH (genn))));

    type = IDS_TYPE (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))));
    length = GetTypesLength (type);

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
 *   node *DoUnrollGenarray(node *wln, node *arg_info)
 *
 * description:
 *   Unrolls all N_Npart nodes which are marked in NPART_COPY.
 *
 ******************************************************************************/

node *
DoUnrollGenarray (node *wln, node *arg_info)
{
    node *partn, *res;
    node *letn, *let_expr;
    void *arg[2];
    ids *arrayname;
    int elements;
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

    /*
     * finally add   arrayname = reshape( ..., [0,...,0])
     */

    type = LET_TYPE (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info)));
    elements = GetTypesLength (type);
    stype = GetBasetype (type);

#if 0
  let_expr = MakePrf( F_reshape,
                      MakeExprs( DupTree( NWITH_SHAPE(wln)),
                                 MakeExprs( CreateZeroVector( elements, stype),
                                            NULL)));
#else
    /*
     * attention: the above reshape() is correct. But it seems that it
     * is not necessary anymore after the TC. reshape() is ignored in
     * compile phase.
     * If WLT is deactivated, not all elements of the WL are rewritten by
     * prf modarray. CF cannot handle prf reshape() so unrolling might fail
     * here. If we drop the reshape, further compilation should(!!!)
     * work without problems and CF can fold all elements.
     */
    let_expr = CreateZeroVector (elements, stype);
#endif
    letn = MakeLet (let_expr, DupOneIds (arrayname));
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
    ok = (IsConstArray (NGEN_BOUND1 (genn)) && IsConstArray (NGEN_BOUND2 (genn))
          && (!NGEN_STEP (genn) || IsConstArray (NGEN_STEP (genn)))
          && (!NGEN_WIDTH (genn) || IsConstArray (NGEN_WIDTH (genn))));

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
    void *arg[5];
    node *letn;

    DBUG_ENTER ("DoUnrollFold");

    partn = NWITH_PART (wln);

    res = NULL;
    while (partn != NULL) {
        /* unroll this part */
        opfun = CreateFold;
        arg[0] = partn;                                               /* N_Npart node */
        arg[1] = LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info))); /* (ids*)  */
        arg[2] = NWITH_CEXPR (wln);                                   /* (node*) */
        arg[3] = NWITH_WITHOP (wln);                                  /* N_Nwithop node */
        arg[4] = INFO_UNR_FUNDEF (arg_info);                          /* N_fundef node */
        opfunarg = arg;
        res = ForEachElement (partn, res);

        partn = NPART_NEXT (partn);
    }

    /* finally add initialisation of accumulator with neutral element. */
    letn = MakeLet (DupTree (NWITH_NEUTRAL (wln)),
                    DupOneIds (LET_IDS (ASSIGN_INSTR (INFO_UNR_ASSIGN (arg_info)))));

    res = MakeAssign (letn, res);

    DBUG_RETURN (res);
}
