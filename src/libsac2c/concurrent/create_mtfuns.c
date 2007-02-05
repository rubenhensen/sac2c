/*****************************************************************************
 *
 * $Id:$
 *
 * file:   create_mtfuns.c
 *
 * prefix: CMTF
 *
 * description:
 *
 *   This file implements a program traversal that creates MT-variants of all
 *   functions executed in parallel by multiple threads.
 *
 *   The traversal uses the following rules:
 *     - Start traversing at an exported or provided fundef, with the ST
 *       marker.
 *     - When we encounter a function application:
 *        - If it is not marked, mark it with the current marker. Traverse
 *          into the function recursively.
 *        - If it is marked with our current marker, skip.
 *        - If it is marked with the other marker, create a copy of the
 *          fundef and mark that one with our marker. Replace application with
 *          application to this function. Traverse into the new function.
 *     - When we encounter an spmd block:
 *        - If the current marker is ST, set the current marker to MT and
 *          enter the block. Reset marker to ST afterwards.
 *        - If the current marker is MT, discard the spmd block. We don't
 *          allow nesting spmd blocks.
 *
 *    This algorithm repeats until all functions are marked and thus a fixed
 *    point is reached. Tagged functions are put in the MT subnamespace.
 *
 *****************************************************************************/

#include <string.h>

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "globals.h"

#include "create_mtfuns.h"

/**
 * INFO structure
 */

struct INFO {
    bool mtcontext;
    bool ismodule;
    bool onspine;
    node *companions;
    node *spmdassigns;
};

/**
 * INFO macros
 */

#define INFO_MTCONTEXT(n) (n->mtcontext)
#define INFO_ISMODULE(n) (n->ismodule)
#define INFO_ONSPINE(n) (n->onspine)
#define INFO_COMPANIONS(n) (n->companions)
#define INFO_SPMDASSIGNS(n) (n->spmdassigns)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_MTCONTEXT (result) = FALSE;
    INFO_ISMODULE (result) = FALSE;
    INFO_ONSPINE (result) = FALSE;
    INFO_COMPANIONS (result) = NULL;
    INFO_SPMDASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#if 0
static bool IsSpmdConditional( node *arg_node)
{
  bool res;
  node *prf;
  
  DBUG_ENTER("IsSpmdConditional");
  
  DBUG_ASSERT( NODE_TYPE( arg_node) == N_cond,
               "IsSpmdConditional() applied to wrong node type.");
  
  res = FALSE;
  
  if ( NODE_TYPE( COND_COND( arg_node)) == N_prf) {
    prf = COND_COND( arg_node);
    if ((PRF_PRF( prf) == F_run_mt_genarray)
        || (PRF_PRF( prf) == F_run_mt_modarray)
        || (PRF_PRF( prf) == F_run_mt_fold)) {
      res = TRUE;
    }
  }
  
  DBUG_RETURN( res);
}
#endif

/******************************************************************************
 *
 * @fn node *CMTFdoCreateMtFuns( node *syntax_tree)
 *
 *  @brief initiates CMTF traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
CMTFdoCreateMtFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CMTFdoCreateMtFuns");

    if ((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop)) {
        DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

        info = MakeInfo ();

        TRAVpush (TR_cmtf);
        syntax_tree = TRAVdo (syntax_tree, info);
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * @fn static node *MakeCompanion( node *fundef)
 *
 *  @brief Duplicates the given function and turns the duplicated into MT.
 *
 *  @param fundef
 *
 *  @return fundef
 *
 *****************************************************************************/

static node *
MakeCompanion (node *fundef)
{
    node *companion;

    DBUG_ENTER ("MakeCompanion");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "MakeMtFun called with non N_fundef argument node");

    DBUG_ASSERT (FUNDEF_ISMTFUN (fundef) || FUNDEF_ISSTFUN (fundef),
                 "Function to be duplicated into companion is neither ST nor MT.");

    companion = DUPdoDupNode (fundef);
    FUNDEF_COMPANION (fundef) = companion;
    FUNDEF_COMPANION (companion) = fundef;

    FUNDEF_ISMTFUN (companion) = !FUNDEF_ISMTFUN (fundef);
    FUNDEF_ISSTFUN (companion) = !FUNDEF_ISSTFUN (fundef);

    DBUG_RETURN (companion);
}

