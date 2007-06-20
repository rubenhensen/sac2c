/*******************************************************************************

 $Id$

 This file realizes the information gathering for the SAC-WLs (SSAWLI phase).
 The decision to fold or not to fold (SSAWLF) is based on these informations.

   ASSIGN_INFO : whenever there is an index-offset calculation in the RHS
   ID_WL : for all N-ids pointing to WL results

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

 additionally all N_id get the attribute ID_WL:
   ID_WL(id)
     points to a defining wl if id is defined by a wl, NULL otherwise

 furthermore, N_assign nodes within wls may obtain the ASSIGN_INFO attribute:
   ASSIGN_INFO(assign)
     if present, it indicates an index-offset calculation into a wl-defined
     variable in its RHS
     Note here, that this is referred to by the macro SSAINDEX !


 *******************************************************************************

 Usage of arg_info:
 - node[0]: NEXT    : store old information in nested WLs
 - node[1]: WL      : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN  : always the last N_assign node (see WLIassign)
 - node[3]: FUNDEF  : pointer to last fundef node. needed to access vardecs.
 - counter: FOLDABLE: indicates if current withloop is foldable or not
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "shape.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "dbug.h"
#include "constants.h"
#include "traverse.h"
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
#define INFO_NEXT(n) (n->next)
#define INFO_WL(n) (n->wl)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_FOLDABLE(n) (n->foldable)
#define INFO_DETFOLDABLE(n) (n->detfoldable)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_NEXT (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_FOLDABLE (result) = 0;
    INFO_DETFOLDABLE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   prf SimplifyFun( prf prf)
 *
 * description:
 *   Transforms special prf names (e.g. F_add_SxV) to the base name (F_add_SxS).
 *   Does this for add, sub, mul, div.
 *
 ******************************************************************************/
