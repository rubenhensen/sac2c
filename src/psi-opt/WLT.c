/*
 *
 * $Log$
 * Revision 3.13  2002/06/27 17:01:48  dkr
 * - CheckOptimize...() works also for nested WLs now
 * - CreateSel() used now
 *
 * Revision 3.12  2002/06/21 14:03:24  dkr
 * Zero-Arrays are build correctly now (by CreateZero...())
 *
 * Revision 3.11  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.10  2001/05/17 13:40:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.9  2001/04/30 12:20:29  nmw
 * integrate traversal of special fundefs in WLT traversal
 *
 * Revision 3.8  2001/04/10 13:16:53  dkr
 * WLTNgenerator() modified:
 * all bounds/step/width vectors (not only constant ones) are propagated
 * now
 *
 * Revision 3.7  2001/04/03 20:59:12  dkr
 * warning messages modified
 *
 * Revision 3.6  2001/04/02 17:10:42  dkr
 * comment in WLTNgenerator() added
 *
 * Revision 3.4  2001/03/05 16:41:44  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.2  2000/12/06 12:32:57  sbs
 * warnings eliminated.
 *
 * Revision 3.1  2000/11/20 18:01:42  sacbase
 * new release made
 *
 * Revision 2.21  2000/10/31 22:43:56  dkr
 * bug in WLTNgenerator() fixed
 * semantics of CreateZeroVector() modified
 * CreateZeroScalar() added
 *
 * Revision 2.20  2000/08/14 13:20:35  dkr
 * bug in WLTNgenerator fixed:
 * When normalizing generator bounds not only the EXPRS-representation
 * but also the CONSTVEC-representation must be modified.
 *
 * Revision 2.19  2000/08/07 14:58:44  dkr
 * some asserts for GEN_BOUND1, GEN_BOUND2 accesses added
 * compound macros used now
 *
 * Revision 2.18  2000/07/28 13:21:20  dkr
 * signature of CheckOptimizePsi() and CheckOptimizeArray changed
 *   (no reference parameter **node anymore)
 * bugs in WLTlet removed:
 *   illegal pointer access removed
 *   semantics of code and meaning of comments are consistent now :-)
 *
 * Revision 2.17  2000/06/23 15:24:22  dkr
 * signature of DupTree changed
 *
 * Revision 2.16  2000/06/23 14:17:43  dkr
 * NWITH_COMPLEX removed
 *
 * Revision 2.15  2000/05/30 12:35:08  dkr
 * functions for old with-loop removed
 *
 * Revision 2.14  2000/05/25 14:58:56  dkr
 * WLTNcode(): NCODE_CEXPR is traversed now
 *
 * Revision 2.13  2000/05/11 11:15:21  dkr
 * Function MakeNullVec renamed into CreateZeroVector.
 * Bug in function CreateFullPartition fixed:
 *   New parts created have always the correct type now :-)
 *
 * Revision 2.12  1999/11/15 18:05:41  dkr
 * VARNO replaced, INFO_VARNO with changed signature
 *
 * Revision 2.11  1999/11/11 20:05:05  dkr
 * signature and name of function IsConstantArray changed
 *
 * Revision 2.10  1999/07/14 12:16:31  bs
 * The function CheckGeneratorBounds won't be used any longer:
 * 'CheckGeneratorBounds' removed.
 * The ConstantFolding is doing this job now (and better).
 *
 * Revision 2.9  1999/05/17 11:20:41  jhs
 * CopyConstVec will be called only is ID/ARRAY_ISCONST.
 *
 * Revision 2.8  1999/05/12 11:39:24  jhs
 * Adjusted macros to new access on constant vectors.
 *
 * Revision 2.7  1999/05/07 14:54:30  jhs
 * bug fixed in CheckGeneratorBounds.
 *
 * Revision 2.6  1999/04/29 07:34:36  bs
 * New function 'CheckGeneratorBounds' added. It is used to check whether
 * the boundaries of WL-generators have got the compact vector propagation.
 *
 * Revision 2.5  1999/04/26 10:56:14  bs
 * Some code cosmetics only.
 *
 * Revision 2.4  1999/03/31 15:41:24  bs
 * braces added.
 *
 * Revision 2.3  1999/03/31 15:09:29  bs
 * I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.2  1999/02/26 14:49:06  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:37  sacbase
 * new release made
 *
 * Revision 1.15  1999/02/02 18:51:34  srs
 * Generation of full partition in special case enabled. See comment
 * 'genarray check' in function CreateFullPartition().
 * New function check_genarray_full_part().
 *
 * Revision 1.14  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.13  1998/11/18 15:07:01  srs
 * N_empty nodes are supported now
 *
 * Revision 1.12  1998/08/20 12:22:57  srs
 * added comments
 * freed pointers in CutSlices() and CompleteGrid()
 *
 * Revision 1.11  1998/05/15 14:46:40  srs
 * fixed bug in WLTlet()
 * adjusted MakeNullVec()
 * added warning for empty generator sets
 *
 * Revision 1.10  1998/04/29 12:49:06  srs
 * changed macro name
 *
 * Revision 1.9  1998/04/24 17:30:20  srs
 * invalid bounds are transformed now
 *
 * Revision 1.8  1998/04/20 08:58:56  srs
 * fixed bug in CheckOptimizePsi()
 *
 * Revision 1.7  1998/04/08 20:23:36  srs
 * adjusted parameters of Tree2InternGen,
 * intern_gen chain is now set free in CreateFullPartition.
 *
 * Revision 1.6  1998/04/08 07:46:05  srs
 * fixed bug in CreateFullPartition
 *
 * Revision 1.5  1998/04/07 15:49:16  srs
 * removed unused variable base_wl
 *
 * Revision 1.4  1998/04/07 08:18:41  srs
 * CreateFullPartition() does not use StartSearchWL() anymore.
 *
 * Revision 1.2  1998/04/01 07:44:22  srs
 * added functions to create full partition
 *
 * Revision 1.1  1998/03/22 18:21:40  srs
 * Initial revision
 *
 */