/******************************************************************************
 *
 * @fn node *HandleApFold( node *arg_node, info *arg_info)
 *
 *  @brief Joint traversal function for N_ap and N_fold nodes.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

static node *
HandleApFold (node *callee, info *arg_info)
{
    DBUG_ENTER ("HandleApFold");

    if (FUNDEF_ISMTFUN (callee) || FUNDEF_ISSTFUN (callee)) {
        /*
         * This function has already been processed before.
         */
        if ((FUNDEF_ISMTFUN (callee) && INFO_MTCONTEXT (arg_info))
            || (FUNDEF_ISSTFUN (callee) && !INFO_MTCONTEXT (arg_info))) {
            /*
             * The called function is already in the right mode.
             * Hence, we do nothing.
             */
        } else {
            /*
             * The called function is in the wrong mode.
             */
            if (FUNDEF_COMPANION (callee) == NULL) {
                /*
                 * There is no companion yet, so we must create one,
                 * exchange the callee with its companion, traverse
                 * the new companion to recursively bring it into the right mode,
                 * and eventually append it to the list of companions.
                 */

                callee = TRAVdo (MakeCompanion (callee), arg_info);

                FUNDEF_NEXT (callee) = INFO_COMPANIONS (arg_info);
                INFO_COMPANIONS (arg_info) = callee;
            } else {
                /*
                 * The companion already exists. So, we simply exchange the called
                 * function.
                 */
                callee = FUNDEF_COMPANION (callee);
            }
        }
    } else {
        /*
         * This function has not yet been processed at all.
         * We turn it into the right mode and continue traversal in the callee.
         */

        FUNDEF_ISMTFUN (callee) = INFO_MTCONTEXT (arg_info);
        FUNDEF_ISSTFUN (callee) = !INFO_MTCONTEXT (arg_info);

        callee = TRAVdo (callee, arg_info);
    }

    DBUG_RETURN (callee);
}

