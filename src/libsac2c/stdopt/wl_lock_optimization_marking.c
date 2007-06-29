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

/** <!-- ****************************************************************** -->
 * @brief If the traversal is on its way down this function traverses down the
 *        arguments to figure out if one contains a variable which depends on
 *        an object.
 *        If the function is a prop_obj_in, it is not necessary to look into
 *        the arguments.
 *        If the traversal is on its way up the tree and either the flag which
 *        indicates that all variables should be marked as !DOWN is set or the
 *        PRF is a prop_obj_out, this function traverses further into the
 *        arguments to mark them as !DOWN.
 *
 * @param arg_node N_prf node
 * @param arg_info INFO structure
 *
 * @return N_prf node
 *******************************************************************************/
node *
WLLOMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMprf");

    if (INFO_WB (arg_info) == FALSE) { /*Traverses down the tree...*/
        if (PRF_PRF (arg_node) == F_prop_obj_in) {
            INFO_FV (arg_info) = TRUE;
        } else {
            arg_node = TRAVcont (arg_node, arg_info);
        }
    } else {                                        /*WB == TRUE*/
        if (PRF_PRF (arg_node) == F_prop_obj_out) { /*If prf is prop_obj_out*/
            INFO_FV (arg_info) = TRUE;              /*Set flags to mark*/
            INFO_MARK_NDOWN (arg_info) = TRUE;

            DBUG_PRINT ("WLLOM", ("??? PROP_OBJ_OUT found, Mark last assignment"));
        }
        if (INFO_MARK_NDOWN (arg_info) == TRUE) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info); /*all vars*/
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief This functions just resets the Way-Back flag in the INFO-structure.
 *
 * @param arg_node N_fundef node
 * @param arg_info INFO structure
 *
 * @return N_fundef node
 *******************************************************************************/
node *
WLLOMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMfundef");

    INFO_WB (arg_info) = FALSE;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief If the traversal is on its way down this function just sets the flag
 *         in the INFO-structure which shows that this ID depends on an object,
 *         if this ID depends on an object.
 *         If the traversal is on its way back this function marks the IDs as
 *         !DOWN if the corresponding INFO-flag is set.
 *
 * @param arg_node N_id node
 * @param arg_info INFO structure
 *
 * @return N_id node
 *******************************************************************************/