static prf
SimplifyFun (prf prf)
{
    DBUG_ENTER ("SimplifyFun");

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        prf = F_add_SxS;
        break;

    case F_sub_SxS:
    case F_sub_SxV:
    case F_sub_VxS:
    case F_sub_VxV:
        prf = F_sub_SxS;
        break;

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        prf = F_mul_SxS;
        break;

    case F_div_SxS:
    case F_div_SxV:
    case F_div_VxS:
    case F_div_VxV:
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

    thisn = INFO_WL (arg_info);
    if (WITH_ISFOLDABLE (thisn)) {
        substn = ID_WL (idn);

        if (substn != NULL) {
            /* the idn references a WL which can be used for folding. */
            substn = ASSIGN_RHS (substn);

            /* We have to assure that the access index and the generator
               index of the substitution WL have the same shape. */
            if (WITH_ISFOLDABLE (substn) && TUshapeKnown (IDS_NTYPE (WITH_VEC (substn)))
                && TUshapeKnown (ID_NTYPE (indexn))
                && SHcompareShapes (TYgetShape (IDS_NTYPE (WITH_VEC (substn))),
                                    TYgetShape (ID_NTYPE (indexn)))) {
                WITH_REFERENCED_FOLD (substn)++;
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
    ntype *atype;

    DBUG_ENTER ("Scalar2ArrayIndex");
    DBUG_ASSERT (N_array == NODE_TYPE (arrayn), ("wrong nodetype (array)"));

    atype = NTCnewTypeCheck_Expr (arrayn);
    if (TUshapeKnown (atype)) {
        elts = SHgetExtent (TYgetShape (atype), 0);
        arrayn = ARRAY_AELEMS (arrayn);

        iinfo = WLFcreateIndex (elts);
        valid_permutation = MEMmalloc (sizeof (int) * elts);
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
                tmpii = WLFvalidLocalId (idn);
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
                } else if (0 < (iinfo->permutation[i] = WLFlocateIndexVar (idn, wln))) {
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
            iinfo = MEMfree (iinfo);
        }
        valid_permutation = MEMfree (valid_permutation);
    } else {
        iinfo = NULL;
    }

    atype = TYfreeType (atype);

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

    assignn = INFO_ASSIGN (arg_info);
    wln = INFO_WL (arg_info);

    DBUG_ASSERT (!ASSIGN_INDEX (assignn), ("index_info already assigned"));

    if (TUshapeKnown (ID_NTYPE (idn))) {
        /* index var? */
        index_var = WLFlocateIndexVar (idn, wln);
        if (index_var != 0) {
            if (index_var < 0) {
                /* Index vector */
                iinfo = WLFcreateIndex (SHgetExtent (TYgetShape (ID_NTYPE (idn)), 0));
            } else {
                iinfo = WLFcreateIndex (0);
            }
            ASSIGN_INDEX (assignn) = iinfo; /* make this N_assign valid */

            if (-1 == index_var) { /* index vector */
                elts = SHgetExtent (TYgetShape (ID_NTYPE (idn)), 0);
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
            iinfo = WLFvalidLocalId (idn);
            if (iinfo) {
                ASSIGN_INDEX (assignn) = WLFduplicateIndexInfo (iinfo);
            }
        }
    }

    DBUG_RETURN (NULL != ASSIGN_INDEX (assignn));
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

    assignn = INFO_ASSIGN (arg_info);
    wln = INFO_WL (arg_info);

    /* CF has been done, so we just search for an Id and a constant.
       Since we do not want to practice constant folding here we ignore
       prfs with two constants. */
    if (N_id == NODE_TYPE (PRF_ARG1 (prfn)) && /* first arg is an Id */
        N_num == NODE_TYPE (PRF_ARG2 (prfn))) {
        /* second arg is a numeric constant */
        id_no = 1;
        idn = PRF_ARG1 (prfn);
    } else if (N_id == NODE_TYPE (PRF_ARG2 (prfn))
               && N_num == NODE_TYPE (PRF_ARG1 (prfn))) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
    }

    if (id_no != 0) {
        /* we found a constant and an Id. If this Id is a vaild Id (i.e.
           it is declared in the generator or it is a valid local Id)
           this transformation is valid, too. */
        iinfo = WLFvalidLocalId (idn);
        if (iinfo == NULL) {
            /* maybe it's an index var. */
            index_var = WLFlocateIndexVar (idn, wln);
        }

        if ((iinfo != NULL) || (index_var != 0)) {
            iinfo = WLFcreateIndex (0);     /* create a scalar index_info */
            ASSIGN_INDEX (assignn) = iinfo; /* make this N_assign valid */

            /* if the Id is an index var... */
            if (index_var != 0) {
                iinfo->last[0] = NULL;
                iinfo->permutation[0] = index_var; /* set permutation, always != -1 */
            } else {
                iinfo->last[0] = WLFvalidLocalId (idn);
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
 *   VxS (SxV): iv prfop c
 *              [i,i,c] prfop c
 *   VxV      : iv prfop [c,c,c]
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
    node *assignn, *wln;
    index_info *iinfo, *tmpinfo = NULL;
    constant *const1, *const2, *constn = NULL;
    int *const_elems;

    DBUG_ENTER (" CreateIndexInfoA");

    assignn = INFO_ASSIGN (arg_info);
    wln = INFO_WL (arg_info);

    const1 = COaST2Constant (PRF_ARG1 (prfn));
    const2 = COaST2Constant (PRF_ARG2 (prfn));

    /* Which argument is constant (so which will be an Id)? */
    if (const1 != NULL) {
        if (const2 != NULL) {
            /**
             * if both are constant, no index structure is required.
             * As a consequence, both constants can be freed.
             * This should fix bug 68!
             */
            const1 = COfreeConstant (const1);
            const2 = COfreeConstant (const2);

        } else {
            idn = PRF_ARG2 (prfn);
            constn = const1;
        }
    } else {
        if (const2 != NULL) {
            idn = PRF_ARG1 (prfn);
            constn = const2;
        }
    }

    if (constn != NULL) {
        const_elems = COgetDataVec (constn);

        /* Is idn an Id of an index vector (or transformation)? */
        if (NODE_TYPE (idn) == N_id) {
            tmpinfo = WLFvalidLocalId (idn);
            index = WLFlocateIndexVar (idn, wln);

            /* The Id is the index vector itself or, else it has to
               be an Id which is a valid vector. It must not be based
               on an index scalar (we do want "i prfop [c,c,c]"). */
            if (TUshapeKnown (ID_NTYPE (idn))
                && ((-1 == index) ||
                    /* ^^^ index vector itself */
                    (tmpinfo && (1 == TYgetDim (ID_NTYPE (idn)))))) {
                /* ^^^ valid local id (vector) */
                elts = SHgetExtent (TYgetShape (ID_NTYPE (idn)), 0);
                iinfo = WLFcreateIndex (elts);
                ASSIGN_INDEX (assignn) = iinfo; /* make this N_assign valid */

                iinfo->arg_no = (constn == const1) ? 1 : 2;
                iinfo->prf = SimplifyFun (PRF_PRF (prfn));

                for (i = 0; i < elts; i++) {
                    val = (COgetDim (constn) == 0) ? const_elems[0] : const_elems[i];

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
                        node *expr;
                        ntype *nt;
                        expr = TCmakePrf2 (PRF_PRF (prfn), TBmakeNum (val),
                                           TBmakeNum (tmpinfo->const_arg[i]));
                        nt = NTCnewTypeCheck_Expr (expr);
                        DBUG_ASSERT ((TYisAKV (nt)) && (TYgetDim (nt) == 0)
                                       && (TYgetSimpleType (TYgetScalar (nt)) == T_int),
                                     "non integer result from constant folding!");
                        iinfo->const_arg[i] = ((int *)COgetDataVec (TYgetValue (nt)))[0];

                        nt = TYfreeType (nt);
                        expr = FREEdoFreeNode (expr);
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
                ASSIGN_INDEX (assignn) = iinfo;

                iinfo->arg_no = (constn == const1) ? 1 : 2;
                iinfo->prf = SimplifyFun (PRF_PRF (prfn));

                for (i = 0; i < elts; i++) {
                    val = (COgetDim (constn) == 0) ? const_elems[0] : const_elems[i];

                    /* is the element in the other vector a constant, too?
                       Then we have to fold immedeately. */
                    if (!iinfo->permutation[i]) {
                        node *expr;
                        ntype *nt;
                        expr = TCmakePrf2 (PRF_PRF (prfn), TBmakeNum (val),
                                           TBmakeNum (tmpinfo->const_arg[i]));
                        nt = NTCnewTypeCheck_Expr (expr);
                        DBUG_ASSERT ((TYisAKV (nt)) && (TYgetDim (nt) == 0)
                                       && (TYgetSimpleType (TYgetScalar (nt)) == T_int),
                                     "non integer result from constant folding!");
                        iinfo->const_arg[i] = ((int *)COgetDataVec (TYgetValue (nt)))[0];

                        nt = TYfreeType (nt);
                        expr = FREEdoFreeNode (expr);
                    } else {
                        iinfo->const_arg[i] = val;
                    }
                }
            }
        }

        constn = COfreeConstant (constn);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *WLIfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   start WLI traversal..
 *
 ******************************************************************************/
node *
WLIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIfundef");

    INFO_WL (arg_info) = NULL;
    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIassign(node *arg_node, info *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node and traverse instruction
 *
 ******************************************************************************/
node *
WLIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIassign");

    INFO_ASSIGN (arg_info) = arg_node;

    if (ASSIGN_INDEX (arg_node) != NULL) {
        /* this is important. Only index transformations
           with a non-null ASSIGN_INDEX are valid. See SSAWLIlet. Before WLI, this
           pointer may be non null (somwhere wrong initialisation -> better
           use MakeAssign()!!! ) */
        ASSIGN_INDEX (arg_node) = FREEfreeIndexInfo (ASSIGN_INDEX (arg_node));
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
WLIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSAWLIcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_with) we want to increment
 *   the number of references to it (WITH_REFERENCED). Therefore the
 *   WL node has to be found via avis_ssaassign backlink.
 *
 ******************************************************************************/
node *
WLIid (node *arg_node, info *arg_info)
{
    node *assignn;

    DBUG_ENTER ("WLIid");

    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if ((assignn != NULL) && (NODE_TYPE (ASSIGN_RHS (assignn)) == N_with)) {
        ID_WL (arg_node) = assignn;
        /*
         * arg_node describes a WL, so NWITH_REFERENCED has to be incremented
         */
        (WITH_REFERENCED (ASSIGN_RHS (assignn))) += 1;
    } else {
        /* id is not defined by a withloop */
        ID_WL (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIlet(node *arg_node, info *arg_info)
 *
 * description:
 *   We are interested in prf applications. This may be transformations of
 *   later used index vectors for arrays to fold.
 *   If we find an F_sel_VxA, we check, based on the above made transformation
 *   checks, if the index vector is valid (i.e. the array reference is
 *   foldable).
 *
 ******************************************************************************/
node *
WLIlet (node *arg_node, info *arg_info)
{
    node *exprn, *tmpn, *old_assignn;
    prf prf;

    DBUG_ENTER ("WLIlet");

    /* traverse sons first so that ID_WL of every Id is defined (or NULL). */
    old_assignn = INFO_ASSIGN (arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_ASSIGN (arg_info) = old_assignn;

    /* if we are inside a WL we have to search for valid index transformations. */
    if (INFO_WL (arg_info)) {
        /* if this is a prf, we are interrested in transformations like +,*,-,/
           and in indexing (F_sel_VxA). */
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

            case F_add_SxV:
            case F_sub_SxV:
            case F_mul_SxV:
            case F_div_SxV:

            case F_add_VxS:
            case F_sub_VxS:
            case F_mul_VxS:
            case F_div_VxS:

            case F_add_VxV:
            case F_sub_VxV:
            case F_mul_VxV:
            case F_div_VxV:
                CreateIndexInfoA (exprn, arg_info);
                break;

            case F_sel_VxA:
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
                        ASSIGN_INDEX (INFO_ASSIGN (arg_info))
                          = FREEfreeIndexInfo (ASSIGN_INDEX (INFO_ASSIGN (arg_info)));
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
            ASSIGN_INDEX (INFO_ASSIGN (arg_info))
              = Scalar2ArrayIndex (exprn, INFO_WL (arg_info));
        }

    } /* is this a WL? */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwith(node *arg_node, info *arg_info)
 *
 * description:
 *   start gathering information for this WL.
 *   First some initialisations in the WL structure are done, then the
 *   N_Nparts are traversed (which call the appropriate N_Ncode subtrees).
 *
 ******************************************************************************/
node *
WLIwith (node *arg_node, info *arg_info)
{
    node *tmpn;
    info *tmpi;

    DBUG_ENTER ("WLIwith");

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpi = MakeInfo ();
    INFO_FUNDEF (tmpi) = INFO_FUNDEF (arg_info);
    INFO_ASSIGN (tmpi) = INFO_ASSIGN (arg_info);
    INFO_NEXT (tmpi) = arg_info;
    arg_info = tmpi;

    /* initialize WL traversal */
    INFO_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = WITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        CODE_VISITED (tmpn) = FALSE;
        tmpn = CODE_NEXT (tmpn);
    }
    /* Attribut FOLDABLE will be set later. The others are initialized here. */
    WITH_REFERENCED (arg_node) = 0;
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCES_FOLDED (arg_node) = 0;

    /* initialize determination of FOLDABLE */
    INFO_FOLDABLE (arg_info) = TRUE;
    INFO_DETFOLDABLE (arg_info) = TRUE;

    /* determine FOLDABLE */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_ISFOLDABLE (INFO_WL (arg_info)) = INFO_FOLDABLE (arg_info);
    INFO_DETFOLDABLE (arg_info) = FALSE;

    /* traverse all parts (and implicitely bodies) */
    DBUG_PRINT ("WLI", ("searching code of  WL in line %d", NODE_LINE (arg_node)));
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    DBUG_PRINT ("WLI", ("searching done"));

    /* traverse N_Nwithop */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* restore arg_info */
    tmpi = arg_info;
    arg_info = INFO_NEXT (arg_info);
    tmpi = FreeInfo (tmpi);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLImodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray we have to check whether the base array Id can be
 *   eleminated by folding. Then NWITH_REFERENCED_FOLD is incremented.
 *
 ******************************************************************************/
node *
WLImodarray (node *arg_node, info *arg_info)
{
    node *substn;

    DBUG_ENTER ("WLImodarray");

    arg_node = TRAVcont (arg_node, arg_info);

    if (WITH_ISFOLDABLE (INFO_WL (arg_info))) {
        substn = ID_WL (MODARRAY_ARRAY (arg_node));
        /*
         * we just traversed through the sons so SSAWLIid for NWITHOP_ARRAY
         * has been called and its result is stored in ID_WL().
         */
        if (substn != NULL) {
            (WITH_REFERENCED_FOLD (ASSIGN_RHS (substn)))++;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINpart(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse appropriate generator and body.
 *
 ******************************************************************************/
node *
WLIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIpart");

    if (INFO_DETFOLDABLE (arg_info)) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    } else {
        /*
         * traverse code. But do this only once, even if there are more than
         * one referencing generators.
         * This is just a cross reference, so just traverse, do not assign the
         * resulting node.
         */
        if (!CODE_VISITED (PART_CODE (arg_node))) {
            TRAVdo (PART_CODE (arg_node), arg_info);
        }
    }

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIgenerator( node *arg_node, info *arg_info)
 *
 * description:
 *   checks whether borders, step and width are constant. The result is stored
 *   in INFO_FOLDABLE( arg_info).
 *
 ******************************************************************************/
node *
WLIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIgenerator");

    INFO_FOLDABLE (arg_info)
      = INFO_FOLDABLE (arg_info) && COisConstant (GENERATOR_BOUND1 (arg_node));
    INFO_FOLDABLE (arg_info)
      = INFO_FOLDABLE (arg_info) && COisConstant (GENERATOR_BOUND2 (arg_node));

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_FOLDABLE (arg_info)
          = INFO_FOLDABLE (arg_info) && COisConstant (GENERATOR_STEP (arg_node));
        if (GENERATOR_WIDTH (arg_node) != NULL) {
            INFO_FOLDABLE (arg_info)
              = INFO_FOLDABLE (arg_info) && COisConstant (GENERATOR_WIDTH (arg_node));
        }
    } else {
        DBUG_ASSERT ((GENERATOR_WIDTH (arg_node) == NULL),
                     "width vector without step vector");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIcode( node *arg_node, info *arg_info)
 *
 * description:
 *   marks this N_code node as processed and enters the code block.
 *
 ******************************************************************************/
node *
WLIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIcode");

    DBUG_ASSERT ((!CODE_VISITED (arg_node)), "Body traversed a second time in WLI");

    /*
     * this body has been traversed and all information has been gathered.
     */
    CODE_VISITED (arg_node) = TRUE;

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPR (arg_node) = TRAVdo (CODE_CEXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn WLIDoWLI( node* arg_node)
 *
 * @brief starts SSAWLI traversal
 *
 * @param arg_node node to process
 *
 *****************************************************************************/
node *
WLIdoWLI (node *arg_node)
{
    info *info;

    DBUG_ENTER ("WLIdoWLI");

    info = MakeInfo ();

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    TRAVpush (TR_wli);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}
