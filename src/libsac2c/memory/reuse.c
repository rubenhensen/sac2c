/**
 * @defgroup emri Reuse Inference
 *
 * This phase identifies reuse candidates for primitive functions and
 * it replaces the corresponding _alloc_ calls for the results by
 * either _alloc_or_reuse_ or _alloc_or_resize_. In doing so,
 * it also expands the arguments from the previous _alloc_ by adding
 * the reuse candidates in the replacement for _alloc_.
 *
 * For example:
 *
  ...
  _emal_4840_a = _alloc_( 1, [ 14 ]);
  a = _fill_( _add_VxV_( _flat_16, _flat_0), _emal_4840_a);
  ...
 * is replaced by
 *
  ...
  _emal_4840_a = _alloc_or_reuse_( 1, [ 14 ], _flat_16, _flat_0);
  a = _fill_( _add_VxV_( _flat_16, _flat_0), _emal_4840_a);
  ...
 *
 * Note here, that this is done without checking whether the arguments of
 * the primitive function under consideration are being referenced later!
 *
 * This phase applies the same transformation to the _alloc_ operations for
 * the results of with-loops. In those cases, the reuse candidates are taken
 * from GENARRAY_RC, GENARRAY_PRC and MODARRAY_RC which are inferred by wrci,
 * run within the optimisations just before ive.
 *
 * Again, it should be noted that this replacement is being done without
 * analysing whether there are later references to the reuse candidates
 * which render them unapplicable.
 * FRC elides those.
 *
 *******************************************************************************
 *
 * Implementation notes:
 *
 *  This traversal has two modes. The mode ri_default traverses through the
 *  program to identify suitable prfs or with-loops. Once found, INFO_RHSCAND
 *  is loaded with the exprs list of candidate ids and the _alloc_ assignment
 *  is being traversed in ri_annotate mode.
 *  This switch is being triggered
 *    for prfs: when _fill_( <a suitable prf>, mem) is found
 *    for WLFs: when non-empty WITHOP_RC/PRC are being found
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file reuse inference
 *
 * Prefix: EMRI
 */
#include "reuse.h"

#include "types.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"

#define DBUG_PREFIX "RI"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"

/**
 * traversal modes
 */
typedef enum {
    ri_default,
    ri_annotate,
} ri_mode;

/**
 * INFO structure
 */
struct INFO {
    node *rhscand;
    node *lhs;
    ri_mode travmode;
    prf allocator;
    /* extended memory reuse */
    node *used_rcs;
    node *tmp_rcs;
    node *loop_fundef;
};

/**
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_RHSCAND(n) ((n)->rhscand)
#define INFO_ALLOCATOR(n) ((n)->allocator)
#define INFO_USED_RCS(n) ((n)->used_rcs)
#define INFO_TMP_RCS(n) ((n)->tmp_rcs)
#define INFO_LOOP_FUNDEF(n) ((n)->loop_fundef)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_RHSCAND (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_TRAVMODE (result) = ri_default;
    INFO_ALLOCATOR (result) = F_unknown;
    INFO_USED_RCS (result) = NULL;
    INFO_TMP_RCS (result) = NULL;
    INFO_LOOP_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief
 *
 * @param a
 * @param b
 *
 * @return true or false
 */
static bool
doIdsMatch (node * exprs, node * id)
{
    if (exprs == NULL) {
        return FALSE;
    } else {
        if (ID_AVIS (id) == ID_AVIS (EXPRS_EXPR (exprs))) {
            return TRUE;
        } else {
            return doIdsMatch (EXPRS_NEXT (exprs), id);
        }
    }
}

/**
 * @brief
 *
 * @param t1
 * @param t2
 *
 * @return
 */
static bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ();

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
}

/**
 * @brief
 *
 * @param avis
 * @param exprs
 *
 * @return
 */
static node *
isSameShapeAvis (node * avis, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    if (exprs == NULL) {
        ret = NULL;
    } else {
        if (ShapeMatch (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs))))
            ret = EXPRS_EXPR (exprs);
        else
            ret = isSameShapeAvis (avis, EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (ret);
}

/**
 * @brief
 *
 * @param id
 * @param exprs
 *
 * @return
 */
static node *
isSameShape (node * id, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    ret = isSameShapeAvis (ID_AVIS (id), exprs);

    DBUG_RETURN (ret);
}


/**
 * @brief
 *
 * @param a
 * @param col
 *
 * @return node chain after filtering
 */
