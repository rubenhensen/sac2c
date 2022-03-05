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
 * syntax_tree' =
 *   MAdoMoveAssigns( syntax_tree, pattern, block, count, stopPattern);
 *
 * where syntax_tree is the ast to change
 *       pattern     is a pattern that matches the rhs of lets of
 *                   assigns that should be moved down
 *       block       true if assigns can be moved into blocks.
 *                   if false leave assign before a block where it
 *                   is used
 *       count       move this number of blocking assigns down
 *       stopPattern give up moving assigns when lhs matches this
 *                   pattern
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

#define DBUG_PREFIX "MA"
#include "debug.h"

#include "pattern_match.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "ctinfo.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    pattern *mpattern;
    pattern *stopPattern;
    bool block;
    int count;

    node *assign;
    node *ids;
    bool found_avis;
    bool is_to_move;
    bool in_block;
    bool stop;
};

/**
 * PATTERN      the pattern to match with the rhs
 * STOP_PATTERN stop moving when reaching this lhs
 * BLOCK        can we move into blocks
 * COUNT        maximun number of assigns to move
 *
 * ASSIGN       the assigment node that we are moving
 * IDS          the ids that affects where we can move to
 * FOUND_AVIS   found the avis that we are looking for
 * IS_TO_MOVE   this is an assign that should be moved
 * IN_BLOCK     are we in a block in the anon trav
 * STOP         we should stop moveing as reached stop pattern
 */
#define INFO_PATTERN(n) (n->mpattern)
#define INFO_STOP_PATTERN(n) (n->stopPattern)
#define INFO_BLOCK(n) (n->block)
#define INFO_COUNT(n) (n->count)

#define INFO_ASSIGN(n) (n->assign)
#define INFO_IDS(n) (n->ids)
#define INFO_FOUND_AVIS(n) (n->found_avis)
#define INFO_IS_TO_MOVE(n) (n->is_to_move)
#define INFO_IN_BLOCK(n) (n->in_block)
#define INFO_STOP(n) (n->stop)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PATTERN (result) = NULL;
    INFO_STOP_PATTERN (result) = NULL;
    INFO_BLOCK (result) = FALSE;
    INFO_COUNT (result) = 0;

    INFO_ASSIGN (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_FOUND_AVIS (result) = FALSE;
    INFO_IS_TO_MOVE (result) = FALSE;
    INFO_IN_BLOCK (result) = FALSE;
    INFO_STOP (result) = FALSE;

    DBUG_RETURN (result);
}

/*
 * Make an info struct and copy global information
 */
