/*    $Id$
 *
 * $Log$
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
 * Revision 1.3  1998/04/03 12:20:13  srs
 * *** empty log message ***
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
 - propagate constant arrays into the generator.
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
 - node[0]: NEXT  : store old information in nested WLs
 - node[1]: WL    : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN: always the last N_assign node
 - node[3]: FUNDEF: pointer to last fundef node. needed to access vardecs.
 - node[4]: LET   : pointer to N_let node of current WL.
                    LET_EXPR(ID) == INFO_WLI_WL.
 - node[5]: REPLACE: if != NULL, replace WL with this node.
 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_compound.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "generatemasks.h"
#include "WithloopFolding.h"
#include "WLT.h"

/******************************************************************************
 *
 * function:
 *   intern_gen *CutSlices(..)
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

intern_gen *
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

    FREE (lsc);
    FREE (usc);

    DBUG_RETURN (root_ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *CompleteGrid(int *ls, int *us, int *step, int *width,
 *
 * description:
 *   adds new generators which specify the elements left out by a grid.
 *
 *
 ******************************************************************************/

intern_gen *
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

    FREE (nw);

    DBUG_RETURN (root_ig);
}

/******************************************************************************
 *
 * function:
 *   int check_genarray_full_part(node *ln)
 * description:
 *   check whether the generator of this genarray-WL specifies a full partition
 *
 *
 ******************************************************************************/
int
check_genarray_full_part (node *wln)
{
    node *lowern, *uppern, *shapen;
    int result;

    shapen = NWITHOP_SHAPE (NWITH_WITHOP (wln));
    lowern = NGEN_BOUND1 (NPART_GEN (NWITH_PART (wln)));
    uppern = NGEN_BOUND2 (NPART_GEN (NWITH_PART (wln)));

    result = 1;

    /* check lower bound */
    lowern = ARRAY_AELEMS (lowern);
    while (result && lowern) {
        if (NUM_VAL (EXPRS_EXPR (lowern)) != 0)
            result = 0;
        lowern = EXPRS_NEXT (lowern);
    }

    /* check upper bound */
    uppern = ARRAY_AELEMS (uppern);
    shapen = ARRAY_AELEMS (shapen);
    while (result && uppern) {
        if (NUM_VAL (EXPRS_EXPR (shapen)) != NUM_VAL (EXPRS_EXPR (uppern)))
            result = 0;
        uppern = EXPRS_NEXT (uppern);
        shapen = EXPRS_NEXT (shapen);
    }

    return (result);
}

/******************************************************************************
 *
 * function:
 *   node * CreateFullPartition(node *wln, node *arg_info)
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

node *
CreateFullPartition (node *wln, node *arg_info)
{
    int gen_shape, do_create, *array_null, *array_shape;
    node *coden, *psi_index, *psi_array, *idn;
    ids *_ids;
    intern_gen *ig;
    char *varname;
    types *type;

    DBUG_ENTER ("CreateFullPartition");

    /*
     * only if we do not have a full partition yet.
     */
    do_create = NWITH_PARTS (wln) < 0;

    /*
     * this is the shape of the index vector (generator)
     */
    gen_shape = IDS_SHAPE (NPART_VEC (NWITH_PART (wln)), 0);

    /*
     * modarray check
     */
    if (do_create && WO_modarray == NWITH_TYPE (wln)) {
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
    if (do_create && NWITH_TYPE (wln) == WO_genarray) {
        if (!NGEN_STEP (NPART_GEN (NWITH_PART (wln))) /* not grid */
            && check_genarray_full_part (wln)) {
            do_create = 0;
            NWITH_PARTS (wln) = 1;
        } else
            do_create = (TYPES_DIM (ID_TYPE (NCODE_CEXPR (NWITH_CODE (wln)))) == 0);
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
            ArrayST2ArrayInt (NWITHOP_SHAPE (NWITH_WITHOP (wln)), &array_shape,
                              gen_shape);
        } else /* modarray */
            /* We can use the *int array of shpseg to create the upper array bound */
            array_shape
              = TYPES_SHPSEG (ID_TYPE (NWITHOP_ARRAY (NWITH_WITHOP (wln))))->shp;

        /* determine type of expr in the operator (result of body) */
        type = ID_TYPE (NCODE_CEXPR (NWITH_CODE (wln)));
        /* create code for all new parts */
        if (NWITH_TYPE (wln) == WO_genarray)
            coden = MakeNum (0);
        else { /* modarray */
            _ids = NPART_VEC (NWITH_PART (wln));
            psi_index = MakeId (StringCopy (IDS_NAME (_ids)), NULL, ST_regular);
            ID_VARDEC (psi_index) = IDS_VARDEC (_ids);
            psi_array = DupTree (NWITHOP_ARRAY (NWITH_WITHOP (wln)), NULL);
            coden = MakePrf (F_psi, MakeExprs (psi_index, MakeExprs (psi_array, NULL)));
        }
        varname = TmpVar ();
        _ids = MakeIds (varname, NULL, ST_regular); /* use memory from GetTmp() */
        IDS_VARDEC (_ids)
          = CreateVardec (varname, type, &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));
        /* varname is duplicated here (own mem) */
        idn = MakeId (StringCopy (varname), NULL, ST_regular); /* use new mem */
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
        FREE (array_null);
        if (WO_genarray == NWITH_TYPE (wln))
            FREE (array_shape); /* no mem allocated in case of modarray. */
    }

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   void CheckOptimizePsi(node **psi, node *arg_info)
 *
 * description:
 *   checks if **psi is an application with index vector and constant. If
 *   possible, the prf psi is replaced by A scalar index vector.
 *
 * example:
 *   tmp = iv[[1]];       =>        tmp = j;
 *
 ******************************************************************************/

