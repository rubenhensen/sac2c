/*******************************************************************************

 This file realizes the information gathering for the SAC-WLs (SSAWLI phase).
 The decision to fold or not to fold (SSAWLF) is based on these informations.

 This implementation is aware of the ssa form and does not use any masks.
 Most code is unchanged from the original implementation in WLI.c.

 Attributes are gathered at three different node types:
 - N_with,
 - N_id, and
 - N_assign

 In detail, these are:

--------------------------------------------------------------------------------
  at N_with:
--------------------------------------------------------------------------------

   WITH_REFERENCED(wl)    (int)
     this is the number of identifiers that reference this withloop
     [ initialized in WLIwith; incremented in WLIid; top-down traversal
       ensures initialisation before incrments!]

   WITH_FOLDABLE(wl)      (bool)
     set to true for all WL whose partitions ALL have constants for bounds
     step and width.
     [ set in WLIgenerator ]

   WITH_REFERENCED_FOLD(wl)
     this is the number of references from foldable (modarray) WL, so
     WITH_REFERENCED(arg_node) >= WITH_REFERENCED_FOLD(arg_node) should hold
     [ this is being initialised in WLIwith and incremented in WLIlet ]

   WITH_REFERENCES_FOLDED(wl)
     is a counter used in SSAWLF that counts the folding operations. it
     is initialized here with 0.

--------------------------------------------------------------------------------
  at N_id:
--------------------------------------------------------------------------------

   ID_WL(id)
     points to the assignment of a defining wl if id is defined by a wl,
     NULL otherwise.
     [ set in WLIid ]

--------------------------------------------------------------------------------
  at N_assign:
--------------------------------------------------------------------------------

   ASSIGN_INDEX(assign)
     if present, it indicates an index-offset calculation into a wl-defined
     variable in its RHS
     Note here, that this is referred to by the macro SSAINDEX !
     [ set in WLIlet ]


 *******************************************************************************

General remarks:
  Most of the code is rather straight forward. The main chrunching happens in
  WLIlet. Here, the attributation of ASSIGN_INDEX happens and previously
  gathered information such as ID_WL or WITH_FOLDABLE is being used to decide
  whether a given selection operation could be folded.
  If this is the case, WITH_REFERENCED_FOLD is incremented at the WL that is
  being folded.

 *******************************************************************************

 Usage of arg_info:
 - INFO_NEXT    : store old information in nested WLs
 - INFO_WL      : reference to base node of current WL (N_with)
 - INFO_ASSIGN  : always the last N_assign node (see WLIassign)
 - INFO_FUNDEF  : pointer to last fundef node. needed to access vardecs.
 - INFO_FOLDABLE: indicates if current withloop is foldable or not
 - INFO_PMLUT   : pattern matching lut for current context
 - INFO_LOCALFUN: indicates if current fundef is a localfun
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

#define DBUG_PREFIX "WLI"
#include "debug.h"

#include "constants.h"
#include "traverse.h"
#include "pattern_match.h"
#include "SSAWithloopFolding.h"
#include "SSAWLI.h"
#include "pattern_match.h"
#include "pattern_match_build_lut.h"
#include "LookUpTable.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    info *next;
    node *wl;
    node *assign;
    node *fundef;
    unsigned int foldable :1;
    bool detfoldable;
    lut_t *pmlut;
    bool localfun;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_NEXT(n) ((n)->next)
#define INFO_WL(n) ((n)->wl)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_FOLDABLE(n) ((n)->foldable)
#define INFO_DETFOLDABLE(n) ((n)->detfoldable)
#define INFO_PMLUT(n) ((n)->pmlut)
#define INFO_LOCALFUN(n) ((n)->localfun)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_NEXT (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_FOLDABLE (result) = 0;
    INFO_DETFOLDABLE (result) = FALSE;
    INFO_PMLUT (result) = NULL;
    INFO_LOCALFUN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
 *   transformation. This is assumed to be true here, and not checked again.
 *
 ******************************************************************************/
