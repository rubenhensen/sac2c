/*
 *
 * $Log$
 * Revision 1.2  2004/10/11 14:49:27  ktr
 * alloc/with/inc_rc combinations are now treated as well.
 *
 * Revision 1.1  2004/10/10 09:55:18  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup rco Reference Counting Optimizations
 * @ingroup rcp
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
#include "LookUpTable.h"

/**
 * INFO structure
 */
struct INFO {
    bool downtrav;
    bool secondtrav;
    bool remassign;
    bool remnext;
    node *nextexpr;
    ids *lhs;
    LUT_t filllut;
};

/**
 * INFO macros
 */
#define INFO_RCO_DOWNTRAV(n) (n->downtrav)
#define INFO_RCO_SECONDTRAV(n) (n->secondtrav)
#define INFO_RCO_REMASSIGN(n) (n->remassign)
#define INFO_RCO_REMNEXT(n) (n->remnext)
#define INFO_RCO_NEXTEXPR(n) (n->nextexpr)
#define INFO_RCO_LHS(n) (n->lhs)
#define INFO_RCO_FILLLUT(n) (n->filllut)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RCO_DOWNTRAV (result) = FALSE;
    INFO_RCO_SECONDTRAV (result) = FALSE;
    INFO_RCO_REMASSIGN (result) = FALSE;
    INFO_RCO_REMNEXT (result) = FALSE;
    INFO_RCO_NEXTEXPR (result) = NULL;
    INFO_RCO_LHS (result) = NULL;
    INFO_RCO_FILLLUT (result) = NULL;

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
    bool secondtrav;
    bool remassign;

    DBUG_ENTER ("EMRCOassign");

    /*
     * Top-down traversal
     */
    INFO_RCO_DOWNTRAV (arg_info) = TRUE;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    secondtrav = INFO_RCO_SECONDTRAV (arg_info);
    INFO_RCO_SECONDTRAV (arg_info) = FALSE;

    remassign = INFO_RCO_REMASSIGN (arg_info);
    INFO_RCO_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS again if required
     */
    INFO_RCO_DOWNTRAV (arg_info) = FALSE;
    INFO_RCO_SECONDTRAV (arg_info) = secondtrav;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    INFO_RCO_SECONDTRAV (arg_info) = FALSE;

    if (INFO_RCO_REMNEXT (arg_info)) {
        DBUG_PRINT ("EMRCO", ("Removing assignment:"));
        DBUG_EXECUTE ("EMRCO", PrintNode (ASSIGN_NEXT (arg_node)););
        ASSIGN_NEXT (arg_node) = FreeNode (ASSIGN_NEXT (arg_node));
        INFO_RCO_REMNEXT (arg_info) = FALSE;
    }

    if (remassign || INFO_RCO_REMASSIGN (arg_info)) {
        DBUG_PRINT ("EMRCO", ("Removing assignment:"));
        DBUG_EXECUTE ("EMRCO", PrintNode (arg_node););
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
    LUT_t old_lut;
    ids *old_lhs;

    DBUG_ENTER ("EMRCOblock");

    old_lut = INFO_RCO_FILLLUT (arg_info);
    old_lhs = INFO_RCO_LHS (arg_info);

    INFO_RCO_FILLLUT (arg_info) = GenerateLUT ();
    INFO_RCO_NEXTEXPR (arg_info) = NULL;

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    INFO_RCO_FILLLUT (arg_info) = RemoveLUT (INFO_RCO_FILLLUT (arg_info));
    INFO_RCO_FILLLUT (arg_info) = old_lut;
    INFO_RCO_LHS (arg_info) = old_lhs;

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

    if (INFO_RCO_DOWNTRAV (arg_info) || INFO_RCO_SECONDTRAV (arg_info)) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        INFO_RCO_NEXTEXPR (arg_info) = NULL;
    } else {
        INFO_RCO_NEXTEXPR (arg_info) = LET_EXPR (arg_node);
    }

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

    if (INFO_RCO_DOWNTRAV (arg_info)) {
        /*
         * We are in Top-down traversal
         */
        node *avis;

        switch (PRF_PRF (arg_node)) {
        case F_alloc:
        case F_alloc_or_reuse:
            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_RCO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_inc_rc:
            avis = SearchInLUT_PP (INFO_RCO_FILLLUT (arg_info),
                                   ID_AVIS (PRF_ARG1 (arg_node)));

            if (avis != ID_AVIS (PRF_ARG1 (arg_node))) {
                node *alloc = ASSIGN_RHS (AVIS_SSAASSIGN (avis));

                NUM_VAL (PRF_ARG1 (alloc)) += NUM_VAL (PRF_ARG2 (arg_node));

                DBUG_PRINT ("EMRCO", ("Melted inc_rc into alloc!"));
                DBUG_EXECUTE ("EMRCO", PrintNode (AVIS_SSAASSIGN (avis)););

                INFO_RCO_REMASSIGN (arg_info) = TRUE;
            }
            break;

        case F_fill:
            INFO_RCO_FILLLUT (arg_info)
              = InsertIntoLUT_P (INFO_RCO_FILLLUT (arg_info),
                                 IDS_AVIS (INFO_RCO_LHS (arg_info)),
                                 ID_AVIS (PRF_ARG2 (arg_node)));
            break;

        default:
            break;
        }
    } else {

        if ((INFO_RCO_NEXTEXPR (arg_info) != NULL)
            && (NODE_TYPE (INFO_RCO_NEXTEXPR (arg_info)) == N_prf)) {

            node *prf = INFO_RCO_NEXTEXPR (arg_info);

            switch (PRF_PRF (arg_node)) {
            case F_alloc:
            case F_alloc_or_reuse:
                if ((PRF_PRF (prf) == F_dec_rc)
                    && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_RCO_LHS (arg_info)))
                    && (NUM_VAL (PRF_ARG1 (arg_node)) == NUM_VAL (PRF_ARG2 (prf)))) {

                    DBUG_PRINT ("EMRCO", ("Superfluous alloc/dec_rc combination found!"));
                    INFO_RCO_REMNEXT (arg_info) = TRUE;
                    INFO_RCO_REMASSIGN (arg_info) = TRUE;
                }
                break;

            default:
                break;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOwithop(node *arg_node, info *arg_info)
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
EMRCOwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
    case WO_modarray:
        INFO_RCO_FILLLUT (arg_info) = InsertIntoLUT_P (INFO_RCO_FILLLUT (arg_info),
                                                       IDS_AVIS (INFO_RCO_LHS (arg_info)),
                                                       ID_AVIS (NWITHOP_MEM (arg_node)));
        break;

    default:
        break;
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        INFO_RCO_LHS (arg_info) = IDS_NEXT (INFO_RCO_LHS (arg_info));
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup rco */