static info *
MakeInfoClone (info *arg_info)
{
    info *result;

    DBUG_ENTER ();

    result = MakeInfo ();

    INFO_PATTERN (result) = INFO_PATTERN (arg_info);
    INFO_STOP_PATTERN (result) = INFO_STOP_PATTERN (arg_info);
    INFO_BLOCK (result) = INFO_BLOCK (arg_info);
    INFO_COUNT (result) = INFO_COUNT (arg_info);

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
MAdoMoveAssigns (node *syntax_tree, pattern *pat, bool block, int count,
                 pattern *stop_pat)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting move assigns traversal.");

    INFO_PATTERN (info) = pat;
    INFO_STOP_PATTERN (info) = stop_pat;
    INFO_BLOCK (info) = block;
    INFO_COUNT (info) = count;

    TRAVpush (TR_ma);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Move assigns traversal complete.");

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
static node *moveAssign (node *assign, node *assigns, info *arg_info);

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
    DBUG_ENTER ();

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
 * @fn node *ATravId(node *arg_node, info *arg_info)
 *
 * @brief Does the current ID node have the avis we are looking for?
 *
 *****************************************************************************/
static node *
ATravId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (SameAvis (INFO_IDS (arg_info), ID_AVIS (arg_node))) {
        INFO_FOUND_AVIS (arg_info) = TRUE;
        DBUG_PRINT ("found %s", AVIS_NAME (ID_AVIS (arg_node)));
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravLet( node *arg_node, info arg_info)
 *
 * @brief If rhs matches stop pattern then mark info stop
 *
 *****************************************************************************/
static node *
ATravLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (PMmatchFlat (INFO_STOP_PATTERN (arg_info), LET_EXPR (arg_node))) {
        INFO_STOP (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravAssign( node *arg_node, info *arg_info)
 *
 * @brief Insert the assign before current assign if it is this assign
 *        that uses the avis that prevents more movement or move this
 *        assign as well.
 *
 *****************************************************************************/
static node *
ATravAssign (node *arg_node, info *arg_info)
{
    bool stackFound;
    DBUG_ENTER ();

    stackFound = INFO_FOUND_AVIS (arg_info);
    INFO_FOUND_AVIS (arg_info) = FALSE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if ((INFO_FOUND_AVIS (arg_info) || INFO_STOP (arg_info))
        && (INFO_BLOCK (arg_info) || !INFO_IN_BLOCK (arg_info))) {

        if (INFO_COUNT (arg_info) > 0) {
            /* move current assign as well */
            node *assign = arg_node;
            node *chain = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (assign) = NULL;

            INFO_COUNT (arg_info)--;

            arg_node = moveAssign (assign, chain, arg_info);
            INFO_FOUND_AVIS (arg_info) = FALSE;
            /*
             * no longer put here as now we have moved blockers we may be
             * able to put this assign later
             */
            INFO_COUNT (arg_info) = 0;               /* Do not push any more */
            arg_node = TRAVopt (arg_node, arg_info); /* TRAV self */
        } else {
            ASSIGN_NEXT (INFO_ASSIGN (arg_info)) = arg_node;
            arg_node = INFO_ASSIGN (arg_info);

            INFO_ASSIGN (arg_info) = NULL;
            INFO_FOUND_AVIS (arg_info) = FALSE;
            INFO_IDS (arg_info) = NULL;
        }
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_FOUND_AVIS (arg_info) = stackFound || INFO_FOUND_AVIS (arg_info);

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
    bool stackFound = FALSE;
    DBUG_ENTER ();

    stack = INFO_IN_BLOCK (arg_info);
    stackFound = INFO_FOUND_AVIS (arg_info);
    INFO_IN_BLOCK (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_IN_BLOCK (arg_info) = stack;
    INFO_FOUND_AVIS (arg_info) = INFO_FOUND_AVIS (arg_info) || stackFound;
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *moveAssign(node *assign, node *assigns, info *arg_info)
 *
 * @brief Place assign as low down as possible in assigns.
 *
 *****************************************************************************/
static node *
moveAssign (node *assign, node *assigns, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_NEXT (assign) == NULL, "Can only move one assign at a time.");

    if ((assign != NULL) && (NODE_TYPE (ASSIGN_STMT (assign)) == N_let)) {
        anontrav_t atrav[6] = {{N_assign, &ATravAssign},
                               {N_id, &ATravId},
                               {N_block, &ATravBlock},
                               {N_let, &ATravLet},
                               {(nodetype)0, NULL}};
        info *stack_info = MakeInfoClone (arg_info);

        /* Been asked to move a let node.  We should be able to do that */
        if (LET_IDS (ASSIGN_STMT (assign)) != NULL) {
            /* Have a let with a lhs so try and move it */
            DBUG_PRINT ("Trying to move %s ...",
                        AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (assign)))));
            INFO_ASSIGN (stack_info) = assign;
            INFO_IDS (stack_info) = LET_IDS (ASSIGN_STMT (assign));

            TRAVpushAnonymous (atrav, &TRAVsons);
            assigns = TRAVopt (assigns, stack_info);
            TRAVpop ();
            if (INFO_ASSIGN (stack_info) != NULL) {
                CTInote (EMPTY_LOC, "Did not find use of lhs placing assign at end of block");
                DBUG_PRINT ("LHS %s ...",
                            AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (assign)))));
                assigns = TCappendAssign (assigns, INFO_ASSIGN (stack_info));
            } else {
                DBUG_PRINT ("Moved %s ...",
                            AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (assign)))));
            }

            INFO_ASSIGN (stack_info) = NULL;
        }
        stack_info = FreeInfo (stack_info);
    } else {
        assigns = TCappendAssign (assign, assigns);
    }

    DBUG_RETURN (assigns);
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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        info *stack_info = MakeInfoClone (arg_info);
        next = TRAVdo (ASSIGN_NEXT (arg_node), stack_info);
        stack_info = FreeInfo (stack_info);
    }

    if (INFO_IS_TO_MOVE (arg_info)) {
        node *move = arg_node;
        ASSIGN_NEXT (move) = NULL;

        arg_node = moveAssign (move, next, arg_info);
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
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (PMmatchFlat (INFO_PATTERN (arg_info), LET_EXPR (arg_node))) {
        INFO_IS_TO_MOVE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