static node *
filterDuplicateIds (node * a, node ** col)
{
    node * filtered;
    DBUG_ENTER ();

    DBUG_PRINT ("filtering out duplicate IDs");

    filtered = TCfilterExprsArg (doIdsMatch, a, col);

    /* we delete all duplicates from col */
    if (filtered != NULL) {
        DBUG_PRINT ("  found and removed the following duplicates:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, filtered));
        filtered = FREEdoFreeTree (filtered);
    }

    DBUG_RETURN (*col);
}

/**
 * @brief
 *
 * @param id
 *
 * @return true or false
 */
static bool
avisExist (node * id)
{
    bool ret = FALSE;
    if (ID_AVIS (id) == NULL || NODE_TYPE (ID_AVIS (id)) != N_avis)
        ret = TRUE;
    return ret;
}

/**
 * @brief
 *
 * @param exprs
 *
 * @return node chain after filtering
 */
static node *
filterInvalidAvis (node ** exprs)
{
    node * filtered;
    DBUG_ENTER ();

    DBUG_PRINT ("filtering out invalid N_avis");

    filtered = TCfilterExprs (avisExist, exprs);

    /* we delete all invalid N_avis from exprs */
    if (filtered != NULL) {
        int size = TCcountExprs (filtered);
        DBUG_PRINT ("  found %d invalid N_avis, removing", size);
        filtered = FREEdoFreeTree (filtered);
    }

    DBUG_RETURN (*exprs);
}

/**
 * @brief
 *
 * @param exprs  exprs chain of N_id nodes
 * @param pot
 *
 * @return
 */
static node *
EMRIfindAndAppendArgs (node * exprs, node * pot)
{
    int size, i;
    node * res = NULL,
         * tmp,
         * find;
    DBUG_ENTER ();

    DBUG_PRINT ("finding suitable args inplace of tmp vars");

    if (pot != NULL) {
        /* for each var in tmp RC, find a suitable var in ERC and replace it */
        size = TCcountExprs (exprs);
        for (i = 0; i < size; i++)
        {
            tmp = TCgetNthExprsExpr (i, exprs);
            find = isSameShape (tmp, pot);
            if (find == NULL)
            {
                DBUG_UNREACHABLE ("  unable to find a valid extended reuse candidate to replace tmp arg in recurisve loopfun!");
            } else {
                DBUG_PRINT ("  found match for tmp arg %s => %s", ID_NAME (tmp), ID_NAME (find));

                /* adding res to args of ap_fundef */
                res = TBmakeExprs (find, res);
            }
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 */
static node *
EMREap (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        DBUG_PRINT ("inspecting application of %s ...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        int ap_arg_len = TCcountExprs (AP_ARGS (arg_node));
        int fun_arg_len = TCcountArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)));

        if (ap_arg_len != fun_arg_len) {
            DBUG_PRINT ("  number args for ap do not match fundef: %d != %d", ap_arg_len, fun_arg_len);

            /* we *always* append new args on fundef */
            for (; ap_arg_len < fun_arg_len; ap_arg_len++)
            {
                node * tmp = TCgetNthArg (ap_arg_len, FUNDEF_ARGS (AP_FUNDEF (arg_node)));
                node * res = isSameShapeAvis (ARG_AVIS (tmp), AP_ARGS (arg_node));
                if (res != NULL) {
                    DBUG_PRINT ("  appending %s", ID_NAME (res));
                    AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), TBmakeExprs (TBmakeId (ID_AVIS (res)), NULL));
                } else {
                    DBUG_UNREACHABLE ("  unable to find arg!");
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}


/**
 *  @brief expects and exprs-chain (cand) and an ids-node (lhs) and deletes
 *         all those expressions in 'cand' whose type whose type is not
 *         identical to the type of 'lhs' or whose type is not at least AKS.
 *
 *  @param cand
 *  @param lhs
 *
 *  @param arg_info
 *
 *****************************************************************************/
static node *
TypeMatch (node *cand, node *lhs)
{
    ntype *lhs_aks;
    ntype *cand_aks;

    DBUG_ENTER ();

    if (cand != NULL) {
        if (EXPRS_NEXT (cand) != NULL) {
            EXPRS_NEXT (cand) = TypeMatch (EXPRS_NEXT (cand), lhs);
        }

        if (NODE_TYPE (EXPRS_EXPR (cand)) == N_id) {
            lhs_aks = TYeliminateAKV (AVIS_TYPE (IDS_AVIS (lhs)));
            cand_aks = TYeliminateAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (cand))));

            if ((!TYisAKS (lhs_aks)) || (!TYeqTypes (lhs_aks, cand_aks))) {
                cand = FREEdoFreeNode (cand);
            }

            lhs_aks = TYfreeType (lhs_aks);
            cand_aks = TYfreeType (cand_aks);
        } else {
            cand = FREEdoFreeNode (cand);
        }
    }
    DBUG_RETURN (cand);
}