/*******************************************************************************

 At the moment we cannot transform generators of WLs to other shapes (this
 could leed to more folding actions but the right transformations steps
 are not easily calculated).

 Transformations we do:
 - propagate arrays into the generator.
 - create new generators to receive a full partition of the array.
   - genarray: Only if the generator has as much elements as the array
               described by the WL.
   - modarray: Only if the generator of the decribed array and of the
               base array have the same number of elements.
 - replace index vectors. This transformation is not needed for WLF but
   prepares a better result for compile.
   - let iv=[i,j,k], then e.g. iv[[2]] is replaced by j,
   - let iv=[i,j,k], then [i,j,k] is replaced by iv.

 *******************************************************************************

 Usage of arg_info:
 - node[0]: NEXT   : store old information in nested WLs
 - node[1]: WL     : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN : always the last N_assign node
 - node[3]: FUNDEF : pointer to last fundef node. needed to access vardecs.
 - node[4]: LET    : pointer to N_let node of current WL.
                     LET_EXPR(ID) == INFO_WLI_WL.
 - node[5]: REPLACE: if != NULL, replace WL with this node.
 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "optimize.h"
#include "generatemasks.h"
#include "WithloopFolding.h"
#include "WLT.h"

/******************************************************************************
 *
 * function:
 *   intern_gen *CutSlices( ...)
 *
 * description:
 *   Creates a (full) partition by adding new intern_gen structs.
 *   If the known part is a grid, this is ignored here (so the resulting
 *   intern_gen chain may still not be a full partition, see CompleteGrid()).
 *   The list of intern_gen is returned.
 *
 * parameters:
 *   ls, us : bounds of the whole array
 *   l, u   : bounds of the given part
 *   dim    : number of elements of ls, us, l, u
 *   ig     : chain of intern_gen struct where to add the new parts. If
 *            ig != NULL, the same pointer is returned.
 *   coden  : Pointer of N_Ncode node where the new generators shall point to.
 *
 ******************************************************************************/