void
CheckOptimizePsi (node **psi, node *arg_info)
{
    int index;
    node *ivn, *indexn, *datan;
    ids *_ids;

    DBUG_ENTER ("CheckOptimizePsi");

    /* first check if the array is the index vector and the index is a
       constant in range. */
    ivn = PRF_ARG2 ((*psi));
    indexn = PRF_ARG1 ((*psi));

    if (N_id == NODE_TYPE (indexn)) {
        datan = MRD_GETDATA (ID_VARNO (indexn), INFO_VARNO);
    } else
        datan = indexn;

    if (datan && N_array == NODE_TYPE (datan)
        && N_num == NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (datan)))
        && -1 == LocateIndexVar (ivn, INFO_WLI_WL (arg_info))) {
        index = NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (datan)));

        /* find index'th scalar index var */
        _ids = NPART_IDS (NWITH_PART (INFO_WLI_WL (arg_info)));
        while (index > 0 && IDS_NEXT (_ids)) {
            index--;
            _ids = IDS_NEXT (_ids);
        }

        if (!index) { /* found scalar index var. */
            INFO_USE[ID_VARNO (ivn)]--;
            if (N_id == NODE_TYPE (indexn))
                INFO_USE[ID_VARNO (indexn)]--;
            FreeTree (*psi);
            *psi = MakeId (StringCopy (IDS_NAME (_ids)), NULL, ST_regular);
            ID_VARDEC ((*psi)) = IDS_VARDEC (_ids);
            INFO_USE[IDS_VARNO (_ids)]++;
            wlt_expr++;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CheckOptimizeArray(node **array, node *arg_info)
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

void
CheckOptimizeArray (node **array, node *arg_info)
{
    int elts, i;
    ids *_ids;
    node *tmpn;
    char *vec_name;

    DBUG_ENTER ("CheckOptimizeArray");
    DBUG_ASSERT (N_array == NODE_TYPE (*array), ("no N_array node"));

    /* shape of index vector */
    _ids = NPART_VEC (NWITH_PART (INFO_WLI_WL (arg_info)));

    tmpn = ARRAY_AELEMS ((*array));
    elts = 0;
    while (tmpn) {
        if (N_id == NODE_TYPE (EXPRS_EXPR (tmpn))
            && LocateIndexVar (EXPRS_EXPR (tmpn), INFO_WLI_WL (arg_info)) == elts + 1) {
            elts++;
            tmpn = EXPRS_NEXT (tmpn);
        } else {
            tmpn = NULL;
            elts = 0;
        }
    }

    if (elts == IDS_SHAPE (_ids, 0)) { /* change to index vector */
        /* adjust USE mask */
        tmpn = ARRAY_AELEMS ((*array));
        for (i = 0; i < elts; i++) {
            INFO_USE[ID_VARNO (EXPRS_EXPR (tmpn))]--;
            tmpn = EXPRS_NEXT (tmpn);
        }

        /* free subtree and make new id node. */
        FreeTree (*array);
        vec_name = StringCopy (IDS_NAME (_ids));
        *array = MakeId (vec_name, NULL, ST_regular);
        ID_VARDEC ((*array)) = IDS_VARDEC (_ids);
        INFO_USE[IDS_VARNO (_ids)]++;
        wlt_expr++;
    }

    DBUG_VOID_RETURN;
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
 *   node *WLTwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLTwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTwith");

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Operator not implemented for with_node");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTlet(node *arg_node, node *arg_info)
 *
 * description:
 *   the follwoing occurences are seached and replaced (iv=[i,j,k])
 *   - constant indexing of index vector
 *     iv[[1]] is replaces by j
 *   - construction of iv with index scalars
 *     [i,j,k] is replaced by iv.
 *
 *   These are optimizations for the compiler phase and are not needed for WLF.
 *
 ******************************************************************************/

node *
WLTlet (node *arg_node, node *arg_info)
{
    node *exprn, *tmpn;

    DBUG_ENTER ("WLTlet");

    if (INFO_WLI_WL (arg_info)) {
        /* if we are inside a WL we have to look for
         1) ..= prf(a1,a2)  where a1 and a2 may be [i,j,k]
         2) ..= expr        where expr may be [i,j,k]
         3) ..= psi(a1,a2)  where a1 may be a const vec and a2 iv.
         */

        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn)) {
            if (F_psi == PRF_PRF (exprn)) /* 3) */
                CheckOptimizePsi (&LET_EXPR (arg_node), arg_info);
            else {
                tmpn = PRF_ARGS (exprn);
                while (tmpn) {
                    if (N_array == NODE_TYPE (EXPRS_EXPR (tmpn))) /* 1) */
                        CheckOptimizeArray (&EXPRS_EXPR (tmpn), arg_info);
                    tmpn = EXPRS_NEXT (tmpn);
                }
            }
        }

        if (N_array == NODE_TYPE (exprn)) /* 2) */
            CheckOptimizeArray (&LET_EXPR (arg_node), arg_info);
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
    tmpn->varno = INFO_VARNO;
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
        NCODE_FLAG (tmpn) = 0;
        tmpn = NCODE_NEXT (tmpn);
    }

    NWITH_FOLDABLE (arg_node) = 1;

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
        if (N_id == NODE_TYPE (arg_node))
            INFO_USE[ID_VARNO (arg_node)]++;
    } else {
        /*
         * traverse N_Nwithop
         */
        NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

        /*
         * generate full partition (genarray, modarray).
         */
        if (NWITH_FOLDABLE (arg_node)
            && (WO_genarray == NWITH_TYPE (arg_node)
                || WO_modarray == NWITH_TYPE (arg_node)))
            arg_node = CreateFullPartition (arg_node, arg_info);

        /*
         * If withop is fold, we cannot create additional N_Npart nodes (based on what?)
         */
        if (NWITH_FOLDABLE (arg_node)
            && (WO_foldfun == NWITH_TYPE (arg_node)
                || WO_foldprf == NWITH_TYPE (arg_node)))
            NWITH_PARTS (arg_node) = 1;
    }

    /*
     * restore arg_info
     */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    INFO_DEF = tmpn->mask[0];
    INFO_USE = tmpn->mask[1];
    INFO_VARNO = tmpn->varno;
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   1. traverse generator to propagate constants,
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
    if (!NCODE_FLAG (NPART_CODE (arg_node)) && !INFO_WLI_REPLACE (arg_info))
        OPTTrav (NPART_CODE (arg_node), arg_info, INFO_WLI_WL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   constant bounds, step and width vectors are substituted into the
 *   generator. If any son is not a constant vector the N_Nwith attribut
 *   FOLDABLE is set to 0.
 *   Generators that surmount the array bounds (like [-5,3] or [11,10] >
 *   [maxdim1,maxdim2] = [10,10]) are changed to fitting gens.
 *
 ******************************************************************************/

node *
WLTNgenerator (node *arg_node, node *arg_info)
{
    node *tmpn, **bound, *lbound, *ubound, *assignn, *blockn, *wln, *idn;
    int i, check_bounds, empty, warning;
    int lnum, unum, tnum, dim;
    ids *_ids, *let_ids;
    char *varname;
    types *type;

    DBUG_ENTER ("WLTNgenerator");

    /* All this work has only to be done once for every WL:
       - inserting constant bounds (done here),
       - check bounds (done here),
       - creating full partition (WLTNwith).
       If only one of these points was not successful, another call of WLT
       will try it again (NWITH_PARTS == -1).

       (OptimizePsi() and OptimizeArray() have to be called multiple
       times!) */

    wln = INFO_WLI_WL (arg_info);
    if (-1 == NWITH_PARTS (wln)) {
        /* try to propagate a constant in all 4 sons */
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
            }

            if (*bound && (NODE_TYPE ((*bound)) == N_id)) {
                tmpn = MRD_GETDATA (ID_VARNO ((*bound)), INFO_VARNO);
                if (IsConstantArray (tmpn, N_num)) {
                    /* this bound references a constant array, which may be substituted.
                     */
                    INFO_USE[ID_VARNO ((*bound))]--;
                    FreeTree (*bound);
                    /* copy const array to *bound */
                    *bound = DupTree (tmpn, NULL);

                    DBUG_ASSERT ((IsConstantArray ((*bound), N_num)),
                                 "generator contains non constant vector!!");

                } else { /* not all sons are constant */
                    if (i < 3)
                        check_bounds = 0;
                    NWITH_FOLDABLE (wln) = 0;
                }
            }
        }

        /* check bound ranges */
        if (check_bounds) {
            dim = 0;
            empty = 0;
            warning = 0;
            lbound = ARRAY_AELEMS (NGEN_BOUND1 (arg_node));
            ubound = ARRAY_AELEMS (NGEN_BOUND2 (arg_node));

            let_ids = LET_IDS (INFO_WLI_LET (arg_info));
            while (lbound) {
                lnum = NUM_VAL (EXPRS_EXPR (lbound));
                unum = NUM_VAL (EXPRS_EXPR (ubound));

                if ((NWITH_TYPE (wln) == WO_modarray)
                    || (NWITH_TYPE (wln) == WO_genarray)) {

                    tnum = IDS_SHAPE (let_ids, dim);
                    if (lnum < 0) {
                        warning = 1;
                        NUM_VAL (EXPRS_EXPR (lbound)) = 0;
                        lnum = 0;
                    }
                    if (unum > tnum) {
                        warning = 1;
                        NUM_VAL (EXPRS_EXPR (ubound)) = tnum;
                        unum = tnum;
                    }
                }

                if (unum <= lnum) {
                    /* empty set of indices */
                    empty = 1;
                    break;
                }

                dim++;
                lbound = EXPRS_NEXT (lbound);
                ubound = EXPRS_NEXT (ubound);
            }

            if (warning)
                WARN (NODE_LINE (arg_node), ("Withloop generator out of range"));

            /* the one and only N_Npart is empty. Transform WL. */
            if (empty) {
                WARN (NODE_LINE (arg_node),
                      ("Withloop generator specifies empty index set"));
                if (WO_genarray == NWITH_TYPE (INFO_WLI_WL (arg_info))) {
                    /* change generator: full scope.  */
                    dim = TYPES_DIM (IDS_TYPE (let_ids));
                    lbound = ARRAY_AELEMS (NGEN_BOUND1 (arg_node));
                    ubound = ARRAY_AELEMS (NGEN_BOUND2 (arg_node));

                    for (i = 0; i < dim; i++) {
                        NUM_VAL (EXPRS_EXPR (lbound)) = 0;
                        NUM_VAL (EXPRS_EXPR (ubound)) = IDS_SHAPE (let_ids, i);
                        lbound = EXPRS_NEXT (lbound);
                        ubound = EXPRS_NEXT (ubound);
                    }

                    if (NGEN_STEP (arg_node))
                        NGEN_STEP (arg_node) = FreeTree (NGEN_STEP (arg_node));
                    if (NGEN_WIDTH (arg_node))
                        NGEN_WIDTH (arg_node) = FreeTree (NGEN_WIDTH (arg_node));

                    /* now modify the code. Only one N_Npart/N_Ncode exists.
                       All elements have to be 0. */
                    blockn
                      = NCODE_CBLOCK (NPART_CODE (NWITH_PART (INFO_WLI_WL (arg_info))));
                    tmpn = BLOCK_INSTR (blockn);
                    if (N_empty == NODE_TYPE (tmpn)) {
                        /* there is no instruction in the block right now. */
                        BLOCK_INSTR (blockn) = FreeTree (BLOCK_INSTR (blockn));
                        /* first, introduce a new variable */
                        varname = TmpVar ();
                        _ids = MakeIds (varname, NULL,
                                        ST_regular); /* use memory from GetTmp() */
                        /* determine type of expr in the operator (result of body) */
                        type
                          = ID_TYPE (NCODE_CEXPR (NWITH_CODE (INFO_WLI_WL (arg_info))));
                        IDS_VARDEC (_ids)
                          = CreateVardec (varname, type,
                                          &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));
                        /* varname is duplicated here (own mem) */

                        /* create nullvec */
                        tmpn = MakeNullVec (TYPES_DIM (IDS_TYPE (let_ids))
                                              - ARRAY_SHAPE (NWITHOP_SHAPE (NWITH_WITHOP (
                                                               INFO_WLI_WL (arg_info))),
                                                             0),
                                            T_int);
                        /* replace N_empty with new assignment "_ids = [0,..,0]" */
                        assignn = MakeAssign (MakeLet (tmpn, _ids), NULL);
                        ASSIGN_MASK (assignn, 0) = GenMask (INFO_VARNO);
                        ASSIGN_MASK (assignn, 1) = GenMask (INFO_VARNO);
                        BLOCK_INSTR (blockn) = assignn;

                        /* replace CEXPR */
                        idn = MakeId (StringCopy (varname), NULL,
                                      ST_regular); /* use new mem */
                        ID_VARDEC (idn) = IDS_VARDEC (_ids);
                        tmpn = NPART_CODE (NWITH_PART (INFO_WLI_WL (arg_info)));
                        NCODE_CEXPR (tmpn) = FreeTree (NCODE_CEXPR (tmpn));
                        NCODE_CEXPR (tmpn) = idn;
                    } else {
                        /* we have a non-empty block.
                           search last assignment and make it the only one in the block.
                         */
                        while (ASSIGN_NEXT (tmpn))
                            tmpn = ASSIGN_NEXT (tmpn);
                        assignn = DupTree (tmpn, NULL);
                        FreeTree (BLOCK_INSTR (blockn));
                        FreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));
                        BLOCK_INSTR (blockn) = assignn;
                        LET_EXPR (ASSIGN_INSTR (assignn))
                          = MakeNullVec (TYPES_DIM (IDS_TYPE (let_ids))
                                           - ARRAY_SHAPE (NWITHOP_SHAPE (NWITH_WITHOP (
                                                            INFO_WLI_WL (arg_info))),
                                                          0),
                                         T_int);
                        ASSIGN_MASK (assignn, 0) = GenMask (INFO_VARNO);
                        ASSIGN_MASK (assignn, 1) = GenMask (INFO_VARNO);
                    }
                } else {
                    if (WO_modarray == NWITH_TYPE (INFO_WLI_WL (arg_info)))
                        /* replace WL with the base array (modarray). */
                        tmpn = NWITHOP_ARRAY (NWITH_WITHOP (INFO_WLI_WL (arg_info)));
                    else
                        /* replace WL with neutral element (fold). */
                        tmpn = NWITHOP_NEUTRAL (NWITH_WITHOP (INFO_WLI_WL (arg_info)));
                    /* the INFO_WLI_REPLACE-mechanism is used to insert the
                       new id or constant. */
                    INFO_WLI_REPLACE (arg_info) = DupTree (tmpn, NULL);
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
 *   node *WLTNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 *
 ******************************************************************************/

node *
WLTNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTNcode");

    DBUG_ASSERT (!NCODE_FLAG (arg_node), ("Body traversed a second time in WLT"));
    NCODE_FLAG (arg_node) = 1; /* this body has been traversed and all
                                  information has been gathered. */

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