static node *
CheckArrayFoldable (node *indexn, node *idn, info *arg_info)
{
    node *substn = NULL, *thisn;

    DBUG_ENTER ();
    DBUG_ASSERT (N_id == NODE_TYPE (indexn), "Wrong nodetype for indexn");
    DBUG_ASSERT (N_id == NODE_TYPE (idn), "Wrong nodetype for idn");

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
                DBUG_PRINT ("CheckArrayFoldable WITH_REFERENCED_FOLD(%s) = %d",
                            AVIS_NAME (ID_AVIS (idn)), WITH_REFERENCED_FOLD (substn));
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
 *   index_info *Scalar2ArrayIndex(node *letn, node *wln, lut_t *pmlut);
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
Scalar2ArrayIndex (node *arrayn, node *wln, lut_t *pmlut)
{
    index_info *iinfo, *tmpii;
    int elts = 1, ok = 1, i, *valid_permutation;
    node *idn;
    pattern *pat1;
    int cval;

    DBUG_ENTER ();
    DBUG_ASSERT (N_array == NODE_TYPE (arrayn), "wrong nodetype (array)");

    pat1 = PMint (1, PMAgetIVal (&cval));

    /*
     * this needs to be a vector of scalar elements
     */
    if (TUisScalar (ARRAY_ELEMTYPE (arrayn))
        && (SHgetDim (ARRAY_FRAMESHAPE (arrayn)) == 1)) {
        elts = SHgetExtent (ARRAY_FRAMESHAPE (arrayn), 0);
        arrayn = ARRAY_AELEMS (arrayn);

        iinfo = WLFcreateIndex (elts);
        valid_permutation = (int *)MEMmalloc (sizeof (int) * elts);
        for (i = 0; i < elts;) {
            valid_permutation[i++] = 0;
        }

        for (i = 0; i < elts && ok; i++) {
            /* check each element. */
            ok = 0;
            iinfo->last[i] = NULL;
            idn = EXPRS_EXPR (arrayn);
            if (PMmatch (PMMflatPrfLut (PMMisInGuards, pmlut), pat1,
                         idn)) { /* this is a constant */
                iinfo->permutation[i] = 0;
                iinfo->const_arg[i] = cval;
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

    pat1 = PMfree (pat1);

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

    DBUG_ENTER ();

    assignn = INFO_ASSIGN (arg_info);
    wln = INFO_WL (arg_info);

    DBUG_ASSERT (!ASSIGN_INDEX (assignn), "index_info already assigned");

    if (TUshapeKnown (ID_NTYPE (idn))) {
        /* index var? */
        index_var = WLFlocateIndexVar (idn, wln);
        if (index_var != 0) {
            DBUG_PRINT ("valid index var found");
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
                DBUG_PRINT ("valid local id found");
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
    int index_var = 0;
    index_info *iinfo;
    node *idn = NULL, *assignn, *wln;
    int cval;
    pattern *pat1, *pat2;
    bool const_second;

    DBUG_ENTER ();

    pat1 = PMprf (0, 2, PMvar (1, PMAgetNode (&idn), 0), PMint (1, PMAgetIVal (&cval)));
    pat2 = PMprf (0, 2, PMint (1, PMAgetIVal (&cval)), PMvar (1, PMAgetNode (&idn), 0));

    assignn = INFO_ASSIGN (arg_info);
    wln = INFO_WL (arg_info);

    /* CF has been done, so we just search for an Id and a constant.
       Since we do not want to practice constant folding here we ignore
       prfs with two constants. */
    const_second
      = PMmatch (PMMflatPrfLut (PMMisInGuards, INFO_PMLUT (arg_info)), pat1, prfn);

    if (const_second
        || PMmatch (PMMflatPrfLut (PMMisInGuards, INFO_PMLUT (arg_info)), pat2, prfn)) {

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

            iinfo->mprf = SimplifyFun (PRF_PRF (prfn));
            iinfo->const_arg[0] = cval;
            iinfo->arg_no = const_second ? 2 : 1;
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
        const_elems = (int *)COgetDataVec (constn);

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
                iinfo->mprf = SimplifyFun (PRF_PRF (prfn));

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
            iinfo = Scalar2ArrayIndex (idn, wln, INFO_PMLUT (arg_info));
            if (iinfo) {
                /* This is a valid vector. Permutation and last of index_info are
                   already set. But we still need to handle the prf. */
                elts = iinfo->vector;
                ASSIGN_INDEX (assignn) = iinfo;

                iinfo->arg_no = (constn == const1) ? 1 : 2;
                iinfo->mprf = SimplifyFun (PRF_PRF (prfn));

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

    DBUG_RETURN ();
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
    bool old_localfun;
    DBUG_ENTER ();

    INFO_WL (arg_info) = NULL;
    INFO_FUNDEF (arg_info) = arg_node;

    if (!INFO_LOCALFUN (arg_info)) {
        DBUG_ASSERT (INFO_PMLUT (arg_info) == NULL,
                     "left-over pattern matching lut found!");
        INFO_PMLUT (arg_info) = PMBLdoBuildPatternMatchingLut (arg_node, PMMflat ());
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_ASSIGNS (arg_node) = TRAVdo (FUNDEF_ASSIGNS (arg_node), arg_info);
    }

    old_localfun = INFO_LOCALFUN (arg_info);
    INFO_LOCALFUN (arg_info) = TRUE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_LOCALFUN (arg_info) = old_localfun;

    if (!INFO_LOCALFUN (arg_info)) {
        DBUG_ASSERT (INFO_PMLUT (arg_info) != NULL, "pattern matching lut got lost!");
        INFO_PMLUT (arg_info) = LUTremoveLut (INFO_PMLUT (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    INFO_ASSIGN (arg_info) = arg_node;

    /* this is important. Only index transformations
       with a non-null ASSIGN_INDEX are valid. See WLIlet. Before WLI, this
       pointer may be non-null (somewhere wrong initialisation -> better
       use MakeAssign()!!! ) */
#if 0
  /**
   * This led to bug 505 :-(
   * However, as time is precious and it is rather difficult to
   * trace this I have added the hack below. Using our tree-checker
   * we should be able to sort this out. Any volunteers?
   * I will leave the bug marked as open...
   */
  DBUG_ASSERT (ASSIGN_INDEX( arg_node) == NULL, "left-over ASSIGN_INDEX found.");
#else
    if (ASSIGN_INDEX (arg_node) != NULL) {
        ASSIGN_INDEX (arg_node) = FREEfreeIndexInfo (ASSIGN_INDEX (arg_node));
    }
#endif

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
    COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);

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

    DBUG_ENTER ();

    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if ((assignn != NULL) && (NODE_TYPE (ASSIGN_RHS (assignn)) == N_with)) {
        ID_WL (arg_node) = assignn;
        /*
         * arg_node describes a WL, so WITH_REFERENCED has to be incremented
         */
        (WITH_REFERENCED (ASSIGN_RHS (assignn))) += 1;
        DBUG_PRINT ("WLIid WITH_REFERENCED(%s) = %d (line %zu)",
                    AVIS_NAME (ID_AVIS (arg_node)),
                    WITH_REFERENCED (ASSIGN_RHS (assignn)), NODE_LINE (arg_node));
    } else {
        /* id is not defined by a withloop */
        DBUG_PRINT_TAG ("WLIEXT", "WLIid %s is not defined by a WL",
                        AVIS_NAME (ID_AVIS (arg_node)));

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

    DBUG_ENTER ();

    /* traverse sons first so that ID_WL of every Id is defined (or NULL). */
    old_assignn = INFO_ASSIGN (arg_info);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_ASSIGN (arg_info) = old_assignn;

    /* if we are inside a WL we have to search for valid index transformations. */
    if (INFO_WL (arg_info)) {
        /* if this is a prf, we are interested in transformations like +,*,-,/
           and in indexing (F_sel_VxA). */
        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn)) {
            prf = PRF_PRF (exprn);
            switch (prf) {
                /* this may be an assignment which calculates an index for an
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
                if (N_id == NODE_TYPE (PRF_ARG1 (exprn))) {
                    DBUG_PRINT ("checking id %s as WL-index", ID_NAME (PRF_ARG1 (exprn)));
                }
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
              = Scalar2ArrayIndex (exprn, INFO_WL (arg_info), INFO_PMLUT (arg_info));
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
 *   N_parts are traversed (which call the appropriate N_code subtrees).
 *
 ******************************************************************************/
node *
WLIwith (node *arg_node, info *arg_info)
{
    node *tmpn;
    info *tmpi;

    DBUG_ENTER ();

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpi = MakeInfo ();
    INFO_FUNDEF (tmpi) = INFO_FUNDEF (arg_info);
    INFO_ASSIGN (tmpi) = INFO_ASSIGN (arg_info);
    INFO_PMLUT (tmpi) = INFO_PMLUT (arg_info);
    INFO_NEXT (tmpi) = arg_info;
    arg_info = tmpi;

    /* initialize WL traversal */
    INFO_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = WITH_CODE (arg_node);
    while (tmpn != NULL) { /* reset traversal flag for each code */
        CODE_VISITED (tmpn) = FALSE;
        tmpn = CODE_NEXT (tmpn);
    }
    /* Attribut FOLDABLE will be set later. The others are initialized here. */
    WITH_REFERENCED (arg_node) = 0;
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCES_FOLDED (arg_node) = 0;

    /* initialize determination of FOLDABLE */
    INFO_FOLDABLE (arg_info) = TRUE;

    /* determine FOLDABLE */
    DBUG_PRINT ("WLIwith determining foldability of  WL in line %zu",
                NODE_LINE (arg_node));
    INFO_DETFOLDABLE (arg_info) = TRUE;
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_ISFOLDABLE (INFO_WL (arg_info)) = INFO_FOLDABLE (arg_info);
    DBUG_PRINT ("WITH_ISFOLDABLE set to %s",
                (INFO_FOLDABLE (arg_info) ? "true" : "false"));
    INFO_DETFOLDABLE (arg_info) = FALSE;

    /* traverse all parts and, implicitly, bodies) */
    DBUG_PRINT ("WLIwith searching code of  WL in line %zu", NODE_LINE (arg_node));
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    DBUG_PRINT ("WLIwith searching done");

    /* traverse N_withop */
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
 *   if op is modarray, we have to check if the base array Id can be
 *   eliminated by folding. If so, then
 *   WITH_REFERENCED_FOLD is incremented.
 *
 ******************************************************************************/
node *
WLImodarray (node *arg_node, info *arg_info)
{
    node *substn;

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (WITH_ISFOLDABLE (INFO_WL (arg_info))) {
        substn = ID_WL (MODARRAY_ARRAY (arg_node));
        /*
         * we just traversed through the sons so WLIid for WITHOP_ARRAY
         * has been called and its result is stored in ID_WL().
         */
        if (substn != NULL) {
            (WITH_REFERENCED_FOLD (ASSIGN_RHS (substn)))++;
            DBUG_PRINT ("WLImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                        AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (arg_node))),
                        WITH_REFERENCED_FOLD (ASSIGN_RHS (substn)));
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
    DBUG_ENTER ();

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

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
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
 *   If being called from SWLF, we allow non-constant bounds, step, width.
 *
 ******************************************************************************/
node *
WLIgenerator (node *arg_node, info *arg_info)
{
    static pattern *pat = NULL;
    lut_t *pmlut = INFO_PMLUT (arg_info);

    DBUG_ENTER ();

    DBUG_ASSERT (pmlut != NULL,
                 "pattern matching lut has not made it to the matching site");

    if (pat == NULL) {
        pat = PMconst (0);
    }
    INFO_FOLDABLE (arg_info)
      = INFO_FOLDABLE (arg_info)
        && ((global.compiler_subphase != PH_opt_cyc) || /* SWLF */
            PMmatch (PMMflatLut (pmlut), pat, GENERATOR_BOUND1 (arg_node)));
    INFO_FOLDABLE (arg_info)
      = INFO_FOLDABLE (arg_info)
        && ((global.compiler_subphase != PH_opt_cyc) || /* SWLF */
            PMmatch (PMMflatLut (pmlut), pat, GENERATOR_BOUND2 (arg_node)));

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_FOLDABLE (arg_info)
          = INFO_FOLDABLE (arg_info)
            && ((global.compiler_subphase != PH_opt_cyc) || /* SWLF */
                PMmatch (PMMflatLut (pmlut), pat, GENERATOR_STEP (arg_node)));
        if (GENERATOR_WIDTH (arg_node) != NULL) {
            INFO_FOLDABLE (arg_info)
              = INFO_FOLDABLE (arg_info)
                && ((global.compiler_subphase != PH_opt_cyc) || /* SWLF */
                    PMmatch (PMMflatLut (pmlut), pat, GENERATOR_WIDTH (arg_node)));
        }
    } else {
        DBUG_ASSERT (GENERATOR_WIDTH (arg_node) == NULL,
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
    DBUG_ENTER ();

    DBUG_ASSERT (!CODE_VISITED (arg_node), "Body traversed a second time in WLI");

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
 * @brief starts WLI traversal
 *
 * @param arg_node node to process
 *
 *****************************************************************************/
node *
WLIdoWLI (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "WLI called on nonN_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

    DBUG_PRINT_TAG ("OPTMEM", "mem currently allocated: %zu bytes",
                    global.current_allocated_mem);

    TRAVpush (TR_wli);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
