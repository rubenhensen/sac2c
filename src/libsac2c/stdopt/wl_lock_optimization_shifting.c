/* $id$ */

#include "wl_lock_optimization_shifting.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"

#define DBUG_PREFIX "WLLOS"
#include "debug.h"

#include "traverse.h"
#include "DupTree.h"
#include "free.h"

/**
 *
 * @wl_lock_optimization_shifting.c
 *
 * @brief The traversels in this file shift the assignements which are
 *        allowed to be moved above the lock or beneath the unlock.
 *
 */

/**
 ** INFO structure
 **/

struct INFO {
    int wllevel;
    bool is_prop_obj;
    bool is_prop_obj_in;
    bool is_prop_obj_out;
    bool found_lock;
    bool wb;
    node *before_lock;
    node *behind_unlock;
    bool insert_chain_bl;
};

/**
 ** INFO macros
 **/

#define INFO_WLLEVEL(n) ((n)->wllevel)
#define INFO_IS_PROP_OBJ(n) ((n)->is_prop_obj)
#define INFO_IS_PROP_OBJ_IN(n) ((n)->is_prop_obj_in)
#define INFO_IS_PROP_OBJ_OUT(n) ((n)->is_prop_obj_out)
#define INFO_FOUND_LOCK(n) ((n)->found_lock)
#define INFO_WB(n) ((n)->wb)
#define INFO_BEFORE_LOCK(n) ((n)->before_lock)
#define INFO_BEHIND_UNLOCK(n) ((n)->behind_unlock)
#define INFO_INSERT_CHAIN_BL(n) ((n)->insert_chain_bl)

/**
 ** INFO functions
 **/

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_WLLEVEL (result) = 0;
    INFO_IS_PROP_OBJ (result) = FALSE;
    INFO_IS_PROP_OBJ_IN (result) = FALSE;
    INFO_IS_PROP_OBJ_OUT (result) = FALSE;
    INFO_FOUND_LOCK (result) = FALSE;
    INFO_WB (result) = FALSE;
    INFO_BEFORE_LOCK (result) = NULL;
    INFO_BEHIND_UNLOCK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *WLLOSprf(node *arg_node, info *arg_info)
 *
 *    @brief This function tests if the prf is a prop_obj_in or a prop_obj_out.
 *
 *    @param arg_node N_prf node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_prf node
 *****************************************************************************/