static intern_gen *
CutSlices (int *ls, int *us, int *l, int *u, int dim, intern_gen *ig, node *coden)
{
    int *lsc, *usc, i, d;
    intern_gen *root_ig = NULL;

    DBUG_ENTER ("CutSlices");

    /* create local copies of the arrays which atr modified here*/
    lsc = Malloc (sizeof (int) * dim);
    usc = Malloc (sizeof (int) * dim);
    for (i = 0; i < dim; i++) {
        lsc[i] = ls[i];
        usc[i] = us[i];
    }

    root_ig = ig;

    for (d = 0; d < dim; d++) {
        /* Check whether there is a cuboid above (below) the given one. */
        if (l[d] > lsc[d]) {
            ig = AppendInternGen (ig, dim, coden, 0);
            for (i = 0; i < dim; i++) {
                ig->l[i] = lsc[i];
                ig->u[i] = usc[i];
            }
            ig->u[d] = l[d];

            if (!root_ig)
                root_ig = ig;
        }

        if (u[d] < usc[d]) {
            ig = AppendInternGen (ig, dim, coden, 0);
            for (i = 0; i < dim; i++) {
                ig->l[i] = lsc[i];
                ig->u[i] = usc[i];
            }
            ig->l[d] = u[d];

            if (!root_ig)
                root_ig = ig;
        }

        /* and modify array bounds to  continue with next dimension */
        lsc[d] = l[d];
        usc[d] = u[d];
    }

    lsc = Free (lsc);
    usc = Free (usc);

    DBUG_RETURN (root_ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *CompleteGrid( int *ls, int *us, int *step, int *width, int dim,
 *                             intern_gen *ig, node *coden)
 *
 * description:
 *   adds new generators which specify the elements left out by a grid.
 *
 ******************************************************************************/

static intern_gen *
CompleteGrid (int *ls, int *us, int *step, int *width, int dim, intern_gen *ig,
              node *coden)
{
    int i, d, *nw;
    intern_gen *root_ig;

    DBUG_ENTER ("CompleteGrid");

    nw = Malloc (sizeof (int) * dim);
    for (i = 0; i < dim; i++)
        nw[i] = step[i];

    root_ig = ig;

    for (d = 0; d < dim; d++) {
        if (step[d] > width[d]) { /* create new gris */
            ig = AppendInternGen (ig, dim, coden, 1);
            for (i = 0; i < dim; i++) {
                ig->l[i] = ls[i];
                ig->u[i] = us[i];
                ig->step[i] = step[i];
                ig->width[i] = nw[i];
            }
            ig->l[d] = ig->l[d] + width[d];
            ig->width[d] = step[d] - width[d];
            i = NormalizeInternGen (ig);
            DBUG_ASSERT (!i, ("internal normalization failure"));

            if (!root_ig)
                root_ig = ig;
        }

        nw[d] = width[d];
    }

    nw = Free (nw);

    DBUG_RETURN (root_ig);
}

/******************************************************************************
 *
 * function:
 *   int check_genarray_full_part( node *wln)
 *
 * description:
 *   check whether the generator of this genarray-WL specifies a full partition
 *
 ******************************************************************************/

static int
check_genarray_full_part (node *wln)
{
    node *lowern, *uppern, *shapen;
    int result;

    shapen = NWITH_SHAPE (wln);
    lowern = NWITH_BOUND1 (wln);
    uppern = NWITH_BOUND2 (wln);

    DBUG_ASSERT (((NODE_TYPE (lowern) == N_array) && (NODE_TYPE (uppern) == N_array)),
                 "generator bounds must be arrays!");

    result = 1;

    /* check lower bound */
    lowern = ARRAY_AELEMS (lowern);
    while (result && lowern) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (lowern)) == N_num),
                     "lower generator bound must be constant!");
        if (NUM_VAL (EXPRS_EXPR (lowern)) != 0) {
            result = 0;
        }
        lowern = EXPRS_NEXT (lowern);
    }

    /* check upper bound */
    uppern = ARRAY_AELEMS (uppern);
    shapen = ARRAY_AELEMS (shapen);
    while (result && uppern) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (uppern)) == N_num),
                     "shape must be constant!");
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (shapen)) == N_num),
                     "upper generator bound must be constant!");
        if (NUM_VAL (EXPRS_EXPR (shapen)) != NUM_VAL (EXPRS_EXPR (uppern))) {
            result = 0;
        }
        uppern = EXPRS_NEXT (uppern);
        shapen = EXPRS_NEXT (shapen);
    }

    return (result);
}

/******************************************************************************
 *
 * function:
 *   node *CreateFullPartition( node *wln, node *arg_info)
 *
 * description:
 *   generates full partition if possible:
 *    - if withop is genarray and index vector has as much elements as
 *      dimension of resulting WL.
 *    - if withop is modarray: always (needed for compilation phase).
 *   Returns wln.
 *
 * parameters:
 *   the wln is the N_Nwith node of the WL to transfrom.
 *   arg_info is needed to access the vardecs of the current function.
 *
 ******************************************************************************/

