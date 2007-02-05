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
#include "dbug.h"
#include "print.h"
#include "str.h"
#include "memory.h"

#include <string.h>

/**
 * traversal modes
 */
typedef enum { ri_default, ri_annotate } ri_mode;

/**
 * INFO structure
 */
struct INFO {
    node *rhscand;
    node *lhs;
    ri_mode travmode;
};

/**
 * INFO macros
 */
#define INFO_LHS(n) (n->lhs)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_RHSCAND(n) (n->rhscand)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_RHSCAND (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_TRAVMODE (result) = ri_default;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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

    DBUG_ENTER ("TypeMatch");

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

    DBUG_ENTER ("EMRIdoReuseInference");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
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
    DBUG_ENTER ("EMRIassign");

    /*
     * Traverse instruction
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
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
    DBUG_ENTER ("EMRIlet");

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

    DBUG_ENTER ("EMRIprf");

    switch (PRF_PRF (arg_node)) {
    case F_prop_obj_in:
    case F_prop_obj_out:
    case F_accu:
    case F_wl_assign:
    case F_free:
    case F_to_unq:
    case F_from_unq:
    case F_inc_rc:
    case F_dec_rc:
    case F_type_conv:
    case F_type_error:
    case F_dispatch_error:
        break;

    case F_reuse:
    case F_suballoc:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
        }
        break;

    case F_reshape:
        DBUG_ASSERT ((0), "Illegal prf!");
        break;

    case F_alloc_or_reshape:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            INFO_RHSCAND (arg_info) = FREEdoFreeTree (INFO_RHSCAND (arg_info));
        }
        break;

    case F_alloc:
    case F_alloc_or_reuse:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            PRF_PRF (arg_node) = F_alloc_or_reuse;
            PRF_ARGS (arg_node)
              = TCappendExprs (PRF_ARGS (arg_node), INFO_RHSCAND (arg_info));
            INFO_RHSCAND (arg_info) = NULL;
        }
        break;

    case F_fill:
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        if (INFO_RHSCAND (arg_info) != NULL) {
            INFO_TRAVMODE (arg_info) = ri_annotate;
            AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))
              = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))), arg_info);
            INFO_TRAVMODE (arg_info) = ri_default;
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

            DBUG_PRINT ("RI", ("RHSCAND"));
            DBUG_EXECUTE ("RI", PRTdoPrint (INFO_RHSCAND (arg_info)););
        }

        rt = TYfreeType (rt);
        lt = TYfreeType (lt);
        break;

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            DBUG_PRINT ("RI", ("prf args"));
            DBUG_EXECUTE ("RI", PRTdoPrint (PRF_ARGS (arg_node)););
        }

        rhc = TypeMatch (DUPdoDupTree (PRF_ARGS (arg_node)), INFO_LHS (arg_info));

        if (rhc != NULL) {
            DBUG_PRINT ("RI", ("rhc"));
            DBUG_EXECUTE ("RI", PRTdoPrint (rhc););
        }

        INFO_RHSCAND (arg_info) = rhc;

        if (INFO_RHSCAND (arg_info) != NULL) {
            DBUG_PRINT ("RI", ("RHSCAND"));
            DBUG_EXECUTE ("RI", PRTdoPrint (INFO_RHSCAND (arg_info)););
        }
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
    DBUG_ENTER ("EMRIgenarray");

    INFO_RHSCAND (arg_info) = GENARRAY_RC (arg_node);
    GENARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_TRAVMODE (arg_info) = ri_annotate;
        AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (GENARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
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
    DBUG_ENTER ("EMRImodarray");

    INFO_RHSCAND (arg_info) = MODARRAY_RC (arg_node);
    MODARRAY_RC (arg_node) = NULL;

    if (INFO_RHSCAND (arg_info) != NULL) {
        INFO_TRAVMODE (arg_info) = ri_annotate;
        AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
          = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);
        INFO_TRAVMODE (arg_info) = ri_default;
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
