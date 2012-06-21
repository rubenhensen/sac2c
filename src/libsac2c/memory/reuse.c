/*
 * $Id$
 */

/**
 * @defgroup emri Reuse Inference
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
};

/**
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_RHSCAND(n) ((n)->rhscand)
#define INFO_ALLOCATOR(n) ((n)->allocator)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_RHSCAND (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_TRAVMODE (result) = ri_default;
    INFO_ALLOCATOR (result) = F_unknown;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn TypeMatch
 *
 *  @brief
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

/** <!--********************************************************************-->
 *
 * @fn EMRIdoReuseInference
 *
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
                 "ReuseInference not started with modul node");

    arg_info = MakeInfo ();

    TRAVpush (TR_emri);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMRIassign
 *
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
    if (ASSIGN_STMT (arg_node) != NULL) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    /*
     * Top-down traversal!
     */
    if (INFO_TRAVMODE (arg_info) != ri_annotate) {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMRIlet
 *
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

/** <!--********************************************************************-->
 *
 * @fn EMRIprf
 *
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

        if (rhc != NULL) {
            DBUG_PRINT ("rhc");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, rhc));
        }

        INFO_RHSCAND (arg_info) = rhc;

        if (INFO_RHSCAND (arg_info) != NULL) {
            DBUG_PRINT ("RHSCAND");
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
        DBUG_ASSERT (0, "Illegal prf!");
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
            if ((PRF_PRF (arg_node) == F_alloc)
                || (PRF_PRF (arg_node) == INFO_ALLOCATOR (arg_info))) {
                PRF_PRF (arg_node) = INFO_ALLOCATOR (arg_info);
                PRF_ARGS (arg_node)
                  = TCappendExprs (PRF_ARGS (arg_node), INFO_RHSCAND (arg_info));
                INFO_RHSCAND (arg_info) = NULL;
            } else {
            }
        }
        break;

    case F_fill:
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        if (INFO_RHSCAND (arg_info) != NULL) {
            INFO_TRAVMODE (arg_info) = ri_annotate;
            INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
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

/** <!--********************************************************************-->
 *
 * @fn node *EMRIgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RHSCAND (arg_info) = GENARRAY_RC (arg_node);
    GENARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_TRAVMODE (arg_info) = ri_annotate;
        INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
        AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
        INFO_ALLOCATOR (arg_info) = F_unknown;
    } else {
        INFO_RHSCAND (arg_info) = GENARRAY_PRC (arg_node);
        GENARRAY_PRC (arg_node) = NULL;
        INFO_TRAVMODE (arg_info) = ri_annotate;
        INFO_ALLOCATOR (arg_info) = F_alloc_or_resize;
        AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
        INFO_ALLOCATOR (arg_info) = F_unknown;
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRImodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RHSCAND (arg_info) = MODARRAY_RC (arg_node);
    MODARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_TRAVMODE (arg_info) = ri_annotate;
        INFO_ALLOCATOR (arg_info) = F_alloc_or_reuse;
        AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
        INFO_ALLOCATOR (arg_info) = F_unknown;
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/

#undef DBUG_PREFIX