static node *
CreateFullPartition (node *wln, node *arg_info)
{
    node *coden, *idn;
    ids *_ids;
    intern_gen *ig;
    types *type;
    char *varname;
    int *array_null, *array_shape;
    int dim, gen_shape;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    /*
     * only if we do not have a full partition yet.
     */
    do_create = (NWITH_PARTS (wln) < 0);

    /*
     * this is the shape of the index vector (generator)
     */
    gen_shape = IDS_SHAPE (NWITH_VEC (wln), 0);
    /* determine type of expr in the operator (result of body) */
    type = ID_TYPE (NWITH_CEXPR (wln));

    /*
     * modarray check
     */
    if (do_create && (NWITH_TYPE (wln) == WO_modarray)) {
        /* noop */
    }

    /*
     * genarray check:
     *
     * if the CEXPR of a genarray WL is not a scalar, we have to create
     * new parts where the CEXPR is a null-vector. Other than the
     * modarray case, we have to create this null-vector first. In general,
     * this can lead to worse code, because another WL is inserted and
     * it is not guaranteed that we can fold later. So we don't create
     * a full partition here.
     *
     * But there is a special case: If the original generator is a
     * full partition itself, we do not have to create the null-vector
     * and so can create a partition with NWITH_PARTS == 1.
     */
    if (do_create && (NWITH_TYPE (wln) == WO_genarray)) {
        if (!NWITH_STEP (wln) /* no grid */
            && check_genarray_full_part (wln)) {
            do_create = FALSE;
            NWITH_PARTS (wln) = 1;
        } else {
            dim = GetShapeDim (type);
            do_create =
#if _WLS_
              (dim >= 0)
#else
              (dim == 0)
#endif
              ;
        }
    }

    /*
     * start creation
     */
    if (do_create) {
        /* create lower array bound */
        array_null = NULL;
        ArrayST2ArrayInt (NULL, &array_null, gen_shape);
        if (NWITH_TYPE (wln) == WO_genarray) {
            /* create upper array bound */
            array_shape = NULL;
            ArrayST2ArrayInt (NWITH_SHAPE (wln), &array_shape, gen_shape);
        } else { /* modarray */
            shpseg *tmp = Type2Shpseg (ID_TYPE (NWITH_ARRAY (wln)), &dim);
            int i;

            array_shape = (int *)Malloc (dim * sizeof (int));
            for (i = 0; i < dim; i++) {
                array_shape[i] = SHPSEG_SHAPE (tmp, i);
            }
            if (tmp != NULL) {
                tmp = FreeShpseg (tmp);
            }
        }

        /* create code for all new parts */
        if (NWITH_TYPE (wln) == WO_genarray) {
            /* create a zero of the correct type */
            coden = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
        } else { /* modarray */
            /*
             * we build NO with-loop here in order to ease optimizations
             *    B = modarray( A, iv, sel( iv, A));   ->   B = A;
             * and to avoid loss of performance
             *    sel( iv, A)   ->   idx_sel( ..., A)
             */
            coden = CreateSel (NWITH_VEC (wln), NWITH_IDS (wln), NWITH_ARRAY (wln), TRUE,
                               INFO_WLI_FUNDEF (arg_info));
        }
        varname = TmpVar ();
        _ids = MakeIds (varname, NULL, ST_regular);
        IDS_VARDEC (_ids)
          = CreateVardec (varname, type, &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));
        /* varname is duplicated here (own mem) */
        idn = MakeId (StringCopy (varname), NULL, ST_regular);
        ID_VARDEC (idn) = IDS_VARDEC (_ids);
        /* create new N_Ncode node  */
        coden
          = MakeNCode (MakeBlock (MakeAssign (MakeLet (coden, _ids), NULL), NULL), idn);

        /* now, copy the only part to ig */
        ig = Tree2InternGen (wln, NULL);
        DBUG_ASSERT (!ig->next, ("more than one part exist"));
        /* create surrounding cuboids */
        ig = CutSlices (array_null, array_shape, ig->l, ig->u, ig->shape, ig, coden);
        /* the original part can still be found at *ig. Now create grids. */
        if (ig->step)
            ig = CompleteGrid (ig->l, ig->u, ig->step, ig->width, ig->shape, ig, coden);

        /* if new codes have been created, add them to code list */
        if (ig->next) {
            NCODE_NEXT (coden) = NWITH_CODE (wln);
            NWITH_CODE (wln) = coden;
        }

        wln = InternGen2Tree (wln, ig);

        /* free the above made arrays */
        ig = FreeInternGenChain (ig);
        array_null = Free (array_null);
        array_shape = Free (array_shape);
    }

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   node *CheckOptimizeSel( node *sel, node *arg_info)
 *
 * description:
 *   checks if 'sel' is an application with index vector and constant. If
 *   possible, the prf sel is replaced by A scalar index vector.
 *
 * example:
 *   tmp = iv[[1]];       =>        tmp = j;
 *
 ******************************************************************************/

static node *
CheckOptimizeSel (node *sel, node *arg_info)
{
    int index;
    node *ivn, *indexn, *datan;
    ids *_ids;

    DBUG_ENTER ("CheckOptimizeSel");

    while ((arg_info != NULL) && (INFO_WLI_WL (arg_info) != NULL)) {
        /* first check if the array is the index vector and the index is a
           constant in range. */
        ivn = PRF_ARG2 (sel);
        indexn = PRF_ARG1 (sel);

        if (N_id == NODE_TYPE (indexn)) {
            datan = MRD_GETDATA (ID_VARNO (indexn), INFO_VARNO (arg_info));
        } else {
            datan = indexn;
        }

        if (datan && (N_array == NODE_TYPE (datan))
            && (N_num == NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (datan))))
            && (-1 == LocateIndexVar (ivn, INFO_WLI_WL (arg_info)))) {
            index = NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (datan)));

            /* find index'th scalar index var */
            _ids = NWITH_IDS (INFO_WLI_WL (arg_info));
            while (index > 0 && IDS_NEXT (_ids)) {
                index--;
                _ids = IDS_NEXT (_ids);
            }

            if (!index) { /* found scalar index var. */
                INFO_USE[ID_VARNO (ivn)]--;
                if (N_id == NODE_TYPE (indexn))
                    INFO_USE[ID_VARNO (indexn)]--;
                sel = FreeTree (sel);
                sel = MakeId (StringCopy (IDS_NAME (_ids)), NULL, ST_regular);
                ID_VARDEC (sel) = IDS_VARDEC (_ids);
                INFO_USE[IDS_VARNO (_ids)]++;
                wlt_expr++;
                break;
            }
        }

        arg_info = INFO_WLI_NEXT (arg_info);
    }

    DBUG_RETURN (sel);
}

