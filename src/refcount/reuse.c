/*
 *
 * $Log$
 * Revision 1.2  2004/08/10 16:14:33  ktr
 * RIicm added.
 *
 * Revision 1.1  2004/08/10 13:30:03  ktr
 * Initial revision
 *
 */

#define NEW_INFO

#include "types.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "dbug.h"
#include "Error.h"
#include "print.h"
#include "ReuseWithArrays.h"
#include "DataFlowMask.h"

/**
 * traversal modes
 */
typedef enum { ri_default, ri_annotate } ri_mode;

/**
 * INFO structure
 */
struct INFO {
    node *candidates;
    ids *lhs;
    node *fundef;
    bool addlhs;
    node *rhscand;
    ri_mode travmode;
};

/**
 * INFO macros
 */
#define INFO_RI_CANDIDATES(n) (n->candidates)
#define INFO_RI_LHS(n) (n->lhs)
#define INFO_RI_ADDLHS(n) (n->addlhs)
#define INFO_RI_FUNDEF(n) (n->fundef)
#define INFO_RI_TRAVMODE(n) (n->travmode)
#define INFO_RI_RHSCAND(n) (n->rhscand)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RI_CANDIDATES (result) = NULL;
    INFO_RI_LHS (result) = NULL;
    INFO_RI_FUNDEF (result) = NULL;
    INFO_RI_ADDLHS (result) = FALSE;
    INFO_RI_TRAVMODE (result) = ri_default;
    INFO_RI_RHSCAND (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn CutExprs
 *
 *  @brief
 *
 *  @param ref
 *  @param list
 *
 *  @param arg_info
 *
 *****************************************************************************/
static node *
CutExprs (node *ref, node *list)
{
    node *tmp;

    DBUG_ENTER ("CutExprs");

    if (list != NULL) {
        EXPRS_NEXT (list) = CutExprs (ref, EXPRS_NEXT (list));

        tmp = ref;
        while (tmp != NULL) {
            if (ID_AVIS (EXPRS_EXPR (tmp)) != ID_AVIS (EXPRS_EXPR (list))) {
                tmp = EXPRS_NEXT (tmp);
            } else {
                break;
            }
        }
        if (tmp == NULL) {
            list = FreeNode (list);
        }
    }

    DBUG_RETURN (list);
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
TypeMatch (node *cand, ids *lhs)
{
    ntype *lhs_aks;
    ntype *cand_aks;

    DBUG_ENTER ("TypeMatch");

    if (cand != NULL) {
        if (EXPRS_NEXT (cand) != NULL) {
            EXPRS_NEXT (cand) = TypeMatch (EXPRS_NEXT (cand), lhs);
        }

        if (NODE_TYPE (EXPRS_EXPR (cand)) == N_id) {
            lhs_aks = TYEliminateAKV (AVIS_TYPE (IDS_AVIS (lhs)));
            cand_aks = TYEliminateAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (cand))));

            if ((!TYIsAKS (lhs_aks)) || (!TYEqTypes (lhs_aks, cand_aks))) {
                cand = FreeNode (cand);
            }

            lhs_aks = TYFreeType (lhs_aks);
            cand_aks = TYFreeType (cand_aks);
        } else {
            cand = FreeNode (cand);
        }
    }
    DBUG_RETURN (cand);
}