/******************************************************************************
 *
 * @fn node *CMTFmodule( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_module node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFmodule");

    if ((MODULE_FILETYPE (arg_node) == F_modimp)
        || (MODULE_FILETYPE (arg_node) == F_classimp)) {
        INFO_ISMODULE (arg_info) = TRUE;
    }

    INFO_ONSPINE (arg_info) = TRUE;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFfundef( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_fundef node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFfundef (node *arg_node, info *arg_info)
{
    node *companion;

    DBUG_ENTER ("CMTFfundef");

    if (INFO_ONSPINE (arg_info)) {

        if (FUNDEF_ISEXPORTED (arg_node) || FUNDEF_ISPROVIDED (arg_node)) {

            if (!FUNDEF_ISMTFUN (arg_node) && !FUNDEF_ISSTFUN (arg_node)) {
                /*
                 * The current function is provided/exported, but has not been
                 * processed yet. We make this one an ST fun and traverse the body.
                 */

                FUNDEF_ISSTFUN (arg_node) = TRUE;

                INFO_MTCONTEXT (arg_info) = FALSE;
                INFO_ONSPINE (arg_info) = FALSE;
                arg_node = TRAVdo (arg_node, arg_info);
                INFO_ONSPINE (arg_info) = TRUE;
            }

            if ((FUNDEF_COMPANION (arg_node) == NULL) && INFO_ISMODULE (arg_info)) {
                /*
                 * The current function is provided/exported and has no companion
                 * yet. If we are in a module/class implementation we want to create
                 * both ST and MT versions of each provided/exported function.
                 * If we are in a program, this must be the main function, which is
                 * always ST and we do not need to create a companion.
                 */

                companion = MakeCompanion (arg_node);

                INFO_ONSPINE (arg_info) = FALSE;
                companion = TRAVdo (companion, arg_info);
                INFO_ONSPINE (arg_info) = TRUE;

                FUNDEF_NEXT (companion) = INFO_COMPANIONS (arg_info);
                INFO_COMPANIONS (arg_info) = companion;
            }
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            /*
             * We are at the end of the spine, so we append the companions
             * collected in the info structure.
             */
            FUNDEF_NEXT (arg_node) = INFO_COMPANIONS (arg_info);
            INFO_COMPANIONS (arg_info) = NULL;
        }

        if (!FUNDEF_ISMTFUN (arg_node) && !FUNDEF_ISSTFUN (arg_node)) {
            FUNDEF_ISSTFUN (arg_node) = TRUE;
            /*
             * We have now processed all functions. If functions are left that are
             * neither tagged ST nor MT, they are neither exported/provided nor used
             * internally. They should have been removed by DFR, but maybe DFR was
             * disabled. To continue compilation in an ordered manner, we tag these
             * functions ST.
             */
        }

        /*
         * On the traversal back up the tree, put all functions marked MT into
         * the MT namespace.
         */
        if (FUNDEF_ISMTFUN (arg_node)) {
            namespace_t *old_namespace = FUNDEF_NS (arg_node);
            FUNDEF_NS (arg_node) = NSgetMTNamespace (old_namespace);
            old_namespace = NSfreeNamespace (old_namespace);
        }
    } else {
        /*
         * We are *not* at the spine of the fundef chain.
         */
        DBUG_ASSERT (FUNDEF_ISSTFUN (arg_node) || FUNDEF_ISMTFUN (arg_node),
                     "Encountered off-spine fundef that is neither MT nor ST.");

        INFO_MTCONTEXT (arg_info) = FUNDEF_ISMTFUN (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFspmd( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_spmd node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFspmd");

#ifndef BEMT
    if (INFO_MTCONTEXT (arg_info)) {
        /*
         * We are already in an MT context. Hence, we eliminate this nested SPMD
         * block to avoid the recursive unfolding of parallelism.
         */

        if (NODE_TYPE (BLOCK_INSTR (SPMD_REGION (arg_node))) != N_empty) {
            INFO_SPMDASSIGNS (arg_info) = BLOCK_INSTR (SPMD_REGION (arg_node));
            /*
             * We store the assignment chain of the SPMD region in the info structure
             * for subsequent integration into the surrounding assignment chain.
             */

            BLOCK_INSTR (SPMD_REGION (arg_node)) = TBmakeEmpty ();
            /*
             * We must restore a correct N_spmd node for later de-allocation.
             */
        }
    } else {
        /*
         * We are not yet in a parallel context. Hence, we traverse the SPMD
         * region in MT context and the optional sequential alternative of a
         * conditional SPMD block in sequential mode. We need not to traverse
         * the condition of a conditional SPMD block because for the time being
         * it may not contain applications of defined functions.
         */

        INFO_MTCONTEXT (arg_info) = TRUE;
        SPMD_REGION (arg_node) = TRAVdo (SPMD_REGION (arg_node), arg_info);
        INFO_MTCONTEXT (arg_info) = FALSE;

        if (SPMD_SEQUENTIAL (arg_node) != NULL) {
            SPMD_SEQUENTIAL (arg_node) = TRAVdo (SPMD_SEQUENTIAL (arg_node), arg_info);
        }
    }
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFcond( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_cond node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFcond");

#ifdef BEMT
    if (IsSpmdConditional (arg_node) && INFO_MTCONTEXT (arg_info)) {
        /*
         * We are already in an MT context. Hence, we eliminate this nested SPMD
         * block to avoid the recursive unfolding of parallelism.
         */

        INFO_SPMDASSIGNS (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));
        /*
         * We store the assignment chain of the SPMD region in the info structure
         * for subsequent integration into the surrounding assignment chain.
         */

        BLOCK_INSTR (COND_ELSE (arg_node)) = TBmakeEmpty ();
        /*
         * We must restore a correct N_spmd node for later de-allocation.
         */
    } else {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFwith2( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_with2 node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFwith2");

#ifdef BEMT
    if (INFO_MTCONTEXT (arg_info)) {
        if (WITH2_MT (arg_node)) {
            WITH2_MT (arg_node) = FALSE;
        }
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    } else {
        /*
         * We are not yet in a parallel context. Hence, we traverse the SPMD
         * region in MT context and the optional sequential alternative of a
         * conditional SPMD block in sequential mode. We need not to traverse
         * the condition of a conditional SPMD block because for the time being
         * it may not contain applications of defined functions.
         */

        if (WITH2_MT (arg_node)) {
            INFO_MTCONTEXT (arg_info) = TRUE;
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
            INFO_MTCONTEXT (arg_info) = FALSE;
        } else {
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        }
    }

#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFassign( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_assign node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFassign (node *arg_node, info *arg_info)
{
    node *assign;

    DBUG_ENTER ("CMTFassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_SPMDASSIGNS (arg_info) != NULL) {
        /*
         * During traversal of ASSIGN_INSTR we have found an SPMD block which
         * needs to be eliminated.
         */
        assign = arg_node;

        arg_node = TCappendAssign (INFO_SPMDASSIGNS (arg_info), ASSIGN_NEXT (arg_node));

        ASSIGN_NEXT (assign) = NULL;
        assign = FREEdoFreeTree (assign);
        INFO_SPMDASSIGNS (arg_info) = NULL;

        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFap( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_ap node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFap");

    AP_FUNDEF (arg_node) = HandleApFold (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *CMTFfold( node *arg_node, info *arg_info)
 *
 *  @brief CMTF traversal function for N_fold node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
CMTFfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFfold");

    FOLD_FUNDEF (arg_node) = HandleApFold (FOLD_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