node *
WLLOSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_WLLEVEL (arg_info) == 1) {
        /*Assignment asks if prop_obj_?*/
        if (INFO_IS_PROP_OBJ (arg_info)) {
            if (PRF_PRF (arg_node) == F_prop_obj_in) {
                INFO_IS_PROP_OBJ_IN (arg_info) = TRUE;
            } else if (PRF_PRF (arg_node) == F_prop_obj_out) {
                INFO_IS_PROP_OBJ_OUT (arg_info) = TRUE;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *WLLOSassign(node *arg_node, info *arg_info)
 *
 *    @brief This function is twofold. If it is on its way down it sorts the
 *           assignments which can be moved up in one assignment chain and the
 *           assignments which can be moved down (and cant be moved up) into a
 *           second assignment chain.
 *           If the traversal is on its way back, it inserts the second chain
 *           behind the unlock and the first chain before the lock. The
 *           insertion before the lock is done directly if the node before is
 *           an assignment, otherwise indirectly by setting a flag.
 *
 *    @param arg_node N_assign node
 *    @param arg_info INFO structure
 *
 *    @return N_assign node
 *****************************************************************************/

node *
WLLOSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*If I shift the current node, I have to return the next and if I shift
     * the current node I have to set the ->next to NULL, so I have to keep
     * the next somewhere*/
    node *ret_node = arg_node;
    node *next_node = ASSIGN_NEXT (arg_node);

    if (INFO_WLLEVEL (arg_info) == 1) {
        /*ask the question if assign is a prop_obj_?*/
        INFO_IS_PROP_OBJ (arg_info) = TRUE;

        /*Traverse into the INSTR to figure out if it is a prop_obj_?*/
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        INFO_IS_PROP_OBJ (arg_info) = FALSE;

        /*If prop_obj_out is reached, we are at the end, so we start to traverse
         * backwards. Also we are at the right point to insert the assign chain
         * of "moved down"-assignments.*/
        if (INFO_IS_PROP_OBJ_OUT (arg_info) == TRUE) {
            INFO_WB (arg_info) = TRUE;

            DBUG_PRINT ("Insert BEHIND-CHAIN");

            INFO_BEHIND_UNLOCK (arg_info)
              = TCappendAssign (INFO_BEHIND_UNLOCK (arg_info), ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = INFO_BEHIND_UNLOCK (arg_info);
            INFO_BEHIND_UNLOCK (arg_info) = NULL;

            INFO_IS_PROP_OBJ_OUT (arg_info) = FALSE;
        } else { /*Some assign, no prop_obj_out*/
            bool old_answer = FALSE;
            if (INFO_IS_PROP_OBJ_IN (arg_info) == TRUE) {

                /* If a prop_obj_in is found, there is a lock and therefore there
                 * are maybe assignments to shift. You also could choose to test
                 * wether there is a propagate() or not but if somewhen the locks
                 * will be splitted from the prop_obj it will be easier to change
                 * this way*/
                INFO_FOUND_LOCK (arg_info) = TRUE;

                /* keep that this is prop_obj_in for the insertion of the chain
                 * at the backtraversal part
                 **/
                old_answer = INFO_IS_PROP_OBJ_IN (arg_info);
                INFO_IS_PROP_OBJ_IN (arg_info) = FALSE;

                DBUG_ASSERT (ASSIGN_NEXT (arg_node) != NULL,
                             "There should be at least a prop_obj_out!");
                ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
            } else if ((INFO_FOUND_LOCK (arg_info) == TRUE)
                       && ((ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (arg_node) == FALSE)
                           || (ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (arg_node) == FALSE))) {

                next_node = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = NULL;

                if (ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (arg_node) == FALSE) {
                    DBUG_PRINT ("^^^Insert %s", ASSIGN_NAME (arg_node));
                    INFO_BEFORE_LOCK (arg_info)
                      = TCappendAssign (INFO_BEFORE_LOCK (arg_info), arg_node);
                } else {
                    DBUG_PRINT ("vvvInsert %s", ASSIGN_NAME (arg_node));
                    INFO_BEHIND_UNLOCK (arg_info)
                      = TCappendAssign (INFO_BEHIND_UNLOCK (arg_info), arg_node);
                }

                ret_node = TRAVdo (next_node, arg_info);
            } else { /*Assignment cant be moved up nor down*/
                ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            }
            /*Here starts the backwards traversal*/
            INFO_IS_PROP_OBJ_IN (arg_info) = old_answer;
        }

        /*If we are in prop_obj_in, append it and insert it one step above*/
        if (INFO_IS_PROP_OBJ_IN (arg_info) == TRUE) {
            INFO_BEFORE_LOCK (arg_info)
              = TCappendAssign (INFO_BEFORE_LOCK (arg_info), arg_node);
            INFO_INSERT_CHAIN_BL (arg_info) = TRUE;
            INFO_IS_PROP_OBJ_IN (arg_info) = FALSE;
            INFO_FOUND_LOCK (arg_info) = FALSE;
        } /*Else if we are in the assignment right above the lock, insert chain*/
        else if (INFO_INSERT_CHAIN_BL (arg_info) == TRUE) {
            DBUG_PRINT ("Insert ABOVE-Chain (ASSIGN)");
            ASSIGN_NEXT (arg_node) = INFO_BEFORE_LOCK (arg_info);
            INFO_BEFORE_LOCK (arg_info) = NULL;
            INFO_INSERT_CHAIN_BL (arg_info) = FALSE;
        }
    } else if (INFO_WLLEVEL (arg_info) == 0) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (ret_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *WLLOSblock(node *arg_node, info arg_info)
 *
 *    @brief This function inserts the assignment-chain which contains the
 *           assignments which can be moved before the lock if it is the first
 *           node before the lock and if we are on the way back.
 *
 *    @param arg_node N_block node
 *    @param arg_info INFO structure
 *
 *    @return N_block node
 *****************************************************************************/

node *
WLLOSblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_WLLEVEL (arg_info) == 1) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

        if (INFO_INSERT_CHAIN_BL (arg_info) == TRUE) {
            DBUG_PRINT ("Insert ABOVE-Chain (BLOCK)");
            BLOCK_INSTR (arg_node) = INFO_BEFORE_LOCK (arg_info);
            INFO_BEFORE_LOCK (arg_info) = NULL;
            INFO_INSERT_CHAIN_BL (arg_info) = FALSE;
        }
    } else if (INFO_WLLEVEL (arg_info) == 0) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *WLLOSwith(node *arg_node, info *arg_info)
 *
 *    @brief this function just increases the with-loop level counter in the
 *           INFO-structure when entering this node, continues traversing if on
 *           the right wl-level and decreases wl level counter afterwards.
 *
 *    @param arg_node N_with node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_with node
 *****************************************************************************/

node *
WLLOSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) + 1;

    if (INFO_WLLEVEL (arg_info) == 1) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) - 1;
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *WLLOSdoLockOptimizationShifting(node *syntax_tree)
 *
 *    @brief function triggering the shifting of the up or down movable
 *           assignments
 *
 *    @param syntax_tree N_module node
 *
 *    @return transformed syntax tree
 *****************************************************************************/

node *
WLLOSdoLockOptimizationShifting (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module,
                 "WLLOSdoLockOptimizationShifting is intended to run on the entire tree");

    info = MakeInfo ();
    TRAVpush (TR_wllos);
    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
