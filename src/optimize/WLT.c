/*    $Id$
 *
 * $Log$
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
 - node[0]: store old information in nested WLs
 - node[1]: reference to base node of current WL (N_Nwith)
 - node[2]: always the last N_assign node

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
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLT.h"

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

    datan = NULL;
    if (N_id == NODE_TYPE (indexn))
        MRD_GETDATA (datan, ID_VARNO (indexn), INFO_VARNO);

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
            *psi = MakeId (IDS_NAME (_ids), NULL, ST_regular);
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
        *array = MakeId (IDS_NAME (_ids), NULL, ST_regular);
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
                CheckOptimizePsi (&exprn, arg_info);
            else {
                if (N_array == NODE_TYPE (PRF_ARG1 (exprn))) /* 1) */
                    CheckOptimizeArray (&PRF_ARG1 (exprn), arg_info);
                if (N_array == NODE_TYPE (PRF_ARG2 (exprn)))
                    CheckOptimizeArray (&PRF_ARG2 (exprn), arg_info);
            }

        if (N_array == NODE_TYPE (exprn)) /* 2) */
            CheckOptimizeArray (&exprn, arg_info);
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
 *   all N_Npart nodes (inclusive bodies) are traversed.
 *
 ******************************************************************************/

node *
WLTNwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLTNwith");

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpn = MakeInfo ();
    tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
    tmpn->mask[1] = INFO_USE; /* to be identical. */
    tmpn->varno = INFO_VARNO;

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
 *   2. create full partition if possible,  ?maybe better in WLTNwith (srs)
 *   3. traverse appropriate body.
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
