/*    $Id$
 *
 * $Log$
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
 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
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
#include "WithloopFolding.h"
#include "WLT.h"

/******************************************************************************
 *
 * function:
 *   intern_gen *CutSlices(..)
 *
 * description:
 *   Creates a (full) partition by adding new intern_gen structs.
 *   If the know part is a grid, this is ignored here (so the resulting
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

    DBUG_RETURN (root_ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *CompleteGrid(int *ls, int *us, int *step, int *width,
 *
 * description:
 *
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

    DBUG_RETURN (root_ig);
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
 *    - if withop is modarray and index vector has as much elements as
 *      generator of base WL.
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

    /* only if we do not have a full partition yet. */
    do_create = NWITH_PARTS (wln) < 0;

    /* this is the shape of the index vector (generator) */
    gen_shape = IDS_SHAPE (NPART_VEC (NWITH_PART (wln)), 0);

    /* genarray check */
    if (do_create && WO_genarray == NWITH_TYPE (wln))
        do_create = 0 == TYPES_DIM (ID_TYPE (NCODE_CEXPR (NWITH_CODE (wln))));

    /* modarray check */
    if (do_create && WO_modarray == NWITH_TYPE (wln)) {
        /* this check has been deactivated because we cannot be sure
           to have MRD-information (needed for StartSearchWL()) at this
           time. So we always create a full partition, even if we cannot
           fold this WL later. But this doen't matter because the same
           code will be derived. */
        /*     base_wl = StartSearchWL(NWITHOP_ARRAY(NWITH_WITHOP(wln)), */
        /*                             INFO_WLI_ASSIGN(arg_info), 2); */
        /*     do_create = (base_wl && */
        /*                  N_Nwith == NODE_TYPE((base_wl =  */
        /*                                        LET_EXPR(ASSIGN_INSTR(base_wl)))) && */
        /*                  gen_shape == IDS_SHAPE(NPART_VEC(NWITH_PART(base_wl)),0) && */
        /*                  NWITH_FOLDABLE(base_wl)); */
    }

    /* start creation*/
    if (do_create) {
        /* create lower array bound */
        array_null = NULL;
        ArrayST2ArrayInt (NULL, &array_null, gen_shape);
        if (WO_genarray == NWITH_TYPE (wln)) {
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
        if (WO_genarray == NWITH_TYPE (wln))
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
          = CreateVardec (varname, type,
                          &FUNDEF_VARDEC (INFO_WLI_FUNDEF (
                            arg_info))); /* varname is duplicated here (own mem) */
        idn = MakeId (StringCopy (varname), NULL, ST_regular); /* use new mem */
        ID_VARDEC (idn) = IDS_VARDEC (_ids);
        /* create new N_Ncode node  */
        coden
          = MakeNCode (MakeBlock (MakeAssign (MakeLet (coden, _ids), NULL), NULL), idn);

        /* now, copy the only part to*/
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
        MRD_GETDATA (datan, ID_VARNO (indexn), INFO_VARNO);
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
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);

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
    node *exprn;

    DBUG_ENTER ("WLTlet");

    if (INFO_WLI_WL (arg_info)) {
        /* if we are inside a WL we have to look for
         1) ..= prf(a1,a2)  where a1 and a2 may be [i,j,k]
         2) ..= expr        where expr may be [i,j,k]
         3) ..= psi(a1,a2)  where a1 may be a const vec and a2 iv.
         */

        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn))
            if (F_psi == PRF_PRF (exprn)) /* 3) */
                CheckOptimizePsi (&LET_EXPR (arg_node), arg_info);
            else {
                if (N_array == NODE_TYPE (PRF_ARG1 (exprn))) /* 1) */
                    CheckOptimizeArray (&PRF_ARG1 (exprn), arg_info);
                if (N_array == NODE_TYPE (PRF_ARG2 (exprn)))
                    CheckOptimizeArray (&PRF_ARG2 (exprn), arg_info);
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

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpn = MakeInfo ();
    tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
    tmpn->mask[1] = INFO_USE; /* to be identical. */
    tmpn->varno = INFO_VARNO;
    INFO_WLI_FUNDEF (tmpn) = INFO_WLI_FUNDEF (arg_info);
    INFO_WLI_ASSIGN (tmpn) = INFO_WLI_ASSIGN (arg_info);
    INFO_WLI_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    /* initialize WL traversal */
    INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        NCODE_FLAG (tmpn) = 0;
        tmpn = NCODE_NEXT (tmpn);
    }
    NWITH_FOLDABLE (arg_node) = 1;

    old_assignn = INFO_WLI_ASSIGN (arg_info);

    /* traverse N_Nwithop */
    NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

    /* traverse all parts (and implicitely bodies).
       It is not possible that WLTNpart calls the NPART_NEXT node because
       the superior OPTTrav mechanism has to finish before calling the
       next part. Else modified USE and DEF masks will case errors. */
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }

    INFO_WLI_ASSIGN (arg_info) = old_assignn;

    /* generate full partition (genarray, modarray). */
    if (NWITH_FOLDABLE (arg_node)
        && (WO_genarray == NWITH_TYPE (arg_node) || WO_modarray == NWITH_TYPE (arg_node)))
        arg_node = CreateFullPartition (arg_node, arg_info);

    /* If withop if fold, we cannot create additional N_Npart nodes (based on what?) */
    if (WO_foldfun == NWITH_TYPE (arg_node) || WO_foldprf == NWITH_TYPE (arg_node))
        NWITH_PARTS (arg_node) = 1;

    /* restore arg_info */
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
       resulting node.*/
    if (!NCODE_FLAG (NPART_CODE (arg_node)))
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
 *
 ******************************************************************************/

node *
WLTNgenerator (node *arg_node, node *arg_info)
{
    node *tmpn, **bound;
    int i;

    DBUG_ENTER ("WLTNgenerator");

    /* try to propagate a constant in all 4 sons */
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

        if (*bound && N_id == NODE_TYPE ((*bound))) {
            MRD_GETDATA (tmpn, ID_VARNO ((*bound)), INFO_VARNO);
            if (IsConstantArray (tmpn, N_num)) {
                /* this bound references a constant array, which can be substituted. */
                INFO_USE[ID_VARNO ((*bound))]--;
                FreeTree (*bound);
                /* copy const array to *bound */
                *bound = DupTree (tmpn, NULL);
            } else /* not all sons are constant */
                NWITH_FOLDABLE (INFO_WLI_WL (arg_info)) = 0;
        }
    }

    arg_node = TravSons (arg_node, arg_info);
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
