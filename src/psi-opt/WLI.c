/*    $Id$
 *
 * $Log$
 * Revision 2.3  1999/02/28 21:44:12  srs
 * renamed FREE_INDEX to FREE_INDEX_INFO
 *
 * Revision 2.2  1999/02/26 14:45:53  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:34  sacbase
 * new release made
 *
 * Revision 1.11  1999/01/20 09:09:19  srs
 * NWITH_REFERENCES_FOLDED is initializes in WLINwith.
 *
 * Revision 1.10  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.9  1998/12/10 17:28:13  sbs
 * ScalarPrf => FoldPrfScalars
 *
 * Revision 1.8  1998/05/16 16:25:57  srs
 * added debug information
 *
 * Revision 1.7  1998/05/15 15:46:45  srs
 * enhanced transformation power of CreateIndexInfoA()
 *
 * Revision 1.6  1998/05/12 09:47:08  srs
 * removed unused lib DupTree.h
 *
 * Revision 1.5  1998/04/20 08:59:22  srs
 * added comments
 *
 * Revision 1.4  1998/04/07 08:19:24  srs
 * inserted wli_phase
 *
 * Revision 1.3  1998/04/03 12:20:07  srs
 * *** empty log message ***
 *
 * Revision 1.2  1998/04/01 07:43:15  srs
 * *** empty log message ***
 *
 * Revision 1.1  1998/03/22 18:21:43  srs
 * Initial revision
 *
 */

/*******************************************************************************

 This file realizes the information gathering for the new SAC-WLs (WLI phase).
 The decision to fold or not to fold (WLF) is based on these informations.

 *******************************************************************************

 Usage of arg_info:
 - node[0]: NEXT  : store old information in nested WLs
 - node[1]: WL    : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN: always the last N_assign node (see WLIassign)
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
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "generatemasks.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLI.h"

int wli_phase;

/******************************************************************************
 *
 * function:
 *   prf SimplifyFun(prf prf)
 *
 * description:
 *   Transforms special prf names (e.g. F_add_SxA) to the base name (F_add).
 *   Does this for add, sub, mul, div.
 *
 ******************************************************************************/

prf
SimplifyFun (prf prf)
{
    DBUG_ENTER ("SimplifyFun");

    switch (prf) {
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        prf = F_add;
        break;

    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
        prf = F_sub;
        break;

    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        prf = F_mul;
        break;

    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        prf = F_div;
        break;

    default:
        break;
    }

    DBUG_RETURN (prf);
}

/******************************************************************************
 *
 * function:
 *   node *CheckArrayFoldable(node *indexn, node *idn, node *arg_info)
 *
 * description:
 *   This function checks whether the application of psi(indexn,idn) inside
 *   a WL can be folded or not. If yes, the WL described by idn is returned,
 *   else NULL.
 *
 * remark:
 *   This function has only to be called if indexn is a known valid index
 *   transformation. This is supposed here and not checked again.
 *
 ******************************************************************************/

node *
CheckArrayFoldable (node *indexn, node *idn, node *arg_info)
{
    node *substn = NULL, *thisn;

    DBUG_ENTER ("CheckArrayFoldable");
    DBUG_ASSERT (N_id == NODE_TYPE (indexn), ("Wrong nodetype for indexn"));
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("Wrong nodetype for idn"));

    thisn = INFO_WLI_WL (arg_info);
    if (NWITH_FOLDABLE (thisn)) {
        substn = ID_WL (idn);
        if (!substn) {
            substn = StartSearchWL (idn, INFO_WLI_ASSIGN (arg_info), 1);
            ID_WL (idn) = substn;
        }
        if (substn) {
            /* the idn references a WL which can be used for folding. */
            substn = LET_EXPR (ASSIGN_INSTR (substn));
            /* We have to assure that the access index and the generator
               index of the substitution WL have the same shape. */
            if (NWITH_FOLDABLE (substn)
                && IDS_SHAPE (NPART_VEC (NWITH_PART (substn)), 0) == ID_SHAPE (indexn, 0))
                NWITH_REFERENCED_FOLD (substn)++;
            else
                substn = NULL;
        }
    }

    DBUG_RETURN (substn);
}

