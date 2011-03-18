/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup Move Assigns
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * ============================================================================
 * can be called on N_module               |   -----   | y   |       |
 * can be called on N_fundef               |   -----   | y   |       |
 * expects LaC funs                        |   -----   | y*  |       |
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
 * *  Does not move assigns into or out of LaC funs
 * ** Untested
 * </pre>
 *
 * This traversal moves assignment statements to just before the first
 * use of there lhs.
 *
 * This is currently a work in progress to make this code generic
 * enough for all use cases.
 *
 * Matching rhs must have at least one of the lhs used somewhere that
 * can be found.  AKA matching rhs must to be part of dead code.
 *
 * syntax_tree' = MAdoMoveAssigns( syntax_tree, pattern, block);
 *
 * where syntax_tree is the ast to change
 *       pattern     is a pattern that matches the rhs of lets of
 *                   assigns that should be moved down
 *       block       true if assigns can be moved into blocks.
 *                   if false leave assign before a block where it
 *                   is used
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_assigns.c
 *
 * Prefix: MA
 *
 *****************************************************************************/
#include "move_assigns.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "pattern_match.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "ctinfo.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    pattern *pattern;
    bool block;

    node *assign;
    node *ids;
    bool found_avis;
    bool is_to_move;
    bool in_block;
};

/**
 * PATTERN     the pattern to match with the rhs
 * BLOCK       can we move into blocks
 *
 * ASSIGN      the assigment node that we are moving
 * IDS         the ids that affects where we can move to
 * FOUND_AVIS  found the avis that we are looking for
 * IS_TO_MOVE  this is an assign that should be moved
 * IN_BLOCK    are we in a block in the anon trav
 */
#define INFO_PATTERN(n) (n->pattern)
#define INFO_BLOCK(n) (n->block)

#define INFO_ASSIGN(n) (n->assign)
#define INFO_IDS(n) (n->ids)
#define INFO_FOUND_AVIS(n) (n->found_avis)
#define INFO_IS_TO_MOVE(n) (n->is_to_move)
#define INFO_IN_BLOCK(n) (n->in_block)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PATTERN (result) = FALSE;
    INFO_BLOCK (result) = FALSE;

    INFO_ASSIGN (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_FOUND_AVIS (result) = FALSE;
    INFO_IS_TO_MOVE (result) = FALSE;
    INFO_IN_BLOCK (result) = FALSE;

    DBUG_RETURN (result);
}

/*
 * Make an info struct and copy global information
 */
static info *
MakeInfoClone (info *arg_info)
{
    info *result;

    DBUG_ENTER ("MakeInfoClone");

    result = MakeInfo ();

    INFO_PATTERN (result) = INFO_PATTERN (arg_info);
    INFO_BLOCK (result) = INFO_BLOCK (arg_info);

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
 * @fn node *MAdoMoveAssigns( node *syntax_tree, pattern *pat, bool block)
 *
 *****************************************************************************/
node *
MAdoMoveAssigns (node *syntax_tree, pattern *pat, bool block)
{
    info *info;

    DBUG_ENTER ("MAdoMoveAssigns");

    info = MakeInfo ();

    DBUG_PRINT ("MA", ("Starting move assigns traversal."));

    INFO_PATTERN (info) = pat;
    INFO_BLOCK (info) = block;

    TRAVpush (TR_ma);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("MA", ("Move assigns traversal complete."));

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
 * @fn bool SameAvis(node *ids, node *avis)
 *
 * @brief Return true if any of the avis in ids is the same as avis
 *
 *****************************************************************************/
static bool
SameAvis (node *ids, node *avis)
{
    bool res = FALSE;
    DBUG_ENTER ("SameAvis");

    if (ids != NULL) {
        if (IDS_AVIS (ids) == avis) {
            res = TRUE;
        } else {
            res = SameAvis (IDS_NEXT (ids), avis);
        }
    }

    DBUG_RETURN (res);
}

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

    if (SameAvis (INFO_IDS (arg_info), ID_AVIS (arg_node))) {
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
 *        that uses the avis that prevents more movement.
 *
 *****************************************************************************/
static node *
ATravAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_FOUND_AVIS (arg_info)
        && (INFO_BLOCK (arg_info) || !INFO_IN_BLOCK (arg_info))) {
        ASSIGN_NEXT (INFO_ASSIGN (arg_info)) = arg_node;
        arg_node = INFO_ASSIGN (arg_info);

        INFO_ASSIGN (arg_info) = NULL;
        INFO_FOUND_AVIS (arg_info) = FALSE;
        INFO_IDS (arg_info) = NULL;
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravBlock( node *arg_node)
 *
 * @brief Look into this block to see if any lhs are used but do not
 * move the code into this block.  If the lhs is used in this block
 * place the assign above the block.
 *
 *****************************************************************************/
static node *
ATravBlock (node *arg_node, info *arg_info)
{
    bool stack = FALSE;
    DBUG_ENTER ("ATravBlock");

    stack = INFO_IN_BLOCK (arg_info);
    INFO_IN_BLOCK (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_IN_BLOCK (arg_info) = stack;

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
 * @fn node *MAAssign(node *arg_node, info *arg_info)
 *
 * @brief If this assign is a moveable assign move it down as far as
 * posible. But not into a different block
 *
 *****************************************************************************/
node *
MAassign (node *arg_node, info *arg_info)
{
    node *next = NULL;
    DBUG_ENTER ("MAAssign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        info *stack_info = MakeInfoClone (arg_info);
        next = TRAVdo (ASSIGN_NEXT (arg_node), stack_info);
        stack_info = FreeInfo (stack_info);
    }

    if (INFO_IDS (arg_info) != NULL) {
        anontrav_t atrav[5] = {{N_assign, &ATravAssign},
                               {N_id, &ATravId},
                               {N_block, &ATravBlock},
                               {0, NULL}};

        ASSIGN_NEXT (arg_node) = NULL;
        INFO_ASSIGN (arg_info) = arg_node;
        arg_node = next;

        TRAVpushAnonymous (atrav, &TRAVsons);
        arg_node = TRAVopt (next, arg_info);
        TRAVpop ();

        if (INFO_ASSIGN (arg_info) != NULL) {
            CTInote ("Did not find use of lhs placing assign at end of block");
            arg_node = TCappendAssign (next, INFO_ASSIGN (arg_info));
            INFO_ASSIGN (arg_info) = NULL;
        }
    } else {
        ASSIGN_NEXT (arg_node) = next;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MALet(node *arg_node, info *arg_info)
 *
 * @brief Is this a moveable assign
 *
 *****************************************************************************/
node *
MAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MALet");

    arg_node = TRAVcont (arg_node, arg_info);

    if (PMmatchFlat (INFO_PATTERN (arg_info), LET_EXPR (arg_node))) {
        INFO_IDS (arg_info) = LET_IDS (arg_node);
        INFO_IS_TO_MOVE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
