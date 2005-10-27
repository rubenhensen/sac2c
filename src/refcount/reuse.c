/*
 *
 * $Log$
 * Revision 1.19  2005/09/15 17:12:13  ktr
 * remove ICM traversal
 *
 * Revision 1.18  2005/08/19 18:20:45  ktr
 * Added neutral behaviour for F_type_conv, F_type_error, F_dispatch_error
 *
 * Revision 1.17  2005/06/30 11:57:02  ktr
 * made adjustments necessary for shapeclass dependent dopying
 *
 * Revision 1.16  2005/06/29 16:04:55  ktr
 * An array copied by means of F_copy is always a reuse candidate
 *
 * Revision 1.15  2005/01/11 13:35:40  cg
 * Useless include of Error.h removed
 *
 * Revision 1.14  2004/12/11 14:48:06  ktr
 * bugfix.
 *
 * Revision 1.13  2004/12/08 21:22:02  ktr
 * minor bugfix
 *
 * Revision 1.12  2004/11/27 01:35:10  ktr
 * function names fixed.
 *
 * Revision 1.11  2004/11/26 14:12:51  ktr
 * function application changed.
 *
 * Revision 1.10  2004/11/23 22:24:25  ktr
 * some renaming.
 *
 * Revision 1.9  2004/11/23 19:52:59  ktr
 * COMPILES!!!
 *
 * Revision 1.8  2004/11/23 19:28:31  jhb
 * ismop
 *
 * Revision 1.7  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
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
 * @defgroup emri Reuse Inference
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
#include "ReuseWithArrays.h"
#include "DataFlowMask.h"
#include "internal_lib.h"

#include <string.h>

/**
 * traversal modes
 */
typedef enum { ri_default, ri_annotate } ri_mode;

/**
 * INFO structure
 */
struct INFO {
    dfmask_t *candidates;
    node *rhscand;
    node *lhs;
    node *fundef;
    bool addlhs;
    ri_mode travmode;
};

/**
 * INFO macros
 */