node *
WLLOMid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMid");

    if (INFO_WB (arg_info) == FALSE) {
        if (AVIS_NUP (ID_AVIS (arg_node)) == TRUE) {
            INFO_FV (arg_info) = TRUE;
        }
    } else { /*WB == TRUE*/
        if (INFO_MARK_NDOWN (arg_info) == TRUE) {
            DBUG_PRINT ("WILLOM", ("Mark ID %s", ID_NAME (arg_node)));
            AVIS_NDOWN (ID_AVIS (arg_node)) = TRUE;
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief If the traversal is on its way down this function just marks the ids
 *        as !UP if the corresponding flag inside the INFO-structure is set.
 *        Afterwards it traverses further down the ids-chain.
 *        If the traversal is on its way back this functions checks if the
 *        IDS is marked as !DOWN and then sets the corresponding flag in the
 *        INFO-structure.
 *
 * @param arg_node N_ids node
 * @param arg_info INFO structure
 *
 * @return N_ids node
 *******************************************************************************/
node *
WLLOMids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMids");

    if (INFO_WB (arg_info) == FALSE) { /*On its way down...*/
        if (INFO_MARK_NUP (arg_info) == TRUE) {
            DBUG_PRINT ("WLLOM", ("Mark IDS %s", IDS_NAME (arg_node)));
            AVIS_NUP (IDS_AVIS (arg_node)) = TRUE;
        }
    } else { /*WB == TRUE*/
        if (AVIS_NDOWN (IDS_AVIS (arg_node)) == TRUE) {
            INFO_FV (arg_info) = TRUE;
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief if the Traversal is on its way down this function just traverses into
 *        the instruction and if this includes a !UP marked variable, it markes
 *        the assignement as well.
 *        Same holds on the way back for the !DOWN marks.
 *
 * @param arg_node N_assign node
 * @param arg_info INFO structure
 *
 * @return N_assign node
 *******************************************************************************/
node *
WLLOMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOMassign");

    /*TravDown and therefor !UP-Part*/

    /*First traverse into the INSTR*/
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*If it is the right WL-level and the INSTR contains variables which depend
     * on an object in the right way, mark assignment as !UP*/
    if ((INFO_WLLEVEL (arg_info) == 1) && (INFO_MARK_NUP (arg_info) == TRUE)) {
        ASSIGN_NUP (arg_node) = TRUE;
        DBUG_PRINT ("WLLOM",
                    ("!!! Marked %s=... entirely (!UP)", ASSIGN_NAME (arg_node)));

        INFO_MARK_NUP (arg_info) = FALSE;
    }

    /*Continue with the next assignment*/
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_WLLEVEL (arg_info) == 1) {
        DBUG_PRINT ("WLLOM", ("--- END OF !UP START !DOWN ---"));
        INFO_WB (arg_info) = TRUE;
    }

    /*TravUp and therefor !DOWN-Part*/

    /*First traverse into the INSTR*/
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*If it is the right WL-level and the INSTR contains variables which depend
     * on an object in the right way, mark assignment as !DOWN*/
    if ((INFO_WLLEVEL (arg_info) == 1) && (INFO_MARK_NDOWN (arg_info) == TRUE)) {
        ASSIGN_NDOWN (arg_node) = TRUE;
        DBUG_PRINT ("WLLOM",
                    ("!!! Marked %s=... entirely (!DOWN)", ASSIGN_NAME (arg_node)));

        INFO_MARK_NDOWN (arg_info) = FALSE;
    }
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief This function is twofold. If it traverses down the assignment chain
 *        it traversals down the RHS. If the RHS includes an variable
 *        which is marked as not beeing allowd to be moved upon the lock, all
 *        variables on the LHS are marked to be not allowed to be moved upon
 *        this lock.
 *        Additional if this is the last assignment in the assignment-Chain,
 *        this circumstance is marked within the INFO-structure.
 *        If the Traversal is on its way back it traverses into the LHS. If the
 *        LHS contains any variable which is not allowed to be moved beneath
 *        the unlock all variables in the RHS are marked as well.
 * @param arg_node N_let node
 * @param arg_info INFO structure
 *
 * @return unchanged N_let node
 *******************************************************************************/
node *
WLLOMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOlet");
    DBUG_PRINT ("WLLOM", ("LET_NAME: %s", LET_NAME (arg_node)));

    if (INFO_WB (arg_info) == FALSE) { /*On its way down...*/
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /*If EXPR contains variables which depend on an object in the appropriate
         * way, set flag NUP and mark the LHS*/
        if ((INFO_WLLEVEL (arg_info) == 1) && (INFO_FV (arg_info) == TRUE)) {
            INFO_MARK_NUP (arg_info) = TRUE;

            DBUG_PRINT ("WLLOM", ("??? Mark %s=... (!UP)", LET_NAME (arg_node)));
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
            DBUG_PRINT ("WLLOM", ("Marked IDS..."));

            INFO_FV (arg_info) = FALSE;
        }
    } else { /*WB == TRUE*/
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

        /*If IDS contains variables which depend on an object in the appropriate
         * way, set flag NDOWN and mark the RHS*/
        if (INFO_WLLEVEL (arg_info) == 1) {
            if (INFO_FV (arg_info) == TRUE) {
                INFO_MARK_NDOWN (arg_info) = TRUE;

                DBUG_PRINT ("WLLOM", ("??? Mark %s=... (!DOWN)", LET_NAME (arg_node)));
            }

            /*Even if  the flag is not set you have to traverse further because
             * the EXPR could be a prop_obj_out*/
            LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

            if (INFO_FV (arg_info) == TRUE) {
                DBUG_PRINT ("WLLOM", ("Marked EXPR..."));

                INFO_FV (arg_info) = FALSE;
            }
        }
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

    DBUG_PRINT ("WLLOM", (">>>Enter WL-Level %i...", INFO_WLLEVEL (arg_info)));

    if (INFO_WLLEVEL (arg_info) == 1) {
        INFO_WB (arg_info) = FALSE;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_PRINT ("WLLOM", ("<<<Leave WL-Level %i...", INFO_WLLEVEL (arg_info)));
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