/******************************************************************************
 *
 * function:
 *   index_info *Scalar2ArrayIndex(node *letn, node *wln);
 *
 * description:
 *   arrayn has to be N_array. This function checks if the array is
 *   a valid construction of an index vector within the body. The
 *   elements have to be constants, scalar index vars or transformations
 *   of scalar index vars.
 *
 *   Every scalar base index may only appear once (i.e. no [i,2,i] is allowed).
 *   This can be deactivated by TRANSF_TRUE_PERMUTATIONS.
 *
 * return:
 *   returns NULL if vector is not valid or else index_info with arg_no == 0.
 *
 ******************************************************************************/

index_info *
Scalar2ArrayIndex (node *arrayn, node *wln)
{
    index_info *iinfo, *tmpii;
    int elts = 1, ok = 1, i, *valid_permutation;
    node *idn;

    DBUG_ENTER ("Scalar2ArrayIndex");
    DBUG_ASSERT (N_array == NODE_TYPE (arrayn), ("wrong nodetype (array)"));

    elts = TYPES_SHAPE (ARRAY_TYPE (arrayn), 0);
    arrayn = ARRAY_AELEMS (arrayn);

    iinfo = CreateIndex (elts);
    valid_permutation = Malloc (sizeof (int) * elts);
    for (i = 0; i < elts;)
        valid_permutation[i++] = 0;

    for (i = 0; i < elts && ok; i++) {
        /* check each element. */
        ok = 0;
        iinfo->last[i] = NULL;
        idn = EXPRS_EXPR (arrayn);
        if (N_num == NODE_TYPE (idn)) { /* this is a constant */
            iinfo->permutation[i] = 0;
            iinfo->const_arg[i] = NUM_VAL (idn);
            ok = 1;
        }

        if (N_id == NODE_TYPE (idn)) {
            tmpii = ValidLocalId (idn);
            if (tmpii &&          /* this is a local id, not index var */
                !tmpii->vector) { /* and scalar, not vector */
                iinfo->permutation[i] = tmpii->permutation[0];
                iinfo->last[i] = tmpii;
                /* may only be incremented once */
#ifdef TRANSF_TRUE_PERMUTATIONS
                ok = 1 == ++valid_permutation[iinfo->permutation[i] - 1];
#else
                ok = 1;
#endif
            } else if (0 < (iinfo->permutation[i] = LocateIndexVar (idn, wln))) {
                /* index scalar */
#ifdef TRANSF_TRUE_PERMUTATIONS
                ok = 1 == ++valid_permutation[iinfo->permutation[i] - 1];
#else
                ok = 1;
#endif
            }
        }

        arrayn = EXPRS_NEXT (arrayn);
    }

    if (!ok)
        FREE (iinfo);
    FREE (valid_permutation);

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   int CreateIndexInfoId(node *idn, node *arg_info)
 *
 * description:
 *   creates an index_info from an Id. This could be an index vector, an
 *   index scalar or just a local valid variable.
 *   If the Id is vaild, the index_info is created/duplicated and assigned
 *   to the assign node given by arg_info. Returns 1 on success.
 *
 ******************************************************************************/

int
CreateIndexInfoId (node *idn, node *arg_info)
{
    index_info *iinfo;
    node *assignn, *wln;
    int index_var, i, elts;

    DBUG_ENTER ("CreateIndexInfoId");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    DBUG_ASSERT (!INDEX (assignn), ("index_info already assigned"));

    /* index var? */
    index_var = LocateIndexVar (idn, wln);
    if (index_var) {
        iinfo = CreateIndex (index_var > 0 ? 0 : ID_SHAPE (idn, 0));
        INDEX (assignn) = iinfo; /* make this N_assign valid */

        if (-1 == index_var) { /* index vector */
            elts = ID_SHAPE (idn, 0);
            for (i = 0; i < elts; i++) {
                iinfo->last[i] = NULL;
                iinfo->permutation[i] = i + 1;
            }
        } else { /* index scalar */
            iinfo->permutation[0] = index_var;
            iinfo->last[0] = NULL;
        }
    } else {
        /* valid local variable */
        iinfo = ValidLocalId (idn);
        if (iinfo)
            INDEX (assignn) = DuplicateIndexInfo (iinfo);
    }

    DBUG_RETURN (NULL != INDEX (assignn));
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoSxS(node *prfn, node *arg_info)
 *
 * description:
 *   checks if application of the prfn is a valid transformation in
 *   this WL wln and, if valid, creates an INDEX_INFO at the assignn.
 *
 ******************************************************************************/

void
CreateIndexInfoSxS (node *prfn, node *arg_info)
{
    int id_no = 0, index_var = 0;
    index_info *iinfo;
    node *idn, *assignn, *wln;

    DBUG_ENTER (" CreateIndexInfoSxS");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    /* CF has been done, so we just search for an Id and a constant.
       Since we do not want to practice constant folding here we ignore
       prfs with two constants. */
    if (N_id == NODE_TYPE (PRF_ARG1 (prfn)) &&  /* first arg is an Id */
        N_num == NODE_TYPE (PRF_ARG2 (prfn))) { /* second arg is a numeric constant */
        id_no = 1;
        idn = PRF_ARG1 (prfn);
    } else if (N_id == NODE_TYPE (PRF_ARG2 (prfn))
               && N_num == NODE_TYPE (PRF_ARG1 (prfn))) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
    }

    if (id_no) {
        /* we found a constant and an Id. If this Id is a vaild Id (i.e.
           it is declared in the generator or it is a valid local Id)
           this transformation is valid, too. */
        iinfo = ValidLocalId (idn);
        if (!iinfo) /* maybe it's an index var. */
            index_var = LocateIndexVar (idn, wln);

        if (iinfo || index_var) {
            iinfo = CreateIndex (0); /* create a scalar index_info */
            INDEX (assignn) = iinfo; /* make this N_assign valid */

            /* if the Id is an index var... */
            if (index_var) {
                iinfo->last[0] = NULL;
                iinfo->permutation[0] = index_var; /* set permutation, always != -1 */
            } else {
                iinfo->last[0] = ValidLocalId (idn);
                /* else copy permutation from last index_info */
                iinfo->permutation[0] = iinfo->last[0]->permutation[0];
            }

            iinfo->prf = SimplifyFun (PRF_PRF (prfn));
            iinfo->const_arg[0]
              = NUM_VAL ((1 == id_no ? PRF_ARG2 (prfn) : PRF_ARG1 (prfn)));
            iinfo->arg_no = 1 == id_no ? 2 : 1;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoA(node *prfn, node *arg_info)
 *
 * description:
 *   checks if application of the prfn is a valid transformation in
 *   this WL wln and, if valid, creates an INDEX_INFO at the assignn.
 *   In contrast to CreateIndexInfoSxS the result of the prfn always is a
 *   vector.
 *
 *   We have to detect the following cases:
 *   (iv: id of an index vector, i : index scalar, c : constant.)
 *
 *   AxS (SxA): iv prfop c
 *              [i,i,c] prfop c
 *   AxA      : iv prfop [c,c,c]
 *              [i,i,c] prfop [c,c,c]
 *
 *   not valid: i prfop [c,c,c]
 *
 * remark: arrays are not duplicated by CF, so if we sind an Id we have to
 *   use MRD to find out if it's a constant array [c,c,c].
 *
 ******************************************************************************/

void
CreateIndexInfoA (node *prfn, node *arg_info)
{
    int id_no = 0, elts, i, index, val;
    node *idn, *constn, *tmpn, *cf_node, *args[2], *assignn, *wln;
    index_info *iinfo, *tmpinfo;
    types *type;
    node *data1, *data2;

    DBUG_ENTER (" CreateIndexInfoA");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    data1 = data2 = NULL;
    if (N_id == NODE_TYPE (PRF_ARG1 (prfn)))
        MRD_GETDATA (data1, ID_VARNO (PRF_ARG1 (prfn)), INFO_VARNO);
    if (N_id == NODE_TYPE (PRF_ARG2 (prfn)))
        MRD_GETDATA (data2, ID_VARNO (PRF_ARG2 (prfn)), INFO_VARNO);

    /* Which argument is the constant (so which will be the Id)? */
    if (N_num == NODE_TYPE (PRF_ARG1 (prfn))
        || IsConstantArray (PRF_ARG1 (prfn), N_num)) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
        constn = PRF_ARG1 (prfn);
    } else if (data1 && IsConstantArray (data1, N_num)) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
        constn = data1;
    }

    if (N_num == NODE_TYPE (PRF_ARG2 (prfn))
        || IsConstantArray (PRF_ARG2 (prfn), N_num)) {
        id_no = 1;
        idn = PRF_ARG1 (prfn);
        constn = PRF_ARG2 (prfn);
    } else if (data2 && IsConstantArray (data2, N_num)) {
        id_no = 1;
        idn = PRF_ARG1 (prfn);
        constn = data2;
    }

    /* Is idn an Id of an index vector (or transformation)? */
    if (id_no && N_id == NODE_TYPE (idn)) {
        tmpinfo = ValidLocalId (idn);
        index = LocateIndexVar (idn, wln);

        /* The Id is the index vector itself or, else it has to
           be an Id which is a valid vector. It must not be based
           on an index scalar (we do want "i prfop [c,c,c]"). */
        if (-1 == index ||                                 /* index vector itself */
            (tmpinfo && 1 == TYPES_DIM (ID_TYPE (idn)))) { /* valid local Id (vector) */
            elts = ID_SHAPE (idn, 0);
            iinfo = CreateIndex (elts);
            INDEX (assignn) = iinfo; /* make this N_assign valid */

            iinfo->arg_no = 1 == id_no ? 2 : 1;
            iinfo->prf = SimplifyFun (PRF_PRF (prfn));

            if (N_num == NODE_TYPE (constn))
                val = NUM_VAL (constn);
            else
                tmpn = ARRAY_AELEMS (constn);
            for (i = 0; i < elts; i++) {
                if (N_num != NODE_TYPE (constn)) {
                    DBUG_ASSERT (tmpn, ("Too few elements in array"));
                    val = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                }

                if (-1 == index) { /* index vector */
                    iinfo->last[i] = NULL;
                    iinfo->permutation[i] = i + 1;
                } else { /* local var, not index vector */
                    if ((iinfo->permutation[i] = tmpinfo->permutation[i]))
                        iinfo->last[i] = tmpinfo;
                    else
                        iinfo->last[i] = NULL; /* elt is constant */
                }

                if (iinfo->permutation[i])
                    iinfo->const_arg[i] = val;
                else {
                    /* constant. Use FoldPrfScalars() to constantfold. */
                    type = MakeType (T_int, 0, NULL, NULL, NULL);
                    args[0] = MakeNum (val);
                    args[1] = MakeNum (tmpinfo->const_arg[i]);
                    cf_node
                      = FoldPrfScalars (PRF_PRF (prfn), args, type, 2 == iinfo->arg_no);
                    iinfo->const_arg[i] = NUM_VAL (cf_node);
                    FREE (type);
                    FREE (args[0]); /* *cf_node == *args[0] */
                    FREE (args[1]);
                }
            }
        } /* this Id is valid. */
    }     /* this is an Id. */

    /* Is it a contruction based on index scalars ([i,i,c])? */
    if (id_no && N_array == NODE_TYPE (idn)) {
        iinfo = Scalar2ArrayIndex (idn, wln);
        if (iinfo) {
            /* This is a valid vector. Permutation and last of index_info are
               already set. But we still need to handle the prf. */
            elts = iinfo->vector;
            INDEX (assignn) = iinfo;

            iinfo->arg_no = 1 == id_no ? 2 : 1;
            iinfo->prf = SimplifyFun (PRF_PRF (prfn));

            if (N_num == NODE_TYPE (constn))
                val = NUM_VAL (constn);
            else
                tmpn = ARRAY_AELEMS (constn);
            for (i = 0; i < elts; i++) {
                if (N_num != NODE_TYPE (constn)) {
                    DBUG_ASSERT (tmpn, ("Too few elements in array"));
                    val = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                }

                /* is the element in the other vector a constant, too?
                   Then we have to fold immedeately. */
                if (!iinfo->permutation[i]) {
                    type = MakeType (T_int, 0, NULL, NULL, NULL);
                    args[0] = MakeNum (val);
                    args[1] = MakeNum (iinfo->const_arg[i]);
                    cf_node
                      = FoldPrfScalars (PRF_PRF (prfn), args, type, 2 == iinfo->arg_no);
                    iinfo->const_arg[i] = NUM_VAL (cf_node);
                    FREE (type);
                    FREE (args[0]); /* *cf_node == *args[0] */
                    FREE (args[1]);
                } else
                    iinfo->const_arg[i] = val;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *WLIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   The optimization traversal OPTTrav is included in WLI traversal to
 *   modify USE and DEF and to create MRD masks.
 *
 ******************************************************************************/

node *
WLIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIfundef");

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIassign(node *arg_node, node *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node. Apply OPTTrav.
 *
 *
 ******************************************************************************/

node *
WLIassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;

    if (INDEX (arg_node))
        /* this is important. Only index transformations
           with a non-null INDEX are valid. See WLIlet. Before WLI, this
           pointer may be non null (somwhere wrong initialisation -> better
           use MakeAssign()!!! ) */
        FREE_INDEX_INFO (INDEX (arg_node));

    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIcond");

    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);

    /* traverse bodies. MRDs are build locally in the bodies. The DEF mask is
       evaluated in the superior OPTTrav to modify the actual MRD list. */
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwith");

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
 *   node *WLIid(node *arg_node, node *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_Nwith) we want to increment
 *   the number of references to it (NWITH_REFERENCED). Therefore the
 *   WL node has to be found.
 *   But this is not always possible (e.g. WLs may be defined in
 *   a conditional, for more information see SearchWL). Then the flag
 *   NWITH_NO_CHANCE is marked which means that the WL cannot be used
 *   for folding anyway. Again see SearchWL for more information.
 *
 ******************************************************************************/

node *
WLIid (node *arg_node, node *arg_info)
{
    node *assignn;

    DBUG_ENTER ("WLIid");

    if (1 == wli_phase)
        ID_WL (arg_node) = NULL; /* could be done in phase 2, too. */
    else {
        /* if arg_node describes a WL the NWITH_REFERENCED has to be
           incremented. But this must not be done if we have an assignment
           of the form A = B. */
        assignn = INFO_WLI_ASSIGN (arg_info);
        if (N_let != ASSIGN_INSTRTYPE (assignn)
            || N_id != NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assignn)))) {
            ID_WL (arg_node) = StartSearchWL (arg_node, assignn, 0);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIlet(node *arg_node, node *arg_info)
 *
 * description:
 *   We are interested in prf applications. This may be transformations of
 *   later used index vectors for arrays to fold.
 *   If we find an F_psi, we check, based on the above made transformation
 *   checks, if the index vector is vald (i.e. the array reference is
 *   foldable).
 *
 ******************************************************************************/

node *
WLIlet (node *arg_node, node *arg_info)
{
    node *exprn, *tmpn, *old_assignn;
    prf prf;

    DBUG_ENTER ("WLIlet");

    /* traverse sons first so that ID_WL of every Id is defined (or NULL). */
    old_assignn = INFO_WLI_ASSIGN (arg_info);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    INFO_WLI_ASSIGN (arg_info) = old_assignn;

    /* if we are inside a WL we have to search for valid index transformations. */
    if (INFO_WLI_WL (arg_info) && (2 == wli_phase)) {
        /* if this is a prf, we are interrested in transformations like +,*,-,/
           and in indexing (F_psi). */
        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn)) {
            prf = PRF_PRF (exprn);
            switch (prf) {
            /* this may ba an assignment which calculates an index for an
               array to fold. */
            case F_add:
            case F_sub:
            case F_mul:
            case F_div: /* both args are scalars */
                CreateIndexInfoSxS (exprn, arg_info);
                break;

            case F_add_SxA:
            case F_sub_SxA:
            case F_mul_SxA:
            case F_div_SxA:

            case F_add_AxS:
            case F_sub_AxS:
            case F_mul_AxS:
            case F_div_AxS:

            case F_add_AxA:
            case F_sub_AxA:
            case F_mul_AxA:
            case F_div_AxA:
                CreateIndexInfoA (exprn, arg_info);
                break;

            case F_psi:
                /* check if index (1st argument) is valid. If yes, the array
                   could be folded.
                   We have 3 possibilities here:
                   - the array's Id (2nd argument) is not based on a foldable
                   withloop. So no index_info is created. Else...
                   - the first argument is the index vector from the generator.
                   Then no index_info for this transformation exists right now.
                   Create it.
                   - the first argument is a reference to a locally defined
                   valid (or invalid) transformation. If valid, duplicate it. */
                if (N_id == NODE_TYPE (PRF_ARG1 (exprn))
                    && CreateIndexInfoId (PRF_ARG1 (exprn), arg_info)) {
                    /* Now, if this was a valid transformation and we index an
                       array which is based on a WL, we have to determine if
                       we finally CAN fold the array.
                       If we can, the index_info is kept, else removed. So in the
                       WLF phase we know for sure that a psi-prf with a
                       valid index_info can be folded. But that still doesn't
                       mean that we want to fold. */
                    tmpn
                      = CheckArrayFoldable (PRF_ARG1 (exprn), PRF_ARG2 (exprn), arg_info);
                    if (!tmpn)
                        FREE_INDEX_INFO (INDEX (INFO_WLI_ASSIGN (arg_info)));
                }
                break;

            default:
                break;
            }
        }

        if (N_id == NODE_TYPE (exprn)) {
            /* The let expr may be an index scalar or the index vector. */
            CreateIndexInfoId (exprn, arg_info);
            /* Or it is an array Id without indexing. But we cannot fold such
               unindexed arrays because its access index has to be transformed
               to the shape of its generator index first. */
        }

        /* The let expr still may be a construction of a vector (without a prf). */
        if (N_array == NODE_TYPE (exprn)) {
            INDEX (INFO_WLI_ASSIGN (arg_info))
              = Scalar2ArrayIndex (exprn, INFO_WLI_WL (arg_info));
        }

    } /* is this a WL? */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINwith(node *arg_node, node *arg_info)
 *
 * description:
 *   start gathering information for this WL.
 *   First some initialisations in the WL structure are done, then the
 *   N_Nparts are traversed (which call the appropriate N_Ncode subtrees).
 *
 ******************************************************************************/

node *
WLINwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLINwith");

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
    /* Attribut FOLDABLE has been set in WLT. The others are initialized here. */
    NWITH_REFERENCED (arg_node) = 0;
    NWITH_REFERENCED_FOLD (arg_node) = 0;
    NWITH_REFERENCES_FOLDED (arg_node) = 0;
    NWITH_COMPLEX (arg_node) = 0;
    NWITH_NO_CHANCE (arg_node) = 0;

    /* traverse N_Nwithop */
    NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

    /* traverse all parts (and implicitely bodies).
       It is not possible that WLINpart calls the NPART_NEXT node because
       the superior OPTTrav mechanism has to finish before calling the
       next part. Else modified USE and DEF masks will case errors. */
    DBUG_PRINT ("WLI", ("searching code of  WL in line %d", NODE_LINE (arg_node)));
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        DBUG_PRINT ("WLI", ("code : %p", tmpn));
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }
    DBUG_PRINT ("WLI", ("searching done"));

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
 *   node *WLINwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   if op is modarray we have to check whether the base array Id can be
 *   eleminated by folding. Then NWITH_REFERENCED_FOLD is incremented.
 *
 ******************************************************************************/

node *
WLINwithop (node *arg_node, node *arg_info)
{
    node *substn;

    DBUG_ENTER ("WLINwithop");

    arg_node = TravSons (arg_node, arg_info);

    if (2 == wli_phase && WO_modarray == NWITHOP_TYPE (arg_node)
        && NWITH_FOLDABLE (INFO_WLI_WL (arg_info))) {
        substn = ID_WL (NWITHOP_ARRAY (arg_node));
        /* we just traversed through the sons so SearchWL for NWITHOP_ARRAY
         has been called and its result is stored in ID_WL(). */
        if (substn)
            NWITH_REFERENCED_FOLD (LET_EXPR (ASSIGN_INSTR (substn)))++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINpart(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse appropriate body.
 *
 ******************************************************************************/

node *
WLINpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLINpart");

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
 *   node *WLINcode(node *arg_node, node *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 *
 ******************************************************************************/

node *
WLINcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLINcode");

    DBUG_ASSERT (!NCODE_FLAG (arg_node), ("Body traversed a second time in WLI"));
    NCODE_FLAG (arg_node) = 1; /* this body has been traversed and all
                                  information has been gathered. */

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