/** <!--********************************************************************-->
 *
 * @fn RIarg
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIarg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    if (ARG_STATUS (arg_node) == ST_regular) {
        node *id;

        id = MakeId (ARG_NAME (arg_node), NULL, ST_regular);

        ID_VARDEC (id) = arg_node;
        ID_AVIS (id) = ARG_AVIS (arg_node);

        INFO_RI_CANDIDATES (arg_info) = MakeExprs (id, INFO_RI_CANDIDATES (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIassign
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIassign");

    /*
     * Traverse instruction
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    /*
     * Top-down traversal!
     */
    if (INFO_RI_TRAVMODE (arg_info) != ri_annotate) {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIcode
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIcode (node *arg_node, info *arg_info)
{
    node *candidates;

    DBUG_ENTER ("RIcode");

    /*
     * Inside a code block a new candidate list is needed
     */
    candidates = INFO_RI_CANDIDATES (arg_info);
    INFO_RI_CANDIDATES (arg_info) = NULL;

    /*
     * Traverse CBLOCK
     */
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * Erase the code block's candidate list before traversing on
     */
    if (INFO_RI_CANDIDATES (arg_info) != NULL) {
        INFO_RI_CANDIDATES (arg_info) = FreeTree (INFO_RI_CANDIDATES (arg_info));
    }
    INFO_RI_CANDIDATES (arg_info) = candidates;

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIcond
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIcond (node *arg_node, info *arg_info)
{
    node *candidates;
    DBUG_ENTER ("RIcond");

    /*
     * Rescue reuse candidates
     */
    candidates = DupTree (INFO_RI_CANDIDATES (arg_info));
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    /*
     * Restore reuse candidates for traversal of else-Branch
     */
    if (INFO_RI_CANDIDATES (arg_info) != NULL) {
        INFO_RI_CANDIDATES (arg_info) = FreeTree (INFO_RI_CANDIDATES (arg_info));
    }
    INFO_RI_CANDIDATES (arg_info) = DupTree (candidates);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /*
     * Restore original reuse candidates for further traversal
     */
    if (INFO_RI_CANDIDATES (arg_info) != NULL) {
        INFO_RI_CANDIDATES (arg_info) = FreeTree (INFO_RI_CANDIDATES (arg_info));
    }
    INFO_RI_CANDIDATES (arg_info) = candidates;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIfundef
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIfundef");

    INFO_RI_FUNDEF (arg_info) = arg_node;

    /*
     * Generate DFM-base
     */
    FUNDEF_DFM_BASE (arg_node)
      = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

    /*
     * FUNDEF args are the initial reuse candidates
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * Traverse body
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Free reuse candidates list
     */
    if (INFO_RI_CANDIDATES (arg_info) != NULL) {
        INFO_RI_CANDIDATES (arg_info) = FreeTree (INFO_RI_CANDIDATES (arg_info));
    }

    /*
     * Remove DFM-base
     */
    FUNDEF_DFM_BASE (arg_node) = DFMRemoveMaskBase (FUNDEF_DFM_BASE (arg_node));

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIicm
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIicm (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("RIicm");

    name = ICM_NAME (arg_node);

    if ((strstr (name, "USE_GENVAR_OFFSET") != NULL)
        || (strstr (name, "VECT2OFFSET") != NULL)
        || (strstr (name, "IDXS2OFFSET") != NULL)) {
        INFO_RI_CANDIDATES (arg_info)
          = MakeExprs (DupNode (ICM_ARG1 (arg_node)), INFO_RI_CANDIDATES (arg_info));
    } else {
        DBUG_ASSERT ((0), "Unknown ICM found during EMRI");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIlet
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIlet");

    INFO_RI_LHS (arg_info) = LET_IDS (arg_node);
    INFO_RI_ADDLHS (arg_info) = TRUE;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    if (INFO_RI_ADDLHS (arg_info)) {
        INFO_RI_CANDIDATES (arg_info) = AppendExprs (Ids2Exprs (INFO_RI_LHS (arg_info)),
                                                     INFO_RI_CANDIDATES (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIprf
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIprf (node *arg_node, info *arg_info)
{
    node *rhc;

    DBUG_ENTER ("RIprf");

    switch (PRF_PRF (arg_node)) {
    case F_accu:
        break;
    case F_alloc:
    case F_alloc_or_reuse:
        if (INFO_RI_TRAVMODE (arg_info) == ri_annotate) {
            PRF_PRF (arg_node) = F_alloc_or_reuse;
            PRF_ARGS (arg_node)
              = AppendExprs (PRF_ARGS (arg_node), INFO_RI_RHSCAND (arg_info));
            INFO_RI_RHSCAND (arg_info) = NULL;
        }
        INFO_RI_ADDLHS (arg_info) = FALSE;
        break;

    case F_fill:
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);

        if (INFO_RI_RHSCAND (arg_info) != NULL) {
            INFO_RI_TRAVMODE (arg_info) = ri_annotate;
            AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))
              = Trav (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))), arg_info);
            INFO_RI_TRAVMODE (arg_info) = ri_default;
        }
        INFO_RI_ADDLHS (arg_info) = TRUE;
        break;

    default:
        rhc = TypeMatch (DupTree (PRF_ARGS (arg_node)), INFO_RI_LHS (arg_info));
        INFO_RI_RHSCAND (arg_info) = CutExprs (INFO_RI_CANDIDATES (arg_info), rhc);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RIwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
RIwith2 (node *arg_node, info *arg_info)
{
    ids *wlids;
    node *rhc;
    node *withop;

    DBUG_ENTER ("RIwith2");

    wlids = INFO_RI_LHS (arg_info);
    withop = NWITH2_WITHOP (arg_node);

    while (withop != NULL) {
        if ((NWITHOP_TYPE (withop) == WO_genarray)
            || (NWITHOP_TYPE (withop) == WO_modarray)) {

            rhc = GetReuseCandidates (arg_node, INFO_RI_FUNDEF (arg_info), wlids);
            INFO_RI_RHSCAND (arg_info) = CutExprs (INFO_RI_CANDIDATES (arg_info), rhc);

            if (INFO_RI_RHSCAND (arg_info) != NULL) {
                INFO_RI_TRAVMODE (arg_info) = ri_annotate;
                AVIS_SSAASSIGN (ID_AVIS (NWITHOP_MEM (withop)))
                  = Trav (AVIS_SSAASSIGN (ID_AVIS (NWITHOP_MEM (withop))), arg_info);
                INFO_RI_TRAVMODE (arg_info) = ri_default;
            }
        }
        wlids = IDS_NEXT (wlids);
        withop = NWITHOP_NEXT (withop);
    }

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn ReuseInference
 *
 *   @brief
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 *
 *****************************************************************************/
node *
ReuseInference (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("ReuseInference");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "ReuseInference not started with modul node");

    act_tab = emri_tab;
    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
