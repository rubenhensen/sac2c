/*
 *
 * $Log$
 * Revision 1.13  2005/09/27 20:32:44  ktr
 * memory reused statically by reshape is not freed
 *
 * Revision 1.12  2004/12/10 18:41:38  ktr
 * EMRCOfundef added.
 *
 * Revision 1.11  2004/12/09 21:09:26  ktr
 * bugfix roundup
 *
 * Revision 1.10  2004/11/27 03:11:25  ktr
 * added EMRCOgenarray
 *
 * Revision 1.9  2004/11/27 01:58:46  ktr
 * EMRCOdoRefCountOpt
 *
 * Revision 1.8  2004/11/23 17:40:16  ktr
 * COMPILES!!!
 *
 * Revision 1.7  2004/11/23 17:32:52  jhb
 * compile
 *
 * Revision 1.6  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.5  2004/11/02 14:29:58  ktr
 * Support for F_free added.
 *
 * Revision 1.4  2004/10/22 15:18:40  ktr
 * Moved some functionality into reuseelimination.c
 *
 * Revision 1.3  2004/10/21 16:22:07  ktr
 * Added support for static reuse.
 *
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
#include "rcopt.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "free.h"
#include "internal_lib.h"
#include "DataFlowMask.h"

/**
 * INFO structure
 */
