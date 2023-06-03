/**
 * @defgroup rco Reference Counting Optimizations
 * @ingroup mm
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

#define DBUG_PREFIX "EMRCO"
#include "debug.h"

#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "free.h"
#include "str.h"
#include "memory.h"
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
#define INFO_DOWNTRAV(n) (n->downtrav)
#define INFO_SECONDTRAV(n) (n->secondtrav)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_REMNEXT(n) (n->remnext)
#define INFO_NEXTEXPR(n) (n->nextexpr)
#define INFO_LHS(n) (n->lhs)
#define INFO_FILLLUT(n) (n->filllut)
#define INFO_NOFREEMASK(n) (n->nofreemask)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_DOWNTRAV (result) = FALSE;
    INFO_SECONDTRAV (result) = FALSE;
    INFO_REMASSIGN (result) = FALSE;
    INFO_REMNEXT (result) = FALSE;
    INFO_NEXTEXPR (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_FILLLUT (result) = NULL;
    INFO_NOFREEMASK (result) = NULL;

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

    DBUG_ENTER ();

    DBUG_PRINT ("Starting Reference counting optimizations");

    info = MakeInfo ();

    TRAVpush (TR_emrco);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Reference counting optimizations complete");

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

    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    INFO_DOWNTRAV (arg_info) = TRUE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    secondtrav = INFO_SECONDTRAV (arg_info);
    INFO_SECONDTRAV (arg_info) = FALSE;

    remassign = INFO_REMASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS again if required
     */
    INFO_DOWNTRAV (arg_info) = FALSE;
    INFO_SECONDTRAV (arg_info) = secondtrav;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_SECONDTRAV (arg_info) = FALSE;

    if (INFO_REMNEXT (arg_info)) {
        DBUG_PRINT ("Removing assignment:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, ASSIGN_NEXT (arg_node)));
        ASSIGN_NEXT (arg_node) = FREEdoFreeNode (ASSIGN_NEXT (arg_node));
        INFO_REMNEXT (arg_info) = FALSE;
    }

    if (remassign || INFO_REMASSIGN (arg_info)) {
        DBUG_PRINT ("Removing assignment:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMASSIGN (arg_info) = FALSE;
        INFO_NEXTEXPR (arg_info) = NULL;
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

    DBUG_ENTER ();

    old_lut = INFO_FILLLUT (arg_info);
    old_lhs = INFO_LHS (arg_info);

    INFO_FILLLUT (arg_info) = LUTgenerateLut ();
    INFO_NEXTEXPR (arg_info) = NULL;

    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    }

    INFO_FILLLUT (arg_info) = LUTremoveLut (INFO_FILLLUT (arg_info));
    INFO_FILLLUT (arg_info) = old_lut;
    INFO_LHS (arg_info) = old_lhs;

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

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        INFO_NOFREEMASK (arg_info) = DFMgenMaskClear (maskbase);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_NOFREEMASK (arg_info) = DFMremoveMask (INFO_NOFREEMASK (arg_info));
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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    if (INFO_DOWNTRAV (arg_info) || INFO_SECONDTRAV (arg_info)) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        INFO_NEXTEXPR (arg_info) = NULL;
    } else {
        INFO_NEXTEXPR (arg_info) = LET_EXPR (arg_node);
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
    DBUG_ENTER ();

    if (INFO_DOWNTRAV (arg_info)) {
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
            DFMsetMaskEntrySet (INFO_NOFREEMASK (arg_info),
                                ID_AVIS (PRF_ARG2 (arg_node)));

            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_reshape_VxA:
            /*
             * Mark reused variable in NOFREEMASK such that it will not be
             * statically freed
             */
            DFMsetMaskEntrySet (INFO_NOFREEMASK (arg_info),
                                ID_AVIS (PRF_ARG4 (arg_node)));

            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_resize:
            /*
             * Mark reused variable in NOFREEMASK such that it will not be
             * statically freed
             */
            DFMsetMaskEntrySet (INFO_NOFREEMASK (arg_info),
                                ID_AVIS (PRF_ARG4 (arg_node)));

            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_alloc:
        case F_alloc_or_reuse:
        case F_alloc_or_reshape:
        case F_alloc_or_resize:
            /*
             * This node must be revisited in bottom-up traversal
             */
            INFO_SECONDTRAV (arg_info) = TRUE;
            break;

        case F_dec_rc:
            /*
             * Convert dec_rc( b, n) into free( b) iff
             *  - b is not aliased AND
             *  - b has not been reused ( marked in NOFREEMASK)
             */
            if ((!AVIS_ISALIAS (ID_AVIS (PRF_ARG1 (arg_node))))
                && (!DFMtestMaskEntry (INFO_NOFREEMASK (arg_info),
                                       ID_AVIS (PRF_ARG1 (arg_node))))) {
                node *new_node = TCmakePrf1 (F_free, DUPdoDupNode (PRF_ARG1 (arg_node)));
                arg_node = FREEdoFreeNode (arg_node);
                arg_node = new_node;
            }
            break;

        case F_inc_rc:
            avis = (node *)LUTsearchInLutPp (INFO_FILLLUT (arg_info),
                                             ID_AVIS (PRF_ARG1 (arg_node)));

            if ((avis != ID_AVIS (PRF_ARG1 (arg_node)))
                && (AVIS_SSAASSIGN (avis) != NULL)) {
                node *alloc = ASSIGN_RHS (AVIS_SSAASSIGN (avis));

                NUM_VAL (PRF_ARG1 (alloc)) += NUM_VAL (PRF_ARG2 (arg_node));

                DBUG_PRINT ("Melted inc_rc into alloc!");
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, AVIS_SSAASSIGN (avis)));

                INFO_REMASSIGN (arg_info) = TRUE;
            }
            break;

        case F_fill:
            INFO_FILLLUT (arg_info) = LUTinsertIntoLutP (INFO_FILLLUT (arg_info),
                                                         IDS_AVIS (INFO_LHS (arg_info)),
                                                         ID_AVIS (PRF_ARG2 (arg_node)));
            break;

        default:
            break;
        }
    } else {

        if ((INFO_NEXTEXPR (arg_info) != NULL)
            && (NODE_TYPE (INFO_NEXTEXPR (arg_info)) == N_prf)) {

            node *prf = INFO_NEXTEXPR (arg_info);

            switch (PRF_PRF (arg_node)) {
            case F_alloc:
            case F_alloc_or_reuse:
            case F_reuse:
            case F_alloc_or_resize:
            case F_resize:
            case F_alloc_or_reshape:
            case F_reshape_VxA:
                if ((PRF_PRF (prf) == F_dec_rc)
                    && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_LHS (arg_info)))
                    && (NUM_VAL (PRF_ARG1 (arg_node)) == NUM_VAL (PRF_ARG2 (prf)))) {

                    DBUG_PRINT ("Superfluous alloc/dec_rc combination found!");
                    INFO_REMNEXT (arg_info) = TRUE;
                    INFO_REMASSIGN (arg_info) = TRUE;
                }
                if ((PRF_PRF (prf) == F_free)
                    && (ID_AVIS (PRF_ARG1 (prf)) == IDS_AVIS (INFO_LHS (arg_info)))) {

                    DBUG_PRINT ("Superfluous alloc/free combination found!");
                    INFO_REMNEXT (arg_info) = TRUE;
                    INFO_REMASSIGN (arg_info) = TRUE;
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
    DBUG_ENTER ();

    INFO_FILLLUT (arg_info)
      = LUTinsertIntoLutP (INFO_FILLLUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (GENARRAY_MEM (arg_node)));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
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
    DBUG_ENTER ();

    if (FOLD_PARTIALMEM (arg_node) != NULL) {
        INFO_FILLLUT (arg_info)
          = LUTinsertIntoLutP (INFO_FILLLUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                               ID_AVIS (FOLD_PARTIALMEM (arg_node)));
    }

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
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
    DBUG_ENTER ();

    INFO_FILLLUT (arg_info)
      = LUTinsertIntoLutP (INFO_FILLLUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (MODARRAY_MEM (arg_node)));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup rco */

#undef DBUG_PREFIX