/******************************************************************************
 *
 * function:
 *   node *CheckOptimizeArray( node *array, node *arg_info)
 *
 * description:
 *   Checks if array is constructed from all scalar index variables
 *   with permutation == identity. Then the array subtree is replaced
 *   by the index vector.
 *
 * example:
 *   sel = [i,j,k];       =>       sel = iv;
 *
 ******************************************************************************/

static node *
CheckOptimizeArray (node *array, node *arg_info)
{
    int elts, i;
    ids *_ids;
    node *tmpn;
    char *vec_name;

    DBUG_ENTER ("CheckOptimizeArray");

    DBUG_ASSERT ((N_array == NODE_TYPE (array)), "no N_array node");

    while ((arg_info != NULL) && (INFO_WLI_WL (arg_info) != NULL)) {
        /* shape of index vector */
        _ids = NWITH_VEC (INFO_WLI_WL (arg_info));

        tmpn = ARRAY_AELEMS (array);
        elts = 0;
        while (tmpn) {
            if ((N_id == NODE_TYPE (EXPRS_EXPR (tmpn)))
                && (LocateIndexVar (EXPRS_EXPR (tmpn), INFO_WLI_WL (arg_info))
                    == elts + 1)) {
                elts++;
                tmpn = EXPRS_NEXT (tmpn);
            } else {
                tmpn = NULL;
                elts = 0;
            }
        }

        if (elts == IDS_SHAPE (_ids, 0)) { /* change to index vector */
            /* adjust USE mask */
            tmpn = ARRAY_AELEMS (array);
            for (i = 0; i < elts; i++) {
                INFO_USE[ID_VARNO (EXPRS_EXPR (tmpn))]--;
                tmpn = EXPRS_NEXT (tmpn);
            }

            /* free subtree and make new id node. */
            array = FreeTree (array);
            vec_name = StringCopy (IDS_NAME (_ids));
            array = MakeId (vec_name, NULL, ST_regular);
            ID_VARDEC (array) = IDS_VARDEC (_ids);
            INFO_USE[IDS_VARNO (_ids)]++;
            wlt_expr++;
            break;
        }

        arg_info = INFO_WLI_NEXT (arg_info);
    }

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *   node *WLTfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   The optimization traversal OPTTrav is included in WLT traversal to
 *   modify USE and DEF and to create MRD masks.
 *
 ******************************************************************************/

node *
WLTfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTfundef");

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTassign(node *arg_node, node *arg_info)
 *
 * description:
 *   needed to apply OPTTrav
 *   store actual assign node in arg_info
 *
 ******************************************************************************/

node *
WLTassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 ******************************************************************************/

node *
WLTcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTcond");

    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 ******************************************************************************/

node *
WLTdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 ******************************************************************************/

node *
WLTwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTap(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse args
 *   traverse in applicated fundef if special one.
 *
 ******************************************************************************/

node *
WLTap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("WLTap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef */
    if ((AP_FUNDEF (arg_node) != NULL) && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_WLI_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        new_arg_info = FreeTree (new_arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTlet(node *arg_node, node *arg_info)
 *
 * description:
 *   the following occurences are searched and replaced (iv=[i,j,k]):
 *   - constant indexing of index vector:
 *     iv[[1]] is replaces by j
 *   - construction of iv with index scalars:
 *     [i,j,k] is replaced by iv.
 *
 *   These are optimizations for the compiler phase and are not needed for WLF.
 *
 ******************************************************************************/

node *
WLTlet (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLTlet");

    if (INFO_WLI_WL (arg_info)) {
        /*
         * if we are inside a WL we have to look for
         *   1:  ..= sel(a1,a2)  where a1 may be a const vec and a2 iv.
         *   2:  ..= prf(a1,a2)  where a1 and a2 may be [i,j,k]
         *   3:  ..= expr        where expr may be [i,j,k]
         */

        if ((N_prf == NODE_TYPE (LET_EXPR (arg_node)))
            && (F_sel == PRF_PRF (LET_EXPR (arg_node)))) {
            /*
             * 1:
             */
            LET_EXPR (arg_node) = CheckOptimizeSel (LET_EXPR (arg_node), arg_info);
        }

        if (N_prf == NODE_TYPE (LET_EXPR (arg_node))) {
            /*
             * 2:
             */
            tmpn = PRF_ARGS (LET_EXPR (arg_node));
            while (tmpn != NULL) {
                if (N_array == NODE_TYPE (EXPRS_EXPR (tmpn))) {
                    EXPRS_EXPR (tmpn) = CheckOptimizeArray (EXPRS_EXPR (tmpn), arg_info);
                }
                tmpn = EXPRS_NEXT (tmpn);
            }
        }

        if (N_array == NODE_TYPE (LET_EXPR (arg_node))) {
            /*
             * 3:
             */
            LET_EXPR (arg_node) = CheckOptimizeArray (LET_EXPR (arg_node), arg_info);
        }
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   start traversal of this WL and store information in new arg_info node.
 *   All N_Npart nodes (inclusive bodies) are traversed.
 *   Afterwards, if WL is foldable and certain conditions are fulfilled,
 *   the WL is transformed into a WL with generators describing a full
 *   partition.
 *
 ******************************************************************************/

node *
WLTNwith (node *arg_node, node *arg_info)
{
    node *tmpn, *old_assignn;

    DBUG_ENTER ("WLTNwith");

    /*
     * inside the body of this WL we may find another WL. So we better
     * save the old arg_info information.
     */
    tmpn = MakeInfo ();
    tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
    tmpn->mask[1] = INFO_USE; /* to be identical. */
    tmpn->varno = INFO_VARNO (arg_info);
    INFO_WLI_FUNDEF (tmpn) = INFO_WLI_FUNDEF (arg_info);
    INFO_WLI_ASSIGN (tmpn) = INFO_WLI_ASSIGN (arg_info);
    INFO_WLI_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    /*
     * initialize WL traversal
     */
    INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */
    INFO_WLI_LET (arg_info) = ASSIGN_INSTR (INFO_WLI_ASSIGN (arg_info));
    INFO_WLI_REPLACE (arg_info) = NULL;

    tmpn = NWITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        NCODE_FLAG (tmpn) = FALSE;
        tmpn = NCODE_NEXT (tmpn);
    }

    NWITH_FOLDABLE (arg_node) = TRUE;

    /*
     * traverse all parts (and implicitely bodies).
     * It is not possible that WLTNpart calls the NPART_NEXT node because
     * the superior OPTTrav mechanism has to finish before calling the
     * next part. Else modified USE and DEF masks will cause errors.
     */
    old_assignn = INFO_WLI_ASSIGN (arg_info);
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }
    INFO_WLI_ASSIGN (arg_info) = old_assignn;

    if (INFO_WLI_REPLACE (arg_info)) {
        /*
         * This WL has to be removed. Replace it by INFO_WLI_REPLACE().
         */
        FreeTree (arg_node);
        arg_node = INFO_WLI_REPLACE (arg_info);
        /*
         * arg_node can only be an N_id, an N_array or a scalar (-node).
         */
        if (N_id == NODE_TYPE (arg_node)) {
            INFO_USE[ID_VARNO (arg_node)]++;
        }
    } else {
        /*
         * traverse N_Nwithop
         */
        NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

        /*
         * generate full partition.
         */
        if (NWITH_FOLDABLE (arg_node)) {
            if (NWITH_IS_FOLD (arg_node)) {
                /*
                 * If withop is fold, we cannot create additional N_Npart nodes
                 * (based on what?)
                 */
                NWITH_PARTS (arg_node) = 1;
            } else {
                arg_node = CreateFullPartition (arg_node, arg_info);
            }
        }
    }

    /*
     * restore arg_info
     */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    INFO_DEF = tmpn->mask[0];
    INFO_USE = tmpn->mask[1];
    INFO_VARNO (arg_info) = tmpn->varno;
    tmpn = Free (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   1. traverse generator to propagate arrays,
 *   2. traverse appropriate body.
 *
 ******************************************************************************/

node *
WLTNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTNpart");

    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    /* traverse code. But do this only once, even if there are more than
       one referencing generators.
       This is just a cross reference, so just traverse, do not assign the
       resulting node.
       Only enter in case of !INFO_WLI_REPLACE() because of USE mask. */
    if ((!NCODE_FLAG (NPART_CODE (arg_node))) && (!INFO_WLI_REPLACE (arg_info))) {
        OPTTrav (NPART_CODE (arg_node), arg_info, INFO_WLI_WL (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   bounds, step and width vectors are substituted into the generator.
 *   If any son is not a constant vector the N_Nwith attribut FOLDABLE is set
 *   to FALSE.
 *   Generators that surmount the array bounds (like [-5,3] or [11,10] >
 *   [maxdim1,maxdim2] = [10,10]) are changed to fitting gens.
 *
 ******************************************************************************/

node *
WLTNgenerator (node *arg_node, node *arg_info)
{
    node *tmpn, **bound, *lb, *ub, *lbe, *ube, *assignn, *blockn, *wln, *idn;
    int i, check_bounds, empty, warning;
    int lbnum, ubnum, tnum, dim;
    ids *_ids, *let_ids;
    char *varname;
    types *type;

    DBUG_ENTER ("WLTNgenerator");

    /*
     * All this work has only to be done once for every WL:
     *  - inserting N_array bounds (done here),
     *  - check bounds (done here),
     *  - creating full partition (WLTNwith).
     * If only one of these points was not successful, another call of WLT
     * will try it again (NWITH_PARTS == -1).
     *
     * (OptimizeSel() and OptimizeArray() have to be called multiple
     * times!)
     */

    wln = INFO_WLI_WL (arg_info);
    if (-1 == NWITH_PARTS (wln)) {
        /*
         * try to propagate a N_array node in all sons
         */
        check_bounds = 1;
        for (i = 1; i <= 4; i++) {
            switch (i) {
            case 1:
                bound = &NGEN_BOUND1 (arg_node);
                break;
            case 2:
                bound = &NGEN_BOUND2 (arg_node);
                break;
            case 3:
                bound = &NGEN_STEP (arg_node);
                break;
            case 4:
                bound = &NGEN_WIDTH (arg_node);
                break;
            default: /* just to please the compiler 8-) */
                bound = NULL;
                break;
            }

            /*
             * wltransform.[ch] (compiler phase 16) can handle with-loop bounds
             * of the form '[1,2,3]', '[a,b,c]' and 'A'.
             *                        ^^^^^^^^^ :-))
             */
            if ((*bound) != NULL) {
                if (NODE_TYPE ((*bound)) == N_array) {
                    tmpn = (*bound);
                } else {
                    DBUG_ASSERT ((NODE_TYPE (*bound) == N_id),
                                 "type of generator son is neither N_id nor N_array");
                    tmpn = MRD_GETDATA (ID_VARNO ((*bound)), INFO_VARNO (arg_info));
                    if ((tmpn != NULL) && (NODE_TYPE (tmpn) == N_array)) {
                        /* this bound references an array, which may be substituted */
                        INFO_USE[ID_VARNO (((*bound)))]--;
                        (*bound) = FreeTree (*bound);
                        /* copy const array to *bound */
                        (*bound) = DupTree (tmpn);
                    }
                }
                if (!IsConstArray (tmpn)) {
                    /* non-constant son found */
                    if (i <= 2) {
                        /* non-constant lower or upper bound found */
                        check_bounds = 0;
                    }
                    NWITH_FOLDABLE (wln) = FALSE;
                }
            } else {
                DBUG_ASSERT ((i > 2), "Unspecified bound (.) in generator found!");
            }
        }

        /*
         * check bound ranges
         */
        if (check_bounds) {
            empty = 0;
            warning = 0;
            lb = NGEN_BOUND1 (arg_node);
            ub = NGEN_BOUND2 (arg_node);
            lbe = ARRAY_AELEMS (lb);
            ube = ARRAY_AELEMS (ub);

            let_ids = LET_IDS (INFO_WLI_LET (arg_info));
            dim = 0;
            while (lbe) {
                DBUG_ASSERT ((ube != NULL),
                             "dimensionality differs in lower and upper bound!");
                DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                              && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                             "generator bounds must be constant!");
                lbnum = NUM_VAL (EXPRS_EXPR (lbe));
                ubnum = NUM_VAL (EXPRS_EXPR (ube));

                if ((NWITH_TYPE (wln) == WO_modarray)
                    || (NWITH_TYPE (wln) == WO_genarray)) {
                    tnum = IDS_SHAPE (let_ids, dim);
                    if (lbnum < 0) {
                        warning = 1;
                        lbnum = NUM_VAL (EXPRS_EXPR (lbe)) = 0;
                        if (ARRAY_ISCONST (lb)) {
                            DBUG_ASSERT ((dim < ARRAY_VECLEN (lb)),
                                         "some entries are missing in ARRAY_CONSTVEC");
                            ((int *)ARRAY_CONSTVEC (lb))[dim] = 0;
                        }
                    }
                    if (ubnum > tnum) {
                        warning = 1;
                        ubnum = NUM_VAL (EXPRS_EXPR (ube)) = tnum;
                        if (ARRAY_ISCONST (ub)) {
                            DBUG_ASSERT ((dim < ARRAY_VECLEN (ub)),
                                         "some entries are missing in ARRAY_CONSTVEC");
                            ((int *)ARRAY_CONSTVEC (ub))[dim] = tnum;
                        }
                    }
                }

                if (lbnum >= ubnum) {
                    /* empty set of indices */
                    empty = 1;
                    break;
                }

                dim++;
                lbe = EXPRS_NEXT (lbe);
                ube = EXPRS_NEXT (ube);
            }

            if (warning) {
                WARN (NODE_LINE (arg_node), ("With-loop generator out of range"));
            }

            /* the one and only N_Npart is empty. Transform WL. */
            if (empty) {
                WARN (NODE_LINE (arg_node),
                      ("With-loop generator specifies empty index set"));

                if (WO_genarray == NWITH_TYPE (INFO_WLI_WL (arg_info))) {
                    /* change generator: full scope.  */
                    dim = GetDim (IDS_TYPE (let_ids));
                    lb = NGEN_BOUND1 (arg_node);
                    ub = NGEN_BOUND2 (arg_node);
                    lbe = ARRAY_AELEMS (lb);
                    ube = ARRAY_AELEMS (ub);

                    i = 0;
                    while ((i < dim) && (lbe != NULL)) {
                        NUM_VAL (EXPRS_EXPR (lbe)) = 0;
                        if (ARRAY_ISCONST (lb)) {
                            DBUG_ASSERT ((i < ARRAY_VECLEN (lb)),
                                         "some entries are missing in ARRAY_CONSTVEC");
                            ((int *)ARRAY_CONSTVEC (lb))[i] = 0;
                        }

                        NUM_VAL (EXPRS_EXPR (ube)) = IDS_SHAPE (let_ids, i);
                        if (ARRAY_ISCONST (ub)) {
                            DBUG_ASSERT ((i < ARRAY_VECLEN (ub)),
                                         "some entries are missing in ARRAY_CONSTVEC");
                            ((int *)ARRAY_CONSTVEC (ub))[i] = IDS_SHAPE (let_ids, i);
                        }

                        lbe = EXPRS_NEXT (lbe);
                        ube = EXPRS_NEXT (ube);
                        i++;
                    }

                    if (NGEN_STEP (arg_node)) {
                        NGEN_STEP (arg_node) = FreeTree (NGEN_STEP (arg_node));
                    }
                    if (NGEN_WIDTH (arg_node)) {
                        NGEN_WIDTH (arg_node) = FreeTree (NGEN_WIDTH (arg_node));
                    }

                    /* now modify the code. Only one N_Npart/N_Ncode exists.
                       All elements have to be 0. */
                    blockn = NWITH_CBLOCK (INFO_WLI_WL (arg_info));
                    tmpn = BLOCK_INSTR (blockn);
                    if (N_empty == NODE_TYPE (tmpn)) {
                        /* there is no instruction in the block right now. */
                        BLOCK_INSTR (blockn) = FreeTree (BLOCK_INSTR (blockn));
                        /* first, introduce a new variable */
                        varname = TmpVar ();
                        _ids = MakeIds (varname, NULL, ST_regular);
                        /* determine type of expr in the operator (result of body) */
                        type = ID_TYPE (NWITH_CEXPR (INFO_WLI_WL (arg_info)));
                        IDS_VARDEC (_ids)
                          = CreateVardec (varname, type,
                                          &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));

                        tmpn
                          = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
                        NCODE_FLAG (NWITH_CODE (INFO_WLI_WL (arg_info))) = TRUE;

                        /* replace N_empty with new assignment "_ids = [0,..,0]" */
                        assignn = MakeAssign (MakeLet (tmpn, _ids), NULL);
                        ASSIGN_MASK (assignn, 0) = GenMask (INFO_VARNO (arg_info));
                        ASSIGN_MASK (assignn, 1) = GenMask (INFO_VARNO (arg_info));
                        BLOCK_INSTR (blockn) = assignn;

                        /* replace CEXPR */
                        idn = MakeId (StringCopy (varname), NULL, ST_regular);
                        ID_VARDEC (idn) = IDS_VARDEC (_ids);
                        tmpn = NWITH_CODE (INFO_WLI_WL (arg_info));
                        NCODE_CEXPR (tmpn) = FreeTree (NCODE_CEXPR (tmpn));
                        NCODE_CEXPR (tmpn) = idn;
                    } else {
                        /* we have a non-empty block.
                           search last assignment and make it the only one in the block.
                         */
                        while (ASSIGN_NEXT (tmpn)) {
                            tmpn = ASSIGN_NEXT (tmpn);
                        }
                        assignn = DupTree (tmpn);
                        FreeTree (BLOCK_INSTR (blockn));
                        FreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));
                        BLOCK_INSTR (blockn) = assignn;
                        _ids = LET_IDS (ASSIGN_INSTR (assignn));
                        type = IDS_TYPE (_ids);

                        tmpn
                          = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
                        NCODE_FLAG (NWITH_CODE (INFO_WLI_WL (arg_info))) = TRUE;

                        LET_EXPR (ASSIGN_INSTR (assignn)) = tmpn;
                        ASSIGN_MASK (assignn, 0) = GenMask (INFO_VARNO (arg_info));
                        ASSIGN_MASK (assignn, 1) = GenMask (INFO_VARNO (arg_info));
                    }
                } else {
                    if (WO_modarray == NWITH_TYPE (INFO_WLI_WL (arg_info))) {
                        /* replace WL with the base array (modarray). */
                        tmpn = NWITH_ARRAY (INFO_WLI_WL (arg_info));
                    } else {
                        /* replace WL with neutral element (fold). */
                        tmpn = NWITH_NEUTRAL (INFO_WLI_WL (arg_info));
                    }
                    /* the INFO_WLI_REPLACE-mechanism is used to insert the
                       new id or constant. */
                    INFO_WLI_REPLACE (arg_info) = DupTree (tmpn);
                }
            }
        } /* check_bounds */

        arg_node = TravSons (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 ******************************************************************************/

node *
WLTNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTNcode");

    DBUG_ASSERT ((!NCODE_FLAG (arg_node)), "Body traversed a second time in WLT");

    /*
     * this body has been traversed and all information has been gathered.
     */
    NCODE_FLAG (arg_node) = TRUE;

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
