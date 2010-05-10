/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup Move Syncs
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * ============================================================================
 * can be called on N_module               |   -----   | y   |       |
 * can be called on N_fundef               |   -----   | y   |       |
 * expects LaC funs                        |   -----   | *   |       |
 * follows N_ap to LaC funs                |   -----   | n   |       |
 * ============================================================================
 * deals with GLF properly                 |    yes    | y   |       |
 * ============================================================================
 * is aware of potential SAA annotations   |    yes    | y   |       |
 * utilises SAA annotations                |   -----   | n   |       |
 * ============================================================================
 * tolerates flattened N_array             |    yes    | y   |       |
 * tolerates flattened Generators          |    yes    | y   |       |
 * tolerates flattened operation parts     |    yes    | y   |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    | y   |       |
 * ============================================================================
 * tolerates multi-operator WLs            |    yes    | y** |       |
 * ============================================================================
 * *  Does not move syncs into or out of LaC funs
 * ** Untested
 * </pre>
 *
 * This traversal moves _syncin_ prf to just before the first use of there
 * lhs.
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_syncs.c
 *
 * Prefix: MS
 *
 *****************************************************************************/
#include "move_syncs.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *sync_assign;
    node *avis;
    bool found_avis;
    bool is_syncin;
};

/**
 * SYNC_ASSIGN the assigment node of the sync that we are moving
 * AVIS        the avis that affects where we can move to
 * FOUND_AVIS  found the avis that we are looking for
 * IS_SYNCIN   this is a syncin prf
 */
#define INFO_SYNC_ASSIGN(n) (n->sync_assign)
#define INFO_AVIS(n) (n->avis)
#define INFO_FOUND_AVIS(n) (n->found_avis)
#define INFO_IS_SYNCIN(n) (n->is_syncin)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SYNC_ASSIGN (result) = NULL;
    INFO_AVIS (result) = NULL;
    INFO_FOUND_AVIS (result) = FALSE;
    INFO_IS_SYNCIN (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MSdoMoveSyncs( node *syntax_tree)
 *
 *****************************************************************************/
node *
MSdoMoveSyncs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MSdoMoveSyncs");

    info = MakeInfo ();

    DBUG_PRINT ("MS", ("Starting move syncs traversal."));

    TRAVpush (TR_ms);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("TEMP", ("Move syncs traversal complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ATravId(node *arg_node)
 *
 * @brief Does the current ID node have the avis we are looking for?
 *
 *****************************************************************************/
static node *
ATravId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravId");

    if (ID_AVIS (arg_node) == INFO_AVIS (arg_info)) {
        INFO_FOUND_AVIS (arg_info) = TRUE;
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravAssign(node *arg_node)
 *
 * @brief Insert the sync assign before current assign if it is this assign
 *        that uses the avis that prevent more movement.
 *
 *****************************************************************************/
static node *
ATravAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_FOUND_AVIS (arg_info)) {
        ASSIGN_NEXT (INFO_SYNC_ASSIGN (arg_info)) = arg_node;
        arg_node = INFO_SYNC_ASSIGN (arg_info);

        INFO_SYNC_ASSIGN (arg_info) = NULL;
        INFO_FOUND_AVIS (arg_info) = FALSE;
        INFO_AVIS (arg_info) = NULL;
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *MSAssign(node *arg_node, info *arg_info)
 *
 * @brief If this assign is a sync in move it down as far as posible.
 *
 *****************************************************************************/
node *
MSassign (node *arg_node, info *arg_info)
{
    node *next = NULL;
    DBUG_ENTER ("MSAssign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        info *stack_info = MakeInfo (stack_info);
        next = TRAVdo (ASSIGN_NEXT (arg_node), stack_info);
        stack_info = FreeInfo (stack_info);
    }

    if (INFO_AVIS (arg_info) != NULL) {
        /* Found an unseen sync in */
        anontrav_t atrav[5]
          = {{N_assign, &ATravAssign},
             {N_id, &ATravId},
             {N_block, &TRAVnone}, /* Do not trav into blocks as we do */
             {N_code, &TRAVnone},  /* not want to change scope */
             {0, NULL}};

        ASSIGN_NEXT (arg_node) = NULL;
        INFO_SYNC_ASSIGN (arg_info) = arg_node;
        arg_node = next;

        TRAVpushAnonymous (atrav, &TRAVsons);
        arg_node = TRAVdo (next, arg_info);
        TRAVpop ();

        DBUG_ASSERT ((INFO_SYNC_ASSIGN (arg_info) == NULL),
                     "Do not know where to put sync assign");
    } else {
        ASSIGN_NEXT (arg_node) = next;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSPrf(node *arg_node, info *arg_info)
 *
 * @brief Is this a sync in prf?
 *
 *****************************************************************************/
node *
MSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSPrf");

    INFO_IS_SYNCIN (arg_info) = (PRF_PRF (arg_node) == F_syncin);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSLet(node *arg_node, info *arg_info)
 *
 * @brief Is this a sync in prf?
 *
 *****************************************************************************/
node *
MSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSLet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_IS_SYNCIN (arg_info)) {
        INFO_AVIS (arg_info) = IDS_AVIS (LET_IDS (arg_node));
        INFO_IS_SYNCIN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
