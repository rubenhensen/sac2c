#include "wl_lock_optimization_marking.h"

#include "tree_basic.h"
#include "tree_compound.h"
/*#include "str.h"*/
#include "memory.h"
/*
#include "shape.h"
#include "new_types.h"*/
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"

/**
 *
 * @wl_lock_optimization_marking.c
 *
 * @brief The traversels in this file marks the assignements which are not
 *        allowed to be moved above the lock or beneath the unlock.
 *
 */

/**
 * INFO structure
 */
struct INFO {
    int wllevel;
    bool fv;
    bool mark_nup;
    bool mark_ndown;
    bool wb;
};

/**
 * INFO macros
 */
#define INFO_WLLEVEL(n) ((n)->wllevel)
#define INFO_FV(n) ((n)->fv)
#define INFO_MARK_NUP(n) ((n)->mark_nup)
#define INFO_MARK_NDOWN(n) ((n)->mark_ndown)
#define INFO_WB(n) ((n)->wb)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WLLEVEL (result) = 0;
    INFO_FV (result) = FALSE;
    INFO_MARK_NUP (result) = FALSE;
    INFO_MARK_NDOWN (result) = FALSE;
    INFO_WB (result) = FALSE;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
WLLOMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMprf");

    if (PRF_PRF (arg_node) == F_prop_obj_in) {
        INFO_FV (arg_info) = TRUE;
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}
node *
WLLOMid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMid");

    if (AVIS_NUP (ID_AVIS (arg_node)) == TRUE) {
        INFO_FV (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

node *
WLLOMids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMids");

    if (INFO_MARK_NUP (arg_info)) {
        DBUG_PRINT ("WLLOM", ("Mark ID %s", IDS_NAME (arg_node)));
        AVIS_NUP (IDS_AVIS (arg_node)) = TRUE;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief if this assignment is a let-assignment this functions traversals down
 *        the RHS. If the RHS includes an variable which is marked as not beeing
 *        able to be moved upon the lock, all variables on the LHS are marked to
 *        be not allowed to be moved upon this lock.
 *        Additional if this is the last assignment in the assignment-Chain,
 *        this circumstance is marked within the INFO-structure.
 *
 * @param arg_node N_with N_assign
 * @param arg_info INFO structure
 *
 * @return N_assign node
 *******************************************************************************/
node *
WLLOMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMassign");

    /*TravDown and therefore !UP-Part*/
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if ((INFO_WLLEVEL (arg_info) == 1) && (INFO_MARK_NUP (arg_info) == TRUE)) {
        ASSIGN_NUP (arg_node) = TRUE;
        DBUG_PRINT ("WLLOM", ("!!! Marked %s=... entirely", ASSIGN_NAME (arg_node)));

        INFO_MARK_NUP (arg_info) = FALSE;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        INFO_WB (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

node *
WLLOMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOlet");
    DBUG_PRINT ("WLLOM", ("LET_NAME: %s", LET_NAME (arg_node)));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if ((INFO_WLLEVEL (arg_info) == 1) && (INFO_FV (arg_info) == TRUE)) {
        INFO_MARK_NUP (arg_info) = TRUE;

        DBUG_PRINT ("WLLOM", ("??? Mark %s=...", LET_NAME (arg_node)));
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        DBUG_PRINT ("WLLOM", ("Marked IDS..."));

        INFO_FV (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief  this function just increases the with-loop level counter in the
 *         INFO-structure when entering this node, continues traversing and
 *         decreases wl level counter afterwards.
 *
 * @param arg_node N_with node
 * @param arg_info INFO structure
 *
 * @return unchanged N_with node
 *******************************************************************************/
node *
WLLOMwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMwith");

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) + 1;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) - 1;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the marking of the not free movable assignments
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 *******************************************************************************/
node *
WLLOMdoLockOptimizationMarking (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLLOMdoLockOptimizationMarking");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "WLLOMdoLockOptimizationMarking is intended to run on the entire tree");

    info = MakeInfo ();
    TRAVpush (TR_wllom);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
