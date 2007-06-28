#include "wl_lock_optimization_shifting.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "dbug.h"
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
    bool wb;
    node *before_lock;
    node *behind_unlock;
    node *last_assign;
    bool insert_chain_bl;
};

/**
 ** INFO macros
 **/
#define INFO_WLLEVEL(n) ((n)->wllevel)
#define INFO_IS_PROP_OBJ(n) ((n)->is_prop_obj)
#define INFO_IS_PROP_OBJ_IN(n) ((n)->is_prop_obj_in)
#define INFO_IS_PROP_OBJ_OUT(n) ((n)->is_prop_obj_out)
#define INFO_WB(n) ((n)->wb)
#define INFO_BEFORE_LOCK(n) ((n)->before_lock)
#define INFO_BEHIND_UNLOCK(n) ((n)->behind_unlock)
#define INFO_LAST_ASSIGN(n) ((n)->last_assign)
#define INFO_INSERT_CHAIN_BL(n) ((n)->insert_chain_bl)

/**
 ** INFO functions
 **/
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WLLEVEL (result) = 0;
    INFO_IS_PROP_OBJ (result) = FALSE;
    INFO_IS_PROP_OBJ_IN (result) = FALSE;
    INFO_IS_PROP_OBJ_OUT (result) = FALSE;
    INFO_WB (result) = FALSE;
    INFO_BEFORE_LOCK (result) = NULL;
    INFO_BEHIND_UNLOCK (result) = NULL;
    INFO_LAST_ASSIGN (result) = NULL;

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
 * @brief This function tests if the prf is a prop_obj_in or a prop_obj_out.
 *
 * @param arg_node N_prf node
 * @param arg_info INFO structure
 *
 * @return unchanged N_prf node
 *******************************************************************************/
