/*
 *
 * $Log$
 * Revision 1.1  2004/10/10 09:55:18  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup rco Reference Counting Optimizations
 * @ingroup rc
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file rcopt.c
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

/**
 * INFO structure
 */
struct INFO {
    bool remassign;
    bool remnext;
    node *nextexpr;
    ids *lhs;
};

/**
 * INFO macros
 */
#define INFO_RCO_REMASSIGN(n) (n->remassign)
#define INFO_RCO_REMNEXT(n) (n->remnext)
#define INFO_RCO_NEXTEXPR(n) (n->nextexpr)
#define INFO_RCO_LHS(n) (n->lhs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RCO_REMASSIGN (result) = FALSE;
    INFO_RCO_REMNEXT (result) = FALSE;
    INFO_RCO_NEXTEXPR (result) = NULL;
    INFO_RCO_LHS (result) = NULL;

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
 * @fn node *EMRCORefCountOpt( node *syntax_tree)
 *
 * @brief starting point of Reference counting optimizations.
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMRCORefCountOpt (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRCORefCountOpt");

    DBUG_PRINT ("EMRCO", ("Starting Reference counting optimizations"));

    info = MakeInfo ();

    act_tab = emrco_tab;

    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_PRINT ("EMRCO", ("Reference counting optimizations complete"));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Reference Counting Optimizations traversal (emrco_tab)
 *
 * prefix: EMRCO
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMRCOassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMRCOassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_RCO_REMNEXT (arg_info)) {
        ASSIGN_NEXT (arg_node) = FreeNode (ASSIGN_NEXT (arg_node));
        INFO_RCO_REMNEXT (arg_info) = FALSE;
    }

    if (INFO_RCO_REMASSIGN (arg_info)) {
        arg_node = FreeNode (arg_node);
        INFO_RCO_REMASSIGN (arg_info) = FALSE;
        INFO_RCO_NEXTEXPR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMRCOblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOblock");

    INFO_RCO_NEXTEXPR (arg_info) = NULL;

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMRCOlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOlet");

    INFO_RCO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_RCO_NEXTEXPR (arg_info) = LET_EXPR (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOprf(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMRCOprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOprf");

    if ((INFO_RCO_NEXTEXPR (arg_info) != NULL)
        && (NODE_TYPE (INFO_RCO_NEXTEXPR (arg_info)) == N_prf)) {

        node *prf = INFO_RCO_NEXTEXPR (arg_info);

        switch (PRF_PRF (arg_node)) {

        case F_alloc:
        case F_alloc_or_reuse:
            if ((PRF_PRF (prf) == F_dec_rc)
                && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_RCO_LHS (arg_info)))
                && (NUM_VAL (PRF_ARG1 (arg_node)) == NUM_VAL (PRF_ARG2 (prf)))) {

                INFO_RCO_REMNEXT (arg_info) = TRUE;
                INFO_RCO_REMASSIGN (arg_info) = TRUE;
            }
            break;

        case F_fill:
            if ((PRF_PRF (prf) == F_inc_rc)
                && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_RCO_LHS (arg_info)))) {
                node *alloc;
                alloc = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))));

                NUM_VAL (PRF_ARG1 (alloc)) += NUM_VAL (PRF_ARG2 (prf));
                INFO_RCO_REMNEXT (arg_info) = TRUE;
            }
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup rco */
