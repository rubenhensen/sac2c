/*
 *
 * $Log$
 * Revision 1.17  2004/08/06 15:16:59  khf
 * corrected setting of NWITH_FOLDABLE
 *
 * Revision 1.16  2004/08/05 11:15:12  khf
 * earlier setting of NWITH_FOLDABLE
 *
 * Revision 1.15  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.14  2004/03/26 13:02:12  khf
 * Attribute NWITH_FOLDABLE is set in SSAWLI now,
 * therefor SSAWLINgenerator added
 *
 * Revision 1.13  2002/10/09 02:11:26  dkr
 * constants modul used instead of ID/ARRAY_CONSTVEC
 *
 * Revision 1.12  2002/10/07 04:51:05  dkr
 * some modifications for dynamic shapes added
 *
 * Revision 1.9  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 1.8  2001/05/23 15:55:18  nmw
 * buggy call to SSACFFoldPrfExpr() fixed
 *
 * Revision 1.7  2001/05/18 12:42:14  nmw
 * some discriptive comments added
 *
 * Revision 1.6  2001/05/17 14:09:32  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 1.5  2001/05/17 13:29:29  cg
 * De-allocation macros FREE_INTERN_GEN and FREE_INDEX_INFO
 * converted to functions.
 *
 * Revision 1.4  2001/05/16 19:52:47  nmw
 * reverted Free() to FREE() due to segfaults when used with linux :-(
 *
 * Revision 1.3  2001/05/16 13:42:25  nmw
 * unused old code removed, comments corrected
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/05/16 09:24:40  nmw
 * bug fixed that calls wrong traversal functions
 *
 * Revision 1.1  2001/05/15 15:41:11  nmw
 * Initial revision
 *
 *
 * created from WLI.c, Revision 3.6 on 2001/05/15 by nmw
 *
 */

/*******************************************************************************

 This file realizes the information gathering for the SAC-WLs (SSAWLI phase).
 The decision to fold or not to fold (SSAWLF) is based on these informations.
 This implementation is aware of the ssa form and does not use any masks.
 Most code is unchanged from the original implementation in WLI.c.

 in this phase every withloop seems to be (nmw) attributed with:
   NWITH_REFERENCED(wl)
     this is the number of identifier that reference this withloop

   NWITH_REFERENCED_FOLD(wl)
     this is the number of references from foldable (modarray) WL, so
     NWITH_REFERENCED(arg_node) >= NWITH_REFERENCED_FOLD(arg_node) should hold

   NWITH_REFERENCES_FOLDED(wl)
     is a counter used in SSAWLF that counts the folding operations. it
     is initialized here with 0.

   NWITH_FOLDABLE(wl)
     marks all WL with constant borders, step and so on.
     only for these WLs withloop folding can be computed.

 additionally all id get the attribute ID_WL, which is pointing to a definition
 assignment with a WL as RHS or NULL else.


 *******************************************************************************

 Usage of arg_info:
 - node[0]: NEXT    : store old information in nested WLs
 - node[1]: WL      : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN  : always the last N_assign node (see WLIassign)
 - node[3]: FUNDEF  : pointer to last fundef node. needed to access vardecs.
 - counter: FOLDABLE: indicates if current withloop is foldable or not
 ******************************************************************************/

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "constants.h"
#include "traverse.h"
#include "optimize.h"
#include "SSAConstantFolding.h"
#include "SSAWithloopFolding.h"
#include "SSAWLI.h"

/*
 * INFO structure
 */
struct INFO {
    info *next;
    node *wl;
    node *assign;
    node *fundef;
    int foldable;
    bool detfoldable;
};

/*
 * INFO macros
 */

#define INFO_SSAWLI_NEXT(n) (n->next)
#define INFO_SSAWLI_WL(n) (n->wl)
#define INFO_SSAWLI_ASSIGN(n) (n->assign)
#define INFO_SSAWLI_FUNDEF(n) (n->fundef)
#define INFO_SSAWLI_FOLDABLE(n) (n->foldable)
#define INFO_SSAWLI_DETFOLDABLE(n) (n->detfoldable)
/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_SSAWLI_NEXT (result) = NULL;
    INFO_SSAWLI_WL (result) = NULL;
    INFO_SSAWLI_ASSIGN (result) = NULL;
    INFO_SSAWLI_FUNDEF (result) = NULL;
    INFO_SSAWLI_FOLDABLE (result) = 0;
    INFO_SSAWLI_DETFOLDABLE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   prf SimplifyFun( prf prf)
 *
 * description:
 *   Transforms special prf names (e.g. F_add_SxA) to the base name (F_add_SxS).
 *   Does this for add, sub, mul, div.
 *
 ******************************************************************************/