/**
 *   @brief
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 *
 *****************************************************************************/
node *
EMRIdoReuseInference (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "ReuseInference not started with module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_emri);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    /* In order to fully do EMR, we need to update the args
     * of the initial loop application to match that of the
     * fundef --- we do not use any ECRs for thus but replicate
     * the args as appropriate.
     */
    if (global.optimize.doemr) {
        anontrav_t emretrav[2] = {{N_ap, &EMREap}, {(nodetype)0, NULL}};

        DBUG_PRINT ("starting anon trav to update application args");

        TRAVpushAnonymous (emretrav, &TRAVsons);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse instruction
     */
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    /*
     * Top-down traversal!
     */
    if (INFO_TRAVMODE (arg_info) != ri_annotate) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIprf (node *arg_node, info *arg_info)
{
    ntype *rt, *lt;
    node *rhc;

    DBUG_ENTER ();

    /*
     * SBS Aug 2013: open question: should these be all normal prfs? are they?
     */
    switch (PRF_PRF (arg_node)) {
    case F_tobool_S:
    case F_toc_S:
    case F_tob_S:
    case F_tos_S:
    case F_toi_S:
    case F_tol_S:
    case F_toll_S:
    case F_toub_S:
    case F_tous_S:
    case F_toui_S:
    case F_toul_S:
    case F_toull_S:
    case F_tof_S:
    case F_tod_S:
    case F_neg_S:
    case F_neg_V:
    case F_abs_S:
    case F_abs_V:
    case F_not_S:
    case F_not_V:
    case F_and_SxS:
    case F_and_SxV:
    case F_and_VxS:
    case F_and_VxV:
    case F_or_SxS:
    case F_or_SxV:
    case F_or_VxS:
    case F_or_VxV:
    case F_eq_SxS:
    case F_eq_SxV:
    case F_eq_VxS:
    case F_eq_VxV:
    case F_neq_SxS:
    case F_neq_SxV:
    case F_neq_VxS:
    case F_neq_VxV:
    case F_le_SxS:
    case F_le_SxV:
    case F_le_VxS:
    case F_le_VxV:
    case F_lt_SxS:
    case F_lt_SxV:
    case F_lt_VxS:
    case F_lt_VxV:
    case F_ge_SxS:
    case F_ge_SxV:
    case F_ge_VxS:
    case F_ge_VxV:
    case F_gt_SxS:
    case F_gt_SxV:
    case F_gt_VxS:
    case F_gt_VxV:
    case F_min_SxS:
    case F_min_SxV:
    case F_min_VxS:
    case F_min_VxV:
    case F_max_SxS:
    case F_max_SxV:
    case F_max_VxS:
    case F_max_VxV:
    case F_mod_SxS:
    case F_mod_SxV:
    case F_mod_VxS:
    case F_mod_VxV:
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:

    /* SIMD operations.  */
    case F_add_SMxSM:
    case F_sub_SMxSM:
    case F_mul_SMxSM:
    case F_div_SMxSM:

    case F_sub_SxS:
    case F_sub_SxV:
    case F_sub_VxS:
    case F_sub_VxV:
    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
    case F_div_SxS:
    case F_div_SxV:
    case F_div_VxS:
    case F_div_VxV:
    case F_dim_A:
    case F_shape_A:
    case F_idxs2offset:
        if (PRF_ARGS (arg_node) != NULL) {
            DBUG_PRINT ("prf args");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, PRF_ARGS (arg_node)));
        }

        rhc = TypeMatch (DUPdoDupTree (PRF_ARGS (arg_node)), INFO_LHS (arg_info));

        INFO_RHSCAND (arg_info) = rhc;

        if (INFO_RHSCAND (arg_info) != NULL) {
            DBUG_PRINT ("RHSCAND:");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
        }
        break;

    case F_reuse:
    case F_suballoc:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
        }
        break;

    case F_reshape_VxA:
        DBUG_UNREACHABLE ("Illegal prf!");
        break;

    case F_alloc_or_reshape:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
        }
        break;

    case F_alloc:
    case F_alloc_or_reuse:
    case F_alloc_or_resize:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            /*
             * we can only extend reuses or resizes but not have both at
             * the same time right now.
             */
            if (PRF_PRF (arg_node) == F_alloc) {
                DBUG_PRINT ("replacing _alloc_ by %s",
                            global.prf_name[INFO_ALLOCATOR (arg_info)]);
                PRF_PRF (arg_node) = INFO_ALLOCATOR (arg_info);
            }
            if (PRF_PRF (arg_node) == INFO_ALLOCATOR (arg_info)) {
                PRF_ARGS (arg_node)
                  = TCappendExprs (PRF_ARGS (arg_node), INFO_RHSCAND (arg_info));
                INFO_RHSCAND (arg_info) = NULL;
                DBUG_PRINT ("adding RHSCAND to reuse candidate list in %s",
                            global.prf_name[INFO_ALLOCATOR (arg_info)]);
            } else {
                INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
            }
        }
        break;

    case F_fill:
        DBUG_PRINT ("cheking fill of \"%s\"...", IDS_NAME (INFO_LHS (arg_info)));
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        if (INFO_RHSCAND (arg_info) != NULL) {
            INFO_TRAVMODE (arg_info) = ri_annotate;
            INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
            DBUG_PRINT ("candidate(s) found, annotating memory allocation of \"%s\"...",
                        IDS_NAME (LET_IDS (
                          ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))))));
            AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))
              = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))), arg_info);
            INFO_TRAVMODE (arg_info) = ri_default;
            INFO_ALLOCATOR (arg_info) = F_unknown;
        }
        break;

    case F_copy:
        /*
         * The copied array can be a reuse candidate without being AKS
         */
        rt = TYeliminateAKV (ID_NTYPE (PRF_ARG1 (arg_node)));
        lt = TYeliminateAKV (IDS_NTYPE (INFO_LHS (arg_info)));

        if (TYeqTypes (lt, rt)) {
            INFO_RHSCAND (arg_info) = DUPdoDupTree (PRF_ARGS (arg_node));

            DBUG_PRINT ("RHSCAND");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
        }

        rt = TYfreeType (rt);
        lt = TYfreeType (lt);
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 */
node *
EMRIgenarray (node *arg_node, info *arg_info)
{
    bool using_tmp_rc = FALSE;
    node * new_avis;
    DBUG_ENTER ();

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
    }
    DBUG_PRINT ("handling WL-genarray; resetting RHSCAND");

    INFO_RHSCAND (arg_info) = GENARRAY_RC (arg_node);
    GENARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_USED_RCS (arg_info) = TCappendExprs (INFO_USED_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));
        INFO_TRAVMODE (arg_info) = ri_annotate;
        INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
        DBUG_PRINT ("candidate(s) found, annotating memory allocation of \"%s\"...",
                    IDS_NAME (LET_IDS (
                      ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))))));
        DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
        AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
        INFO_ALLOCATOR (arg_info) = F_unknown;
    } else {
        if (global.optimize.doemr) {
            DBUG_PRINT ("no candidates found; resetting RHSCAND to extended *or* partial candidates");
            // we need to filter the chain to make sure we don't select an ERC that was already used as an RC
            INFO_RHSCAND (arg_info) = filterDuplicateIds (INFO_USED_RCS (arg_info), &GENARRAY_ERC (arg_node));
            GENARRAY_ERC (arg_node) = NULL;
            if (INFO_RHSCAND (arg_info) != NULL) {
                // we only want to pick the first one, so we free all the rest
                if (EXPRS_NEXT (INFO_RHSCAND (arg_info)) != NULL) {
                    EXPRS_NEXT (INFO_RHSCAND (arg_info)) = FREEdoFreeTree (EXPRS_NEXT (INFO_RHSCAND (arg_info)));
                }
                INFO_USED_RCS (arg_info) = TCappendExprs (INFO_USED_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));
                INFO_TRAVMODE (arg_info) = ri_annotate;
                INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
                DBUG_PRINT ("extended candidate(s) found, annotating memory allocation of \"%s\"...",
                            IDS_NAME (LET_IDS (
                              ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))))));
                DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
                AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
                  = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
                INFO_TRAVMODE (arg_info) = ri_default;
                INFO_ALLOCATOR (arg_info) = F_unknown;
            } else { // no usable ERC found, creating tmp_var from LOOPFUN ERCs
                if (INFO_LOOP_FUNDEF (arg_info) != NULL
                  && TYisAKS (ID_NTYPE (GENARRAY_MEM (arg_node)))) {
                    DBUG_PRINT ("no extended candidate was found, generating tmp one!");

                    // we need this to stop from going using PRCs
                    using_tmp_rc = TRUE;

                    // the new avis must have the same type/shape as genarray shape
                    new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                            TYcopyType (ID_NTYPE (GENARRAY_MEM (arg_node)))
                            );

                    // extend the fundef arguments to include the new var
                    FUNDEF_ARGS (INFO_LOOP_FUNDEF (arg_info)) = TCappendArgs (FUNDEF_ARGS (INFO_LOOP_FUNDEF (arg_info)), TBmakeArg (new_avis, NULL));

                    // add the new var to RC
                    INFO_RHSCAND (arg_info) = TBmakeExprs( TBmakeId (new_avis), NULL);
                    INFO_TMP_RCS (arg_info) = TCappendExprs (INFO_TMP_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));

                    DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
                    INFO_TRAVMODE (arg_info) = ri_annotate;
                    INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
                    DBUG_PRINT ("created new candidate, annotating memory allocation of \"%s\"...",
                                IDS_NAME (LET_IDS (
                                  ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))))));
                    AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
                      = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
                    INFO_TRAVMODE (arg_info) = ri_default;
                    INFO_ALLOCATOR (arg_info) = F_unknown;
                }
            }
        }

        if (INFO_RHSCAND (arg_info) == NULL && !using_tmp_rc) { // try to use partials
            DBUG_PRINT ("no candidates found; resetting RHSCAND to partial candidates");
            INFO_RHSCAND (arg_info) = GENARRAY_PRC (arg_node);
            GENARRAY_PRC (arg_node) = NULL;
            if (INFO_RHSCAND (arg_info) != NULL) {
                INFO_USED_RCS (arg_info) = TCappendExprs (INFO_USED_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));
                INFO_TRAVMODE (arg_info) = ri_annotate;
                INFO_ALLOCATOR (arg_info) = F_alloc_or_resize;
                DBUG_PRINT (
                  "partial candidate(s) found, annotating memory allocation of \"%s\"...",
                  IDS_NAME (LET_IDS (
                    ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))))));
                DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
                AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
                  = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
                INFO_TRAVMODE (arg_info) = ri_default;
                INFO_ALLOCATOR (arg_info) = F_unknown;
            }
        }
    }

    if (GENARRAY_ERC (arg_node) != NULL)
        GENARRAY_ERC (arg_node) = FREEdoFreeTree (GENARRAY_ERC (arg_node));

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 */
node *
EMRImodarray (node *arg_node, info *arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
    }
    DBUG_PRINT ("handling WL-modarray; resetting RHSCAND");

    INFO_RHSCAND (arg_info) = MODARRAY_RC (arg_node);
    MODARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_USED_RCS (arg_info) = TCappendExprs (INFO_USED_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));
        INFO_TRAVMODE (arg_info) = ri_annotate;
        INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
        DBUG_PRINT ("candidate(s) found, annotating memory allocation of \"%s\"...",
                    IDS_NAME (LET_IDS (
                      ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))))));
        DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
        AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
        INFO_ALLOCATOR (arg_info) = F_unknown;
    } else {
        if (global.optimize.doemr) {
            DBUG_PRINT ("no candidates found; resetting RHSCAND to extended candidates");
            // we need to filter the chain to make sure we don't select an ERC that was already used as an RC
            INFO_RHSCAND (arg_info) = filterDuplicateIds (INFO_USED_RCS (arg_info), &MODARRAY_ERC (arg_node));
            MODARRAY_ERC (arg_node) = NULL;
            if (INFO_RHSCAND (arg_info) != NULL) {
                // we only want to pick the first one, so we free all the rest
                if (EXPRS_NEXT (INFO_RHSCAND (arg_info)) != NULL) {
                    EXPRS_NEXT (INFO_RHSCAND (arg_info)) = FREEdoFreeTree (EXPRS_NEXT (INFO_RHSCAND (arg_info)));
                }
                INFO_USED_RCS (arg_info) = TCappendExprs (INFO_USED_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));
                DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
                INFO_TRAVMODE (arg_info) = ri_annotate;
                INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
                DBUG_PRINT ("extended candidate(s) found, annotating memory allocation of \"%s\"...",
                            IDS_NAME (LET_IDS (
                              ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))))));
                AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
                  = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);
                INFO_TRAVMODE (arg_info) = ri_default;
                INFO_ALLOCATOR (arg_info) = F_unknown;
            } else { // no usable ERC found, creating tmp_var from LOOPFUN ERCs
                if (INFO_LOOP_FUNDEF (arg_info) != NULL
                        && TYisAKS (ID_NTYPE (MODARRAY_MEM (arg_node)))) {
                    DBUG_PRINT ("no extended candidate was found, generating tmp one!");

                    // the new avis must have the same type/shape as modarray array
                    new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                            TYcopyType (ID_NTYPE (MODARRAY_MEM (arg_node)))
                            );

                    // extend the fundef arguments to include the new var
                    FUNDEF_ARGS (INFO_LOOP_FUNDEF (arg_info)) = TCappendArgs (FUNDEF_ARGS (INFO_LOOP_FUNDEF (arg_info)), TBmakeArg (new_avis, NULL));

                    // add the new var to RC
                    INFO_RHSCAND (arg_info) = TBmakeExprs( TBmakeId (new_avis), NULL);
                    INFO_TMP_RCS (arg_info) = TCappendExprs (INFO_TMP_RCS (arg_info), DUPdoDupNode (INFO_RHSCAND (arg_info)));

                    DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_RHSCAND (arg_info)));
                    INFO_TRAVMODE (arg_info) = ri_annotate;
                    INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
                    DBUG_PRINT ("created new candidate, annotating memory allocation of \"%s\"...",
                                IDS_NAME (LET_IDS (
                                  ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))))));
                    AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
                      = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);
                    INFO_TRAVMODE (arg_info) = ri_default;
                    INFO_ALLOCATOR (arg_info) = F_unknown;
                }
            }
        }
    }

    if (MODARRAY_ERC (arg_node) != NULL)
        MODARRAY_ERC (arg_node) = FREEdoFreeTree (MODARRAY_ERC (arg_node));

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 */
node *
EMRIap (node * arg_node, info * arg_info)
{
    node * rec_filt, * new_args;
    DBUG_ENTER ();

    DBUG_PRINT ("checking application of %s ...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

    if (global.optimize.doemr) {
        if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
            // we inside the loop
            if (INFO_LOOP_FUNDEF (arg_info) != NULL
                    && AP_FUNDEF (arg_node) == INFO_LOOP_FUNDEF (arg_info)) {
                DBUG_PRINT ("  this is a recursive loop application");

                if (INFO_TMP_RCS (arg_info) != NULL) {
                    DBUG_PRINT ("have found tmp vars at application's fundef");

                    /* remove all N_id that point to N_avis that does not exist anymore */
                    rec_filt = filterInvalidAvis (&FUNDEF_ERC (AP_FUNDEF (arg_node)));

                    /* we filter out current application args */
                    rec_filt = filterDuplicateIds (AP_ARGS (arg_node), &rec_filt);

                    /* for each var in tmp RC, find a suitable var in ERC and replace it */
                    new_args = EMRIfindAndAppendArgs (INFO_TMP_RCS (arg_info), rec_filt);
                    AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), DUPdoDupTree (new_args));

                    DBUG_PRINT ("  args are now:");
                    DBUG_EXECUTE (if (AP_ARGS (arg_node) != NULL) {
                            PRTdoPrintFile (stderr, AP_ARGS (arg_node));});

                    /* clear tmp rcs */
                    INFO_TMP_RCS (arg_info) = FREEdoFreeTree (INFO_TMP_RCS (arg_info));

                    /* clear the fundef ERC */
                    FUNDEF_ERC (AP_FUNDEF (arg_node)) = FREEdoFreeTree (FUNDEF_ERC (AP_FUNDEF (arg_node)));
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 */
node *
EMRIfundef (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    /* Top-down Traversal */
    if (global.optimize.doemr) {
        DBUG_PRINT ("checking function %s ...", FUNDEF_NAME (arg_node));
        if (FUNDEF_ISLOOPFUN (arg_node)) {
            INFO_LOOP_FUNDEF (arg_info) = arg_node;
        }

        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* we only need to carry the loop fundef within its body */
        INFO_LOOP_FUNDEF (arg_info) = NULL;

        /* used RCs are specific to each fundef body */
        if (INFO_USED_RCS (arg_info) != NULL) {
            INFO_USED_RCS (arg_info) = FREEdoFreeTree (INFO_USED_RCS (arg_info));
        }
    } else {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*@}*/

#undef DBUG_PREFIX