node *
WLLOSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSprf");

    if (INFO_WLLEVEL (arg_info) == 1) {
        /*Assignment asks if prop_obj_?*/
        if (INFO_IS_PROP_OBJ (arg_info) == TRUE) {
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
 * @brief This function is twofold. If it is on its way down it sorts the
 *        assignments which can be moved up in one assignment chain and the
 *        assignments which can be moved down (and cant be moved up) into a
 *        second assignment chain.
 *        If the traversal is on its way back, it inserts the second chain
 *        behind the unlock and the first chain before the lock. The insertion
 *        before the lock is done directly if the node before is an assignment,
 *        otherwise indirectly by setting a flag.
 *
 * @param arg_node N_assign node
 * @param arg_info INFO structure
 *
 * @return N_assign node
 *******************************************************************************/
node *
WLLOSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSassign");

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

            INFO_BEHIND_UNLOCK (arg_info)
              = TCappendAssign (INFO_BEHIND_UNLOCK (arg_info), ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = INFO_BEHIND_UNLOCK (arg_info);
            INFO_BEHIND_UNLOCK (arg_info) = NULL;

            INFO_IS_PROP_OBJ_OUT (arg_info) = FALSE;
        } else { /*Some assign, no prop_obj_out*/
            /*keep prop_obj_in so we dont have to traverse into all the lets and
             *prf again at the end*/
            bool old_answer = INFO_IS_PROP_OBJ_IN (arg_info);
            INFO_IS_PROP_OBJ_IN (arg_info) = FALSE;

            if (ASSIGN_NUP (arg_node) == FALSE) { /*Assign can be moved up*/
                DBUG_PRINT ("WLLOS", ("^^^Insert Assignment %s in above-chain",
                                      ASSIGN_NAME (arg_node)));
                if (INFO_LAST_ASSIGN (arg_info) != NULL) {
                    ASSIGN_NEXT (INFO_LAST_ASSIGN (arg_info)) = ASSIGN_NEXT (arg_node);
                }
                INFO_BEFORE_LOCK (arg_info)
                  = TCappendAssign (INFO_BEFORE_LOCK (arg_info), arg_node);
                DBUG_PRINT ("WLLOS", ("Assignment inserted..."));
                if (ASSIGN_NEXT (arg_node) != NULL) {
                    ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
                }
                ASSIGN_NEXT (arg_node) = NULL;
            } else if (ASSIGN_NDOWN (arg_node) == FALSE) { /*Assign can be moved down*/
                DBUG_PRINT ("WLLOS", ("vvvInsert Assignment %s in above-chain",
                                      ASSIGN_NAME (arg_node)));
                if (INFO_LAST_ASSIGN (arg_info) != NULL) {
                    ASSIGN_NEXT (INFO_LAST_ASSIGN (arg_info)) = ASSIGN_NEXT (arg_node);
                }
                INFO_BEHIND_UNLOCK (arg_info)
                  = TCappendAssign (INFO_BEHIND_UNLOCK (arg_info), arg_node);
                DBUG_PRINT ("WLLOS", ("Assignment inserted..."));
                if (ASSIGN_NEXT (arg_node) != NULL) {
                    ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
                }
                ASSIGN_NEXT (arg_node) = NULL;
            } else { /*Assignment cant be moved up nor down*/
                INFO_LAST_ASSIGN (arg_info) = arg_node;
                ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
            }
            /*Here starts the backwards traversal*/
            INFO_IS_PROP_OBJ_IN (arg_info) = old_answer;
        }

        /*If we are in prop_obj_in, append it and insert it ohne step above*/
        if (INFO_IS_PROP_OBJ_IN (arg_info) == TRUE) {
            INFO_BEFORE_LOCK (arg_info)
              = TCappendAssign (INFO_BEFORE_LOCK (arg_info), arg_node);
            INFO_INSERT_CHAIN_BL (arg_info) = TRUE;
            INFO_IS_PROP_OBJ_IN (arg_info) = FALSE;
        } /*Else if we are in the assignment right above the lock, insert chain*/
        else if (INFO_INSERT_CHAIN_BL (arg_info) == TRUE) {
            ASSIGN_NEXT (arg_node) = INFO_BEFORE_LOCK (arg_info);
            INFO_BEFORE_LOCK (arg_info) = NULL;
            INFO_INSERT_CHAIN_BL (arg_info) = FALSE;
        }
    } else if (INFO_WLLEVEL (arg_info) == 0) {
        DBUG_PRINT ("WLLOS", ("Do TRAVcont..."));
        DBUG_ASSERT (ASSIGN_INSTR (arg_node) != NULL, "THERE SHOULD BE AN ASSIGN_INSTR!");
        arg_node = TRAVcont (arg_node, arg_info);
        DBUG_PRINT ("WLLOS", ("Return from TRAVcont..."));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief This function inserts the assignment-chain which contains the
 *        assignments which can be moved before the lock if it is the first
 *        node before the lock and if we are on the way back.
 *
 * @param arg_node N_block node
 * @param arg_info INFO structure
 *
 * @return N_block node
 *******************************************************************************/
node *
WLLOSblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSblock");

    if (INFO_WLLEVEL (arg_info) == 1) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

        if (INFO_INSERT_CHAIN_BL (arg_info) == TRUE) {
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
 * @brief this function just increases the with-loop level counter in the
 *        INFO-structure when entering this node, continues traversing if on
 *        the right wl-level and decreases wl level counter afterwards.
 *
 * @param arg_node N_with node
 * @param arg_info INFO structure
 *
 * @return unchanged N_with node
 *******************************************************************************/
node *
WLLOSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLLOSwith");

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) + 1;

    if (INFO_WLLEVEL (arg_info) == 1) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    INFO_WLLEVEL (arg_info) = INFO_WLLEVEL (arg_info) - 1;
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the shifting of the up or down movable
 *        assignments
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 ********************************************************************************/

node *
WLLOSdoLockOptimizationShifting (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLLOSdoLockOptimizationShifting");

    DBUG_PRINT ("WLLOS", ("START TRAVERSAL"));

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "WLLOSdoLockOptimizationShifting is intended to run on the entire tree");

    info = MakeInfo ();
    TRAVpush (TR_wllos);
    DBUG_PRINT ("WLLOS", ("Enter Traversal"));
    syntax_tree = TRAVdo (syntax_tree, info);
    DBUG_PRINT ("WLLOS", ("Left Traversal"));

    TRAVpop ();

    info = FreeInfo (info);
    DBUG_PRINT ("WLLOS", ("TRAVERSAL DONE"));

    DBUG_RETURN (syntax_tree);
}