static prf
SimplifyFun (prf prf)
{
    DBUG_ENTER ("SimplifyFun");

    switch (prf) {
    case F_add_SxS:
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        prf = F_add_SxS;
        break;

    case F_sub_SxS:
    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
        prf = F_sub_SxS;
        break;

    case F_mul_SxS:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        prf = F_mul_SxS;
        break;

    case F_div_SxS:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        prf = F_div_SxS;
        break;

    default:
        break;
    }

    DBUG_RETURN (prf);
}

/******************************************************************************
 *
 * function:
 *   node *CheckArrayFoldable(node *indexn, node *idn, info *arg_info)
 *
 * description:
 *   This function checks whether the application of sel(indexn,idn) inside
 *   a WL can be folded or not. If yes, the WL described by idn is returned,
 *   else NULL.
 *
 * remark:
 *   This function has only to be called if indexn is a known valid index
 *   transformation. This is supposed here and not checked again.
 *
 ******************************************************************************/
static node *
CheckArrayFoldable (node *indexn, node *idn, info *arg_info)
{
    node *substn = NULL, *thisn;

    DBUG_ENTER ("CheckArrayFoldable");
    DBUG_ASSERT (N_id == NODE_TYPE (indexn), ("Wrong nodetype for indexn"));
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("Wrong nodetype for idn"));

    thisn = INFO_SSAWLI_WL (arg_info);
    if (NWITH_FOLDABLE (thisn)) {
        substn = ID_WL (idn);

#if 0
    /* ID_WL has been set in SSAWLIid called by SSAWLIlet before */
    if (substn == NULL) {
      substn = SSAStartSearchWL( idn, INFO_SSAWLI_ASSIGN( arg_info), 1);
      ID_WL(idn) = substn;
    }
#endif

        if (substn != NULL) {
            /* the idn references a WL which can be used for folding. */
            substn = ASSIGN_RHS (substn);

            /* We have to assure that the access index and the generator
               index of the substitution WL have the same shape. */
            if (NWITH_FOLDABLE (substn)
                && IDS_SHAPE (NPART_VEC (NWITH_PART (substn)), 0)
                     == ID_SHAPE (indexn, 0)) {
                NWITH_REFERENCED_FOLD (substn)++;
            } else {
                substn = NULL;
            }
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
static index_info *
Scalar2ArrayIndex (node *arrayn, node *wln)
{
    index_info *iinfo, *tmpii;
    int elts = 1, ok = 1, i, *valid_permutation;
    node *idn;

    DBUG_ENTER ("Scalar2ArrayIndex");
    DBUG_ASSERT (N_array == NODE_TYPE (arrayn), ("wrong nodetype (array)"));

    if (GetShapeDim (ARRAY_TYPE (arrayn)) >= 0) {
        elts = TYPES_SHAPE (ARRAY_TYPE (arrayn), 0);
        arrayn = ARRAY_AELEMS (arrayn);

        iinfo = SSACreateIndex (elts);
        valid_permutation = Malloc (sizeof (int) * elts);
        for (i = 0; i < elts;) {
            valid_permutation[i++] = 0;
        }

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
                tmpii = SSAValidLocalId (idn);
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
                } else if (0 < (iinfo->permutation[i] = SSALocateIndexVar (idn, wln))) {
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

        if (!ok) {
            iinfo = Free (iinfo);
        }
        valid_permutation = Free (valid_permutation);
    } else {
        iinfo = NULL;
    }

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   int CreateIndexInfoId(node *idn, info *arg_info)
 *
 * description:
 *   creates an index_info from an Id. This could be an index vector, an
 *   index scalar or just a local valid variable.
 *   If the Id is valid, the index_info is created/duplicated and assigned
 *   to the assign node given by arg_info. Returns 1 on success.
 *
 ******************************************************************************/
static int
CreateIndexInfoId (node *idn, info *arg_info)
{
    index_info *iinfo;
    node *assignn, *wln;
    int index_var, i, elts;

    DBUG_ENTER ("CreateIndexInfoId");

    assignn = INFO_SSAWLI_ASSIGN (arg_info);
    wln = INFO_SSAWLI_WL (arg_info);

    DBUG_ASSERT (!SSAINDEX (assignn), ("index_info already assigned"));

    if (GetShapeDim (ID_TYPE (idn)) >= 0) {
        /* index var? */
        index_var = SSALocateIndexVar (idn, wln);
        if (index_var) {
            iinfo = SSACreateIndex ((index_var > 0) ? 0 : ID_SHAPE (idn, 0));
            SSAINDEX (assignn) = iinfo; /* make this N_assign valid */

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
            iinfo = SSAValidLocalId (idn);
            if (iinfo) {
                SSAINDEX (assignn) = SSADuplicateIndexInfo (iinfo);
            }
        }
    }

    DBUG_RETURN (NULL != SSAINDEX (assignn));
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoSxS(node *prfn, info *arg_info)
 *
 * description:
 *   checks if application of the prfn is a valid transformation in
 *   this WL wln and, if valid, creates an INDEX_INFO at the assignn.
 *
 ******************************************************************************/
static void
CreateIndexInfoSxS (node *prfn, info *arg_info)
{
    int id_no = 0, index_var = 0;
    index_info *iinfo;
    node *idn = NULL, *assignn, *wln;

    DBUG_ENTER (" CreateIndexInfoSxS");

    assignn = INFO_SSAWLI_ASSIGN (arg_info);
    wln = INFO_SSAWLI_WL (arg_info);

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
        iinfo = SSAValidLocalId (idn);
        if (!iinfo) /* maybe it's an index var. */
            index_var = SSALocateIndexVar (idn, wln);

        if (iinfo || index_var) {
            iinfo = SSACreateIndex (0); /* create a scalar index_info */
            SSAINDEX (assignn) = iinfo; /* make this N_assign valid */

            /* if the Id is an index var... */
            if (index_var) {
                iinfo->last[0] = NULL;
                iinfo->permutation[0] = index_var; /* set permutation, always != -1 */
            } else {
                iinfo->last[0] = SSAValidLocalId (idn);
                /* else copy permutation from last index_info */
                iinfo->permutation[0] = iinfo->last[0]->permutation[0];
            }

            iinfo->prf = SimplifyFun (PRF_PRF (prfn));
            iinfo->const_arg[0]
              = NUM_VAL (((id_no == 1) ? PRF_ARG2 (prfn) : PRF_ARG1 (prfn)));
            iinfo->arg_no = (id_no == 1) ? 2 : 1;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoA(node *prfn, info *arg_info)
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
 ******************************************************************************/
static void
CreateIndexInfoA (node *prfn, info *arg_info)
{
    int elts, i, index;
    int val = 0;
    node *idn = NULL;
    node *cf_node, *assignn, *wln;
    node *args[3];
    index_info *iinfo, *tmpinfo;
    constant *const1, *const2, *constn = NULL;
    int *const_elems;

    DBUG_ENTER (" CreateIndexInfoA");

    assignn = INFO_SSAWLI_ASSIGN (arg_info);
    wln = INFO_SSAWLI_WL (arg_info);

    const1 = COAST2Constant (PRF_ARG1 (prfn));
    const2 = COAST2Constant (PRF_ARG2 (prfn));

    /* Which argument is the constant (so which will be the Id)? */
    if (const1 != NULL) {
        idn = PRF_ARG2 (prfn);
        constn = const1;
    }
    if (const2 != NULL) {
        idn = PRF_ARG1 (prfn);
        constn = const2;
    }
    DBUG_ASSERT (((const1 == NULL) || (const2 == NULL)), "both arguments are constants!");
    if (constn != NULL) {
        const_elems = COGetDataVec (constn);

        /* Is idn an Id of an index vector (or transformation)? */
        if (NODE_TYPE (idn) == N_id) {
            tmpinfo = SSAValidLocalId (idn);
            index = SSALocateIndexVar (idn, wln);

            /* The Id is the index vector itself or, else it has to
               be an Id which is a valid vector. It must not be based
               on an index scalar (we do want "i prfop [c,c,c]"). */
            if ((GetShapeDim (ID_TYPE (idn)) >= 0)
                && ((-1 == index) ||
                    /* ^^^ index vector itself */
                    (tmpinfo && (1 == TYPES_DIM (ID_TYPE (idn)))))) {
                /* ^^^ valid local id (vector) */
                elts = ID_SHAPE (idn, 0);
                iinfo = SSACreateIndex (elts);
                SSAINDEX (assignn) = iinfo; /* make this N_assign valid */

                iinfo->arg_no = (constn == const1) ? 1 : 2;
                iinfo->prf = SimplifyFun (PRF_PRF (prfn));

                for (i = 0; i < elts; i++) {
                    val = (COGetDim (constn) == 0) ? const_elems[0] : const_elems[i];

                    if (-1 == index) { /* index vector */
                        iinfo->last[i] = NULL;
                        iinfo->permutation[i] = i + 1;
                    } else { /* local var, not index vector */
                        if ((iinfo->permutation[i] = tmpinfo->permutation[i])) { /* !!! */
                            iinfo->last[i] = tmpinfo;
                        } else {
                            iinfo->last[i] = NULL; /* elt is constant */
                        }
                    }

                    if (iinfo->permutation[i]) {
                        iinfo->const_arg[i] = val;
                    } else {
                        /*
                         * constant. Use SSACFFoldPrfExpr() to constantfold.,
                         * there MUST always be three args in arg array!!!
                         */
                        args[0] = MakeNum (val);
                        args[1] = MakeNum (tmpinfo->const_arg[i]);
                        args[2] = NULL;
                        cf_node = SSACFFoldPrfExpr (PRF_PRF (prfn), args);
                        DBUG_ASSERT ((NODE_TYPE (cf_node) == N_num),
                                     "non integer result from constant folding");
                        iinfo->const_arg[i] = NUM_VAL (cf_node);
                        args[0] = FreeTree (args[0]);
                        args[1] = FreeTree (args[1]);
                        cf_node = FreeTree (cf_node);
                    }
                }
            } /* this Id is valid. */
        }     /* this is an Id. */

        /* Is it a contruction based on index scalars ([i,i,c])? */
        if (NODE_TYPE (idn) == N_array) {
            iinfo = Scalar2ArrayIndex (idn, wln);
            if (iinfo) {
                /* This is a valid vector. Permutation and last of index_info are
                   already set. But we still need to handle the prf. */
                elts = iinfo->vector;
                SSAINDEX (assignn) = iinfo;

                iinfo->arg_no = (constn == const1) ? 1 : 2;
                iinfo->prf = SimplifyFun (PRF_PRF (prfn));

                for (i = 0; i < elts; i++) {
                    val = (COGetDim (constn) == 0) ? const_elems[0] : const_elems[i];

                    /* is the element in the other vector a constant, too?
                       Then we have to fold immedeately. */
                    if (!iinfo->permutation[i]) {
                        /*
                         * constant. Use SSACFFoldPrfExpr() to constantfold.
                         * there MUST always be 3 args in array!!!
                         */
                        args[0] = MakeNum (val);
                        args[1] = MakeNum (iinfo->const_arg[i]);
                        args[2] = NULL;
                        cf_node = SSACFFoldPrfExpr (PRF_PRF (prfn), args);
                        DBUG_ASSERT ((NODE_TYPE (cf_node) == N_num),
                                     "non integer result from constant folding");
                        iinfo->const_arg[i] = NUM_VAL (cf_node);
                        args[0] = Free (args[0]);
                        args[1] = Free (args[1]);
                        cf_node = Free (cf_node);
                    } else {
                        iinfo->const_arg[i] = val;
                    }
                }
            }
        }

        constn = COFreeConstant (constn);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   start WLI traversal..
 *
 ******************************************************************************/
node *
SSAWLIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLIfundef");

    INFO_SSAWLI_WL (arg_info) = NULL;
    INFO_SSAWLI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIassign(node *arg_node, info *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node and traverse instruction
 *
 ******************************************************************************/
node *
SSAWLIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLIassign");

    INFO_SSAWLI_ASSIGN (arg_info) = arg_node;

    if (SSAINDEX (arg_node) != NULL) {
        /* this is important. Only index transformations
           with a non-null SSAINDEX are valid. See SSAWLIlet. Before WLI, this
           pointer may be non null (somwhere wrong initialisation -> better
           use MakeAssign()!!! ) */
        SSAINDEX (arg_node) = FreeIndexInfo (SSAINDEX (arg_node));
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
SSAWLIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLIcond");

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = Trav (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = Trav (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIap(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse args
 *   traverse in applicated fundef if special one (like SSAWithloopFolding())
 *
 ******************************************************************************/
node *
SSAWLIap (node *arg_node, info *arg_info)
{
    info *new_arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("SSAWLTap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef
     */
    if ((AP_FUNDEF (arg_node) != NULL) && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_SSAWLI_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        tmp_tab = act_tab;
        act_tab = ssawli_tab;
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        /* break after WLI? */
        if ((break_after != PH_sacopt) || strcmp (break_specifier, "wli")) {
            /* SSAWLF traversal: fold WLs */
            act_tab = ssawlf_tab;
            AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);
        }

        act_tab = tmp_tab;

        new_arg_info = FreeInfo (new_arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_Nwith) we want to increment
 *   the number of references to it (NWITH_REFERENCED). Therefore the
 *   WL node has to be found via avis_ssaassign backlink.
 *
 ******************************************************************************/
node *
SSAWLIid (node *arg_node, info *arg_info)
{
    node *assignn;

    DBUG_ENTER ("SSAWLIid");

    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if ((assignn != NULL) && (NODE_TYPE (ASSIGN_RHS (assignn)) == N_Nwith)) {
        ID_WL (arg_node) = assignn;
        /*
         * arg_node describes a WL, so NWITH_REFERENCED has to be incremented
         */
        (NWITH_REFERENCED (ASSIGN_RHS (assignn)))++;
    } else {
        /* id is not defined by a withloop */
        ID_WL (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLIlet(node *arg_node, info *arg_info)
 *
 * description:
 *   We are interested in prf applications. This may be transformations of
 *   later used index vectors for arrays to fold.
 *   If we find an F_sel, we check, based on the above made transformation
 *   checks, if the index vector is valid (i.e. the array reference is
 *   foldable).
 *
 ******************************************************************************/
node *
SSAWLIlet (node *arg_node, info *arg_info)
{
    node *exprn, *tmpn, *old_assignn;
    prf prf;

    DBUG_ENTER ("SSAWLIlet");

    /* traverse sons first so that ID_WL of every Id is defined (or NULL). */
    old_assignn = INFO_SSAWLI_ASSIGN (arg_info);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    INFO_SSAWLI_ASSIGN (arg_info) = old_assignn;

    /* if we are inside a WL we have to search for valid index transformations. */
    if (INFO_SSAWLI_WL (arg_info)) {
        /* if this is a prf, we are interrested in transformations like +,*,-,/
           and in indexing (F_sel). */
        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn)) {
            prf = PRF_PRF (exprn);
            switch (prf) {
                /* this maybe an assignment which calculates an index for an
                   array to fold. */
            case F_add_SxS:
            case F_sub_SxS:
            case F_mul_SxS:
            case F_div_SxS: /* both args are scalars */
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

            case F_sel:
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
                       WLF phase we know for sure that a sel-prf with a
                       valid index_info can be folded. But that still doesn't
                       mean that we want to fold. */
                    tmpn
                      = CheckArrayFoldable (PRF_ARG1 (exprn), PRF_ARG2 (exprn), arg_info);
                    if (!tmpn) {
                        SSAINDEX (INFO_SSAWLI_ASSIGN (arg_info))
                          = FreeIndexInfo (SSAINDEX (INFO_SSAWLI_ASSIGN (arg_info)));
                    }
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
            SSAINDEX (INFO_SSAWLI_ASSIGN (arg_info))
              = Scalar2ArrayIndex (exprn, INFO_SSAWLI_WL (arg_info));
        }

    } /* is this a WL? */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLINwith(node *arg_node, info *arg_info)
 *
 * description:
 *   start gathering information for this WL.
 *   First some initialisations in the WL structure are done, then the
 *   N_Nparts are traversed (which call the appropriate N_Ncode subtrees).
 *
 ******************************************************************************/
node *
SSAWLINwith (node *arg_node, info *arg_info)
{
    node *tmpn;
    info *tmpi;

    DBUG_ENTER ("SSAWLINwith");

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpi = MakeInfo ();
    INFO_SSAWLI_FUNDEF (tmpi) = INFO_SSAWLI_FUNDEF (arg_info);
    INFO_SSAWLI_ASSIGN (tmpi) = INFO_SSAWLI_ASSIGN (arg_info);
    INFO_SSAWLI_NEXT (tmpi) = arg_info;
    arg_info = tmpi;

    /* initialize WL traversal */
    INFO_SSAWLI_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        NCODE_FLAG (tmpn) = FALSE;
        tmpn = NCODE_NEXT (tmpn);
    }
    /* Attribut FOLDABLE will be set later. The others are initialized here. */
    NWITH_REFERENCED (arg_node) = 0;
    NWITH_REFERENCED_FOLD (arg_node) = 0;
    NWITH_REFERENCES_FOLDED (arg_node) = 0;

    /* initialize determination of FOLDABLE */
    INFO_SSAWLI_FOLDABLE (arg_info) = TRUE;
    INFO_SSAWLI_DETFOLDABLE (arg_info) = TRUE;

    /* determine FOLDABLE */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }
    NWITH_FOLDABLE (INFO_SSAWLI_WL (arg_info)) = INFO_SSAWLI_FOLDABLE (arg_info);
    INFO_SSAWLI_DETFOLDABLE (arg_info) = FALSE;

    /* traverse all parts (and implicitely bodies) */
    DBUG_PRINT ("WLI", ("searching code of  WL in line %d", NODE_LINE (arg_node)));
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }
    DBUG_PRINT ("WLI", ("searching done"));

    /* traverse N_Nwithop */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* restore arg_info */
    tmpi = arg_info;
    arg_info = INFO_SSAWLI_NEXT (arg_info);
    tmpi = FreeInfo (tmpi);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLINwithop(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray we have to check whether the base array Id can be
 *   eleminated by folding. Then NWITH_REFERENCED_FOLD is incremented.
 *
 ******************************************************************************/
node *
SSAWLINwithop (node *arg_node, info *arg_info)
{
    node *substn;

    DBUG_ENTER ("SSAWLINwithop");

    arg_node = TravSons (arg_node, arg_info);

    if ((WO_modarray == NWITHOP_TYPE (arg_node))
        && NWITH_FOLDABLE (INFO_SSAWLI_WL (arg_info))) {
        substn = ID_WL (NWITHOP_ARRAY (arg_node));
        /*
         * we just traversed through the sons so SSAWLIid for NWITHOP_ARRAY
         * has been called and its result is stored in ID_WL().
         */
        if (substn) {
            (NWITH_REFERENCED_FOLD (ASSIGN_RHS (substn)))++;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLINpart(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse appropriate generator and body.
 *
 ******************************************************************************/
node *
SSAWLINpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLINpart");

    if (INFO_SSAWLI_DETFOLDABLE (arg_info)) {

        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    } else {
        /*
         * traverse code. But do this only once, even if there are more than
         * one referencing generators.
         * This is just a cross reference, so just traverse, do not assign the
         * resulting node.
         */
        if (!NCODE_FLAG (NPART_CODE (arg_node))) {
            Trav (NPART_CODE (arg_node), arg_info);
        }
    }

    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLINgenerator( node *arg_node, info *arg_info)
 *
 * description:
 *   checks whether borders, step and width are constant. The result is stored
 *   in INFO_SSAWLI_FOLDABLE( arg_info).
 *
 ******************************************************************************/
node *
SSAWLINgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLINgenerator");

    INFO_SSAWLI_FOLDABLE (arg_info)
      = INFO_SSAWLI_FOLDABLE (arg_info) && COIsConstant (NGEN_BOUND1 (arg_node));
    INFO_SSAWLI_FOLDABLE (arg_info)
      = INFO_SSAWLI_FOLDABLE (arg_info) && COIsConstant (NGEN_BOUND2 (arg_node));

    if (NGEN_STEP (arg_node) != NULL) {
        INFO_SSAWLI_FOLDABLE (arg_info)
          = INFO_SSAWLI_FOLDABLE (arg_info) && COIsConstant (NGEN_STEP (arg_node));
        if (NGEN_WIDTH (arg_node) != NULL) {
            INFO_SSAWLI_FOLDABLE (arg_info)
              = INFO_SSAWLI_FOLDABLE (arg_info) && COIsConstant (NGEN_WIDTH (arg_node));
        }
    } else {
        DBUG_ASSERT ((NGEN_WIDTH (arg_node) == NULL), "width vector without step vector");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLINcode( node *arg_node, info *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 ******************************************************************************/
node *
SSAWLINcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLINcode");

    DBUG_ASSERT ((!NCODE_FLAG (arg_node)), "Body traversed a second time in WLI");

    /*
     * this body has been traversed and all information has been gathered.
     */
    NCODE_FLAG (arg_node) = TRUE;

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
