/*
 *
 * $Log$
 * Revision 1.2  2004/10/12 12:18:01  ktr
 * moved declaration of local variables in EMFRCfundef in order to
 * please Stephan's compiler.
 *
 * Revision 1.1  2004/10/12 10:07:52  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup frc Filter reuse candidates
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file filterrc.c
 *
 *
 */
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DataFlowMask.h"

/**
 * INFO structure
 */
struct INFO {
    DFMmask_t usemask;
    DFMmask_t oldmask;
    node *condargs;
};

/**
 * INFO macros
 */
#define INFO_FRC_USEMASK(n) (n->usemask)
#define INFO_FRC_OLDMASK(n) (n->oldmask)
#define INFO_FRC_CONDARGS(n) (n->condargs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_FRC_USEMASK (result) = NULL;
    INFO_FRC_OLDMASK (result) = NULL;
    INFO_FRC_CONDARGS (result) = NULL;

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
 * @fn node *EMFRCFilterReuseCandidates( node *syntax_tree)
 *
 * @brief starting point of filter reuse candidates traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMFRCFilterReuseCandidates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMFRCFilterReuseCandidates");

    DBUG_PRINT ("EMFRC", ("Starting to filter reuse candidates..."));

    info = MakeInfo ();

    act_tab = emfrc_tab;

    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_PRINT ("EMFRC", ("Filtering of reuse candidates complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *FilterTrav( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
FilterTrav (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FilterTrav");

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = FilterTrav (EXPRS_NEXT (arg_node), arg_info);
    }

    if (DFMTestMaskEntry (INFO_FRC_USEMASK (arg_info), NULL,
                          ID_VARDEC (EXPRS_EXPR (arg_node)))) {
        DBUG_PRINT ("EMFRC", ("Invalid reuse candidate removed: %s",
                              ID_NAME (EXPRS_EXPR (arg_node))));
        arg_node = FreeNode (arg_node);
    } else {
        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FilterRCs( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
FilterRCs (node *arg_node, info *arg_info)
{
    node *alloc;

    DBUG_ENTER ("FilterRCs");

    alloc = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node)));

    DBUG_EXECUTE ("EMFRC", Print (alloc););

    DBUG_ASSERT ((NODE_TYPE (alloc) == N_prf)
                   && ((PRF_PRF (alloc) == F_alloc)
                       || (PRF_PRF (alloc) == F_alloc_or_reuse)
                       || (PRF_PRF (alloc) == F_suballoc)),
                 "Illegal node type!");

    if (PRF_PRF (alloc) != F_suballoc) {

        if (PRF_EXPRS3 (alloc) != NULL) {
            PRF_EXPRS3 (alloc) = FilterTrav (PRF_EXPRS3 (alloc), arg_info);
        }

        if (PRF_EXPRS3 (alloc) == NULL) {
            PRF_PRF (alloc) = F_alloc;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Filter reuse candidates traversal (emfrc_tab)
 *
 * prefix: EMFRC
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMFRCap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCap");

    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
        INFO_FRC_CONDARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCarg");

    if (DFMTestMaskEntry (INFO_FRC_OLDMASK (arg_info), NULL,
                          ID_VARDEC (EXPRS_EXPR (INFO_FRC_CONDARGS (arg_info))))) {
        DBUG_PRINT ("EMFRC",
                    ("Variable used in calling context: %s", ARG_NAME (arg_node)));
        DFMSetMaskEntrySet (INFO_FRC_USEMASK (arg_info), NULL, arg_node);
    }

    INFO_FRC_CONDARGS (arg_info) = EXPRS_NEXT (INFO_FRC_CONDARGS (arg_info));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCcond (node *arg_node, info *arg_info)
{
    DFMmask_t thenmask, elsemask, usemask;

    DBUG_ENTER ("EMFRCcond");

    usemask = INFO_FRC_USEMASK (arg_info);
    thenmask = DFMGenMaskCopy (usemask);
    elsemask = DFMGenMaskCopy (usemask);

    INFO_FRC_USEMASK (arg_info) = thenmask;
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    INFO_FRC_USEMASK (arg_info) = elsemask;
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    DFMSetMaskOr (usemask, thenmask);
    DFMSetMaskOr (usemask, elsemask);

    thenmask = DFMRemoveMask (thenmask);
    elsemask = DFMRemoveMask (elsemask);

    INFO_FRC_USEMASK (arg_info) = usemask;

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if ((!FUNDEF_IS_CONDFUN (arg_node)) || (INFO_FRC_USEMASK (arg_info) != NULL)) {

            DFMmask_base_t maskbase;
            DFMmask_t oldmask;

            DBUG_PRINT ("EMFRC", ("Filtering reuse candidates in function %s",
                                  FUNDEF_NAME (arg_node)));

            oldmask = INFO_FRC_USEMASK (arg_info);

            maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_FRC_USEMASK (arg_info) = DFMGenMaskClear (maskbase);

            if (oldmask != NULL) {
                INFO_FRC_OLDMASK (arg_info) = oldmask;
                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
                }
                INFO_FRC_OLDMASK (arg_info) = NULL;
            }

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            INFO_FRC_USEMASK (arg_info) = DFMRemoveMask (INFO_FRC_USEMASK (arg_info));

            maskbase = DFMRemoveMaskBase (maskbase);

            INFO_FRC_USEMASK (arg_info) = oldmask;

            DBUG_PRINT ("EMFRC", ("Filtering reuse candidates in function %s complete",
                                  FUNDEF_NAME (arg_node)));
        }
    }

    if ((INFO_FRC_USEMASK (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCid");

    DBUG_PRINT ("EMFRC", ("Used variable: %s", ID_NAME (arg_node)););

    DFMSetMaskEntrySet (INFO_FRC_USEMASK (arg_info), NULL, ID_VARDEC (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCprf");

    if (PRF_PRF (arg_node) == F_fill) {
        PRF_ARG2 (arg_node) = FilterRCs (PRF_ARG2 (arg_node), arg_info);
    }

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCwith2");

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCwithop( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMFRCwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_MEM (arg_node) = FilterRCs (NWITHOP_MEM (arg_node), arg_info);
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        break;

    case WO_modarray:
        NWITHOP_MEM (arg_node) = FilterRCs (NWITHOP_MEM (arg_node), arg_info);
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;

    case WO_foldfun:
    case WO_foldprf:
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
