/*
 *
 * $Log$
 * Revision 1.6  2004/11/15 12:30:36  ktr
 * Reuse inference now works with dataflowmasks
 *
 * Revision 1.5  2004/08/12 12:10:10  ktr
 * now only args with ARG_STATUS == ST_artificial are not treated as
 * potential reuse candidates.
 *
 * Revision 1.4  2004/08/11 13:13:25  ktr
 * empty fundefs are no longer traversed.
 *
 * Revision 1.3  2004/08/10 16:42:59  ktr
 * All prfs should be handled now.
 *
 * Revision 1.2  2004/08/10 16:14:33  ktr
 * RIicm added.
 *
 * Revision 1.1  2004/08/10 13:30:03  ktr
 * Initial revision
 *
 */
/**
 *
 * @defgroup ri Reuse Inference
 * @ingroup emm
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file reuse inference
 *
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
    DFMmask_t candidates;
    node *rhscand;
    ids *lhs;
    node *fundef;
    bool addlhs;
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
    INFO_RI_RHSCAND (result) = NULL;
    INFO_RI_LHS (result) = NULL;
    INFO_RI_FUNDEF (result) = NULL;
    INFO_RI_ADDLHS (result) = FALSE;
    INFO_RI_TRAVMODE (result) = ri_default;

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
CutExprs (DFMmask_t candidates, node *list)
{
    DBUG_ENTER ("CutExprs");

    if (list != NULL) {
        EXPRS_NEXT (list) = CutExprs (candidates, EXPRS_NEXT (list));

        if (!DFMTestMaskEntry (candidates, NULL, ID_VARDEC (EXPRS_EXPR (list)))) {
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

    if (ARG_STATUS (arg_node) != ST_artificial) {
        DFMSetMaskEntrySet (INFO_RI_CANDIDATES (arg_info), NULL, arg_node);
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
    DFMmask_t oldcands;

    DBUG_ENTER ("RIcode");

    /*
     * Inside a code block a new candidate list is needed
     */
    oldcands = INFO_RI_CANDIDATES (arg_info);
    INFO_RI_CANDIDATES (arg_info)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_RI_FUNDEF (arg_info)));

    /*
     * Traverse CBLOCK
     */
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * Erase the code block's candidate list before traversing on
     */
    INFO_RI_CANDIDATES (arg_info) = DFMRemoveMask (INFO_RI_CANDIDATES (arg_info));
    INFO_RI_CANDIDATES (arg_info) = oldcands;

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
    node *oldcands;
    DBUG_ENTER ("RIcond");

    /*
     * Rescue reuse candidates
     */
    oldcands = DFMGenMaskCopy (INFO_RI_CANDIDATES (arg_info));
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    /*
     * Restore reuse candidates for traversal of else-Branch
     */
    INFO_RI_CANDIDATES (arg_info) = DFMRemoveMask (INFO_RI_CANDIDATES (arg_info));
    INFO_RI_CANDIDATES (arg_info) = oldcands;
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /*
     * After a conditional, nothing can be reused
     */
    DFMSetMaskClear (INFO_RI_CANDIDATES (arg_info));

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

    if (FUNDEF_BODY (arg_node) != NULL) {

        INFO_RI_FUNDEF (arg_info) = arg_node;

        /*
         * Generate DFM-base
         */
        FUNDEF_DFM_BASE (arg_node)
          = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        INFO_RI_CANDIDATES (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));

        /*
         * FUNDEF args are the initial reuse candidates
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        /*
         * Traverse body
         */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Remove reuse candidates mask
         */
        INFO_RI_CANDIDATES (arg_info) = DFMRemoveMask (INFO_RI_CANDIDATES (arg_info));

        /*
         * Remove DFM-base
         */
        FUNDEF_DFM_BASE (arg_node) = DFMRemoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

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
        DFMSetMaskEntrySet (INFO_RI_CANDIDATES (arg_info), NULL,
                            ID_VARDEC (ICM_ARG1 (arg_node)));
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
        ids *_ids = LET_IDS (arg_node);
        while (_ids != NULL) {
            DFMSetMaskEntrySet (INFO_RI_CANDIDATES (arg_info), NULL, IDS_VARDEC (_ids));
            _ids = IDS_NEXT (_ids);
        }
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
    case F_wl_assign:
    case F_free:
    case F_to_unq:
    case F_from_unq:
    case F_inc_rc:
    case F_dec_rc:
        break;

    case F_suballoc:
        if (INFO_RI_RHSCAND (arg_info) != NULL) {
            INFO_RI_RHSCAND (arg_info) = FreeTree (INFO_RI_RHSCAND (arg_info));
        }
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

    case F_reshape:
        /*
         * reshape( shp, A)
         *
         * A is a reuse candidate for the reshape Operation as it has
         * the same number of elements as the resulting array.
         */
        INFO_RI_RHSCAND (arg_info) = MakeExprs (DupNode (PRF_ARG2 (arg_node)), NULL);
        break;

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            DBUG_PRINT ("RI", ("prf args"));
            DBUG_EXECUTE ("RI", Print (PRF_ARGS (arg_node)););
        }

        rhc = TypeMatch (DupTree (PRF_ARGS (arg_node)), INFO_RI_LHS (arg_info));

        if (rhc != NULL) {
            DBUG_PRINT ("RI", ("rhc"));
            DBUG_EXECUTE ("RI", Print (rhc); Print (INFO_RI_CANDIDATES (arg_info)););
        }

        INFO_RI_RHSCAND (arg_info) = CutExprs (INFO_RI_CANDIDATES (arg_info), rhc);

        if (INFO_RI_RHSCAND (arg_info) != NULL) {
            DBUG_PRINT ("RI", ("RHSCAND"));
            DBUG_EXECUTE ("RI", Print (INFO_RI_RHSCAND (arg_info)););
        }
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

    INFO_RI_ADDLHS (arg_info) = TRUE;

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

/*@}*/