struct INFO {
    bool downtrav;
    bool secondtrav;
    bool remassign;
    bool remnext;
    node *nextexpr;
    node *lhs;
    lut_t *filllut;
    dfmask_t *nofreemask;
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
#define INFO_RCO_NOFREEMASK(n) (n->nofreemask)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RCO_DOWNTRAV (result) = FALSE;
    INFO_RCO_SECONDTRAV (result) = FALSE;
    INFO_RCO_REMASSIGN (result) = FALSE;
    INFO_RCO_REMNEXT (result) = FALSE;
    INFO_RCO_NEXTEXPR (result) = NULL;
    INFO_RCO_LHS (result) = NULL;
    INFO_RCO_FILLLUT (result) = NULL;
    INFO_RCO_NOFREEMASK (result) = NULL;

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
 * @fn node *EMRCOdoRefCountOpt( node *syntax_tree)
 *
 * @brief starting point of Reference counting optimizations.
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMRCOdoRefCountOpt (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRCOdoRefCountOpt");

    DBUG_PRINT ("EMRCO", ("Starting Reference counting optimizations"));

    info = MakeInfo ();

    TRAVpush (TR_emrco);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

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
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    secondtrav = INFO_RCO_SECONDTRAV (arg_info);
    INFO_RCO_SECONDTRAV (arg_info) = FALSE;

    remassign = INFO_RCO_REMASSIGN (arg_info);
    INFO_RCO_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS again if required
     */
    INFO_RCO_DOWNTRAV (arg_info) = FALSE;
    INFO_RCO_SECONDTRAV (arg_info) = secondtrav;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    INFO_RCO_SECONDTRAV (arg_info) = FALSE;

    if (INFO_RCO_REMNEXT (arg_info)) {
        DBUG_PRINT ("EMRCO", ("Removing assignment:"));
        DBUG_EXECUTE ("EMRCO", PRTdoPrintNode (ASSIGN_NEXT (arg_node)););
        ASSIGN_NEXT (arg_node) = FREEdoFreeNode (ASSIGN_NEXT (arg_node));
        INFO_RCO_REMNEXT (arg_info) = FALSE;
    }

    if (remassign || INFO_RCO_REMASSIGN (arg_info)) {
        DBUG_PRINT ("EMRCO", ("Removing assignment:"));
        DBUG_EXECUTE ("EMRCO", PRTdoPrintNode (arg_node););
        arg_node = FREEdoFreeNode (arg_node);
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
    lut_t *old_lut;
    node *old_lhs;

    DBUG_ENTER ("EMRCOblock");

    old_lut = INFO_RCO_FILLLUT (arg_info);
    old_lhs = INFO_RCO_LHS (arg_info);

    INFO_RCO_FILLLUT (arg_info) = LUTgenerateLut ();
    INFO_RCO_NEXTEXPR (arg_info) = NULL;

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    INFO_RCO_FILLLUT (arg_info) = LUTremoveLut (INFO_RCO_FILLLUT (arg_info));
    INFO_RCO_FILLLUT (arg_info) = old_lut;
    INFO_RCO_LHS (arg_info) = old_lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOfundef(node *arg_node, info *arg_info)
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
EMRCOfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *maskbase;

    DBUG_ENTER ("EMRCOfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        INFO_RCO_NOFREEMASK (arg_info) = DFMgenMaskClear (maskbase);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_RCO_NOFREEMASK (arg_info) = DFMremoveMask (INFO_RCO_NOFREEMASK (arg_info));
        maskbase = DFMremoveMaskBase (maskbase);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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

    if (INFO_RCO_DOWNTRAV (arg_info) || INFO_RCO_SECONDTRAV (arg_info)) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
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

        case F_reuse:
            /*
             * Mark reused variable in NOFREEMASK such that it will not be
             * statically freed
             */
            DFMsetMaskEntrySet (INFO_RCO_NOFREEMASK (arg_info), NULL,
                                ID_AVIS (PRF_ARG2 (arg_node)));

            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_RCO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_reshape:
            /*
             * Mark reused variable in NOFREEMASK such that it will not be
             * statically freed
             */
            DFMsetMaskEntrySet (INFO_RCO_NOFREEMASK (arg_info), NULL,
                                ID_AVIS (PRF_ARG4 (arg_node)));

            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_RCO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_alloc:
        case F_alloc_or_reuse:
        case F_alloc_or_reshape:
            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_RCO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_dec_rc:
            /*
             * Convert dec_rc( b, n) into free( b) iff
             *  - b is not aliased AND
             *  - b has not been reused ( marked in NOFREEMASK)
             */
            if ((!AVIS_ISALIAS (ID_AVIS (PRF_ARG1 (arg_node))))
                && (!DFMtestMaskEntry (INFO_RCO_NOFREEMASK (arg_info), NULL,
                                       ID_AVIS (PRF_ARG1 (arg_node))))) {
                node *new_node = TCmakePrf1 (F_free, DUPdoDupNode (PRF_ARG1 (arg_node)));
                arg_node = FREEdoFreeNode (arg_node);
                arg_node = new_node;
            }
            break;

        case F_inc_rc:
            avis = LUTsearchInLutPp (INFO_RCO_FILLLUT (arg_info),
                                     ID_AVIS (PRF_ARG1 (arg_node)));

            if (avis != ID_AVIS (PRF_ARG1 (arg_node))) {
                node *alloc = ASSIGN_RHS (AVIS_SSAASSIGN (avis));

                NUM_VAL (PRF_ARG1 (alloc)) += NUM_VAL (PRF_ARG2 (arg_node));

                DBUG_PRINT ("EMRCO", ("Melted inc_rc into alloc!"));
                DBUG_EXECUTE ("EMRCO", PRTdoPrintNode (AVIS_SSAASSIGN (avis)););

                INFO_RCO_REMASSIGN (arg_info) = TRUE;
            }
            break;

        case F_fill:
            INFO_RCO_FILLLUT (arg_info)
              = LUTinsertIntoLutP (INFO_RCO_FILLLUT (arg_info),
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
            case F_reuse:
            case F_alloc_or_reshape:
            case F_reshape:
                if ((PRF_PRF (prf) == F_dec_rc)
                    && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_RCO_LHS (arg_info)))
                    && (NUM_VAL (PRF_ARG1 (arg_node)) == NUM_VAL (PRF_ARG2 (prf)))) {

                    DBUG_PRINT ("EMRCO", ("Superfluous alloc/dec_rc combination found!"));
                    INFO_RCO_REMNEXT (arg_info) = TRUE;
                    INFO_RCO_REMASSIGN (arg_info) = TRUE;
                }
                if ((PRF_PRF (prf) == F_free)
                    && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_RCO_LHS (arg_info)))) {

                    DBUG_PRINT ("EMRCO", ("Superfluous alloc/free combination found!"));
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
 * @fn node *EMRCOgenarray(node *arg_node, info *arg_info)
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
EMRCOgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOgenarray");

    INFO_RCO_FILLLUT (arg_info) = LUTinsertIntoLutP (INFO_RCO_FILLLUT (arg_info),
                                                     IDS_AVIS (INFO_RCO_LHS (arg_info)),
                                                     ID_AVIS (GENARRAY_MEM (arg_node)));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_RCO_LHS (arg_info) = IDS_NEXT (INFO_RCO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRCOfold(node *arg_node, info *arg_info)
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
EMRCOfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOfold");

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_RCO_LHS (arg_info) = IDS_NEXT (INFO_RCO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
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
EMRCOmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCOmodarray");

    INFO_RCO_FILLLUT (arg_info) = LUTinsertIntoLutP (INFO_RCO_FILLLUT (arg_info),
                                                     IDS_AVIS (INFO_RCO_LHS (arg_info)),
                                                     ID_AVIS (MODARRAY_MEM (arg_node)));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_RCO_LHS (arg_info) = IDS_NEXT (INFO_RCO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup rco */