#define INFO_CANDIDATES(n) (n->candidates)
#define INFO_LHS(n) (n->lhs)
#define INFO_ADDLHS(n) (n->addlhs)
#define INFO_FUNDEF(n) (n->fundef)
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

    result = ILIBmalloc (sizeof (info));

    INFO_CANDIDATES (result) = NULL;
    INFO_RHSCAND (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_ADDLHS (result) = FALSE;
    INFO_TRAVMODE (result) = ri_default;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
CutExprs (dfmask_t *candidates, node *list)
{
    DBUG_ENTER ("CutExprs");

    if (list != NULL) {
        EXPRS_NEXT (list) = CutExprs (candidates, EXPRS_NEXT (list));

        if (!DFMtestMaskEntry (candidates, NULL, ID_AVIS (EXPRS_EXPR (list)))) {
            list = FREEdoFreeNode (list);
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
 * @fn EMRIarg
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRIarg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    if (!ARG_ISARTIFICIAL (arg_node)) {
        DFMsetMaskEntrySet (INFO_CANDIDATES (arg_info), NULL, ARG_AVIS (arg_node));
    }

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
 * @fn EMRIcode
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIcode (node *arg_node, info *arg_info)
{
    dfmask_t *oldcands;

    DBUG_ENTER ("EMRIcode");

    /*
     * Inside a code block a new candidate list is needed
     */
    oldcands = INFO_CANDIDATES (arg_info);
    INFO_CANDIDATES (arg_info)
      = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    /*
     * Traverse CBLOCK
     */
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /*
     * Erase the code block's candidate list before traversing on
     */
    INFO_CANDIDATES (arg_info) = DFMremoveMask (INFO_CANDIDATES (arg_info));
    INFO_CANDIDATES (arg_info) = oldcands;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMRIcond
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIcond (node *arg_node, info *arg_info)
{
    dfmask_t *oldcands;
    DBUG_ENTER ("EMRIcond");

    /*
     * Rescue reuse candidates
     */
    oldcands = DFMgenMaskCopy (INFO_CANDIDATES (arg_info));
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    /*
     * Restore reuse candidates for traversal of else-Branch
     */
    INFO_CANDIDATES (arg_info) = DFMremoveMask (INFO_CANDIDATES (arg_info));
    INFO_CANDIDATES (arg_info) = oldcands;
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    /*
     * After a conditional, nothing can be reused
     */
    DFMsetMaskClear (INFO_CANDIDATES (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMRIfundef
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        INFO_FUNDEF (arg_info) = arg_node;

        /*
         * Generate DFM-base
         */
        FUNDEF_DFM_BASE (arg_node)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        INFO_CANDIDATES (arg_info) = DFMgenMaskClear (FUNDEF_DFM_BASE (arg_node));

        /*
         * FUNDEF args are the initial reuse candidates
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        /*
         * Traverse body
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Remove reuse candidates mask
         */
        INFO_CANDIDATES (arg_info) = DFMremoveMask (INFO_CANDIDATES (arg_info));

        /*
         * Remove DFM-base
         */
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
    INFO_ADDLHS (arg_info) = TRUE;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if (INFO_ADDLHS (arg_info)) {
        node *_ids = LET_IDS (arg_node);
        while (_ids != NULL) {
            DFMsetMaskEntrySet (INFO_CANDIDATES (arg_info), NULL, IDS_AVIS (_ids));
            _ids = IDS_NEXT (_ids);
        }
    }

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
        INFO_ADDLHS (arg_info) = FALSE;
        break;

    case F_alloc:
    case F_alloc_or_reuse:
        if (INFO_TRAVMODE (arg_info) == ri_annotate) {
            PRF_PRF (arg_node) = F_alloc_or_reuse;
            PRF_ARGS (arg_node)
              = TCappendExprs (PRF_ARGS (arg_node), INFO_RHSCAND (arg_info));
            INFO_RHSCAND (arg_info) = NULL;
        }
        INFO_ADDLHS (arg_info) = FALSE;
        break;

    case F_fill:
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        if (INFO_RHSCAND (arg_info) != NULL) {
            INFO_TRAVMODE (arg_info) = ri_annotate;
            AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))
              = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))), arg_info);
            INFO_TRAVMODE (arg_info) = ri_default;
        }
        INFO_ADDLHS (arg_info) = TRUE;
        break;

    case F_copy:
        /*
         * The copied array can be a reuse candidate without being AKS
         */
        rt = TYeliminateAKV (ID_NTYPE (PRF_ARG1 (arg_node)));
        lt = TYeliminateAKV (IDS_NTYPE (INFO_LHS (arg_info)));

        if (TYeqTypes (lt, rt)) {
            rhc = DUPdoDupTree (PRF_ARGS (arg_node));
            INFO_RHSCAND (arg_info) = CutExprs (INFO_CANDIDATES (arg_info), rhc);

            if (INFO_RHSCAND (arg_info) != NULL) {
                DBUG_PRINT ("RI", ("RHSCAND"));
                DBUG_EXECUTE ("RI", PRTdoPrint (INFO_RHSCAND (arg_info)););
            }
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
            DBUG_EXECUTE ("RI", PRTdoPrint (rhc);
                          DFMprintMask (global.outfile, "%s",
                                        INFO_CANDIDATES (arg_info)););
        }

        INFO_RHSCAND (arg_info) = CutExprs (INFO_CANDIDATES (arg_info), rhc);

        if (INFO_RHSCAND (arg_info) != NULL) {
            DBUG_PRINT ("RI", ("RHSCAND"));
            DBUG_EXECUTE ("RI", PRTdoPrint (INFO_RHSCAND (arg_info)););
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMRIwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *****************************************************************************/
node *
EMRIwith2 (node *arg_node, info *arg_info)
{
    node *wlids;
    node *rhc = NULL;
    node *withop;

    DBUG_ENTER ("EMRIwith2");

    wlids = INFO_LHS (arg_info);
    withop = WITH2_WITHOP (arg_node);

    while (withop != NULL) {
        if ((NODE_TYPE (withop) == N_genarray) || (NODE_TYPE (withop) == N_modarray)) {

            rhc = REUSEdoGetReuseArrays (arg_node, INFO_FUNDEF (arg_info), wlids);

            INFO_RHSCAND (arg_info) = CutExprs (INFO_CANDIDATES (arg_info), rhc);

            if (INFO_RHSCAND (arg_info) != NULL) {
                node *mem;
                mem = (NODE_TYPE (withop) == N_genarray) ? GENARRAY_MEM (withop)
                                                         : MODARRAY_MEM (withop);

                INFO_TRAVMODE (arg_info) = ri_annotate;
                AVIS_SSAASSIGN (ID_AVIS (mem))
                  = TRAVdo (AVIS_SSAASSIGN (ID_AVIS (mem)), arg_info);
                INFO_TRAVMODE (arg_info) = ri_default;
            }
        }
        wlids = IDS_NEXT (wlids);
        withop = WITHOP_NEXT (withop);
    }

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    INFO_ADDLHS (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/*@}*/
