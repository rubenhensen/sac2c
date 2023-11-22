/*****************************************************************************
 *
 * file:   create_mtst_funs_mdule.c
 *
 * prefix: MTSTFMOD
 *
 * description:
 *
 *   This file implements a program traversal that creates XT, MT and ST-variants of
 *   all functions in a module or class definition. Note that we deal with three
 *   dfferent classes of functions wrt multithreaded execution:
 *
 *    SEQ : These functions will definitely be executed sequentially.
 *    ST  : These functions are called in a sequential context, but may contain
 *          parallelised with-loops.
 *    MT  : These functions will be called in a replicated manner in
 *          multithreaded contexts.
 *    XT  : These functions are called in multi-threaded context and may
 *          contain parallelized with-loops.
 *
 *    We distinguish between such functions within this module through the
 *    flags FUNDEF_ISSTFUN, FUNDEF_ISMTFUN and FUNDEF_ISXTFUN and later on
 *    through the name spaces ST, MT and XT. Sequential functions stay in the
 *    standard name space.
 *
 *    We use this specialised version for modules and classes because we
 *    do not want to trace the static call graph of functions in this trivial
 *    case.
 *
 *****************************************************************************/

#include "create_mtst_funs_module.h"

#define DBUG_PREFIX "MTSTFMOD"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "globals.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"

/**
 * INFO structure
 */

typedef enum { SEQ, ST, MT, XT } context_t;

struct INFO {
    context_t context;
    node *vardecs;
    node *stcompanions;
    node *mtcompanions;
    node *xtcompanions;
    node *spmdassigns;
    node *spmdcondition;
};

/**
 * INFO macros
 */

#define INFO_CONTEXT(n) ((n)->context)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_STCOMPANIONS(n) ((n)->stcompanions)
#define INFO_MTCOMPANIONS(n) ((n)->mtcompanions)
#define INFO_XTCOMPANIONS(n) ((n)->xtcompanions)
#define INFO_SPMDASSIGNS(n) ((n)->spmdassigns)
#define INFO_SPMDCONDITION(n) ((n)->spmdcondition)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = SEQ;
    INFO_VARDECS (result) = NULL;
    INFO_STCOMPANIONS (result) = NULL;
    INFO_MTCOMPANIONS (result) = NULL;
    INFO_XTCOMPANIONS (result) = NULL;
    INFO_SPMDASSIGNS (result) = NULL;
    INFO_SPMDCONDITION (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn bool IsSpmdConditional( node *arg_node)
 *
 *  @brief checks for an argument node of type N_cond whether it is one of
 *    conditionals introduced by the cost model.
 *
 *  @param arg_node of type N_cond
 *
 *  @return predicate value
 *
 *****************************************************************************/

static bool
IsSpmdConditional (node *arg_node)
{
    bool res;
    node *prf;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond,
                 "IsSpmdConditional() applied to wrong node type.");

    res = FALSE;

    if (NODE_TYPE (COND_COND (arg_node)) == N_prf) {
        prf = COND_COND (arg_node);
        if ((PRF_PRF (prf) == F_run_mt_genarray) || (PRF_PRF (prf) == F_run_mt_modarray)
            || (PRF_PRF (prf) == F_run_mt_fold)) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
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
    DBUG_ENTER ();

    DBUG_PRINT ("handling call to %s .....", FUNDEF_NAME (callee));

    if (FUNDEF_ISMTFUN (callee) || FUNDEF_ISSTFUN (callee) || FUNDEF_ISXTFUN (callee)) {
        /*
         * we are dealing with a recursive call which has been adjusted
         * through DupTree. This needs to be reset to ensure proper handling.
         * See bug 1137 for further details.
         */
        callee = FUNDEF_COMPANION (callee);
    }

    switch (INFO_CONTEXT (arg_info)) {
    case SEQ:
        break;
    case ST:
        if (FUNDEF_COMPANION (callee) != NULL) {
            DBUG_ASSERT (FUNDEF_ISSTFUN (FUNDEF_COMPANION (callee)),
                         "ST companion of function %s is no ST function",
                         FUNDEF_NAME (callee));

            callee = FUNDEF_COMPANION (callee);
            DBUG_PRINT ("setting call to %s to mode ST", FUNDEF_NAME (callee));
        }
        break;
    case MT:
        if (FUNDEF_MTCOMPANION (callee) != NULL) {
            DBUG_ASSERT (FUNDEF_ISMTFUN (FUNDEF_MTCOMPANION (callee)),
                         "MT companion of function %s is no MT function",
                         FUNDEF_NAME (callee));

            callee = FUNDEF_MTCOMPANION (callee);
            DBUG_PRINT ("setting call to %s to mode MT", FUNDEF_NAME (callee));
        }
        break;
    case XT:
        if (FUNDEF_XTCOMPANION (callee) != NULL) {
            DBUG_ASSERT (FUNDEF_ISXTFUN (FUNDEF_XTCOMPANION (callee)),
                         "XT companion of function %s is no XT function",
                         FUNDEF_NAME (callee));

            callee = FUNDEF_XTCOMPANION (callee);
            DBUG_PRINT ("setting call to %s to mode XT", FUNDEF_NAME (callee));
        }
        break;
    }

    DBUG_RETURN (callee);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODdoCreateMtStFunsModule( node *syntax_tree)
 *
 *  @brief initiates MTSTFMOD traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTSTFMODdoCreateMtStFunsModule (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT ((MODULE_FILETYPE (syntax_tree) == FT_modimp)
                   || (MODULE_FILETYPE (syntax_tree) == FT_classimp),
                 "MTSTFMODdoCreateMtStFunsModule() not applicable to programs");

    info = MakeInfo ();

    TRAVpush (TR_mtstfmod);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODmodule( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_module node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODfundef( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_fundef node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODfundef (node *arg_node, info *arg_info)
{
    node *companion;
    namespace_t *old_namespace;

    DBUG_ENTER ();

    if (!FUNDEF_ISMTFUN (arg_node) && !FUNDEF_ISSTFUN (arg_node)
        && !FUNDEF_ISXTFUN (arg_node) && !FUNDEF_ISEXTERN (arg_node)) {

        /*
         * Create the ST companion:
         */

        companion = DUPdoDupNode (arg_node);

        FUNDEF_ISSTFUN (companion) = TRUE;

        old_namespace = FUNDEF_NS (companion);
        FUNDEF_NS (companion) = NSgetSTNamespace (old_namespace);
        old_namespace = NSfreeNamespace (old_namespace);

        FUNDEF_COMPANION (arg_node) = companion;
        FUNDEF_COMPANION (companion) = arg_node;

        FUNDEF_NEXT (companion) = INFO_STCOMPANIONS (arg_info);
        INFO_STCOMPANIONS (arg_info) = companion;

        /*
         * Create the MT companion:
         */

        companion = DUPdoDupNode (arg_node);

        FUNDEF_ISMTFUN (companion) = TRUE;

        old_namespace = FUNDEF_NS (companion);
        FUNDEF_NS (companion) = NSgetMTNamespace (old_namespace);
        old_namespace = NSfreeNamespace (old_namespace);

        FUNDEF_MTCOMPANION (arg_node) = companion;
        FUNDEF_COMPANION (companion) = arg_node;

        FUNDEF_NEXT (companion) = INFO_MTCOMPANIONS (arg_info);
        INFO_MTCOMPANIONS (arg_info) = companion;

        /*
         * Create the XT companion:
         */

        companion = DUPdoDupNode (arg_node);

        FUNDEF_ISXTFUN (companion) = TRUE;

        old_namespace = FUNDEF_NS (companion);
        FUNDEF_NS (companion) = NSgetXTNamespace (old_namespace);
        old_namespace = NSfreeNamespace (old_namespace);

        FUNDEF_XTCOMPANION (arg_node) = companion;
        FUNDEF_COMPANION (companion) = arg_node;

        FUNDEF_NEXT (companion) = INFO_XTCOMPANIONS (arg_info);
        INFO_XTCOMPANIONS (arg_info) = companion;
    }

    /*
     * Recursively traverse through fundef chain.
     * Append ST, MT, XT companions from info structure as created so far.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        FUNDEF_NEXT (arg_node) = INFO_STCOMPANIONS (arg_info);
        INFO_STCOMPANIONS (arg_info) = NULL;

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = INFO_MTCOMPANIONS (arg_info);
            INFO_MTCOMPANIONS (arg_info) = NULL;

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            } else {
                FUNDEF_NEXT (arg_node) = INFO_XTCOMPANIONS (arg_info);
                INFO_XTCOMPANIONS (arg_info) = NULL;

                FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
            }
        }
    }

    /*
     * On the way up the fundef chain, we traverse into the function bodies
     * in order to adapt the AP links.
     */

    DBUG_PRINT ("traversing body of function %s", FUNDEF_NAME (arg_node));

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_ISMTFUN (arg_node)) {
            INFO_CONTEXT (arg_info) = MT;
            DBUG_PRINT ("context MT");
        } else if (FUNDEF_ISSTFUN (arg_node)) {
            INFO_CONTEXT (arg_info) = ST;
            DBUG_PRINT ("context ST");
        } else if (FUNDEF_ISXTFUN (arg_node)) {
            INFO_CONTEXT (arg_info) = XT;
            DBUG_PRINT ("context XT");
        } else {
            INFO_CONTEXT (arg_info) = SEQ;
            DBUG_PRINT ("context SEQ");
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("finished traversing body of function %s", FUNDEF_NAME (arg_node));

        FUNDEF_VARDECS (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODcond( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_cond node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (IsSpmdConditional (arg_node)) {
        if ((INFO_CONTEXT (arg_info) == SEQ) || (INFO_CONTEXT (arg_info) == MT)) {
            /*
             * We are either in an MT or SEQ context and this is a special conditional
             * introduced by the cost model to postpone the parallelisation decision
             * until runtime. We can now decide that we do *not* want to parallelise
             * the associated with-loop in the current context.
             */

            INFO_SPMDASSIGNS (arg_info) = BLOCK_ASSIGNS (COND_ELSE (arg_node));
            /*
             * We store the assignment chain of the else-case (sequential)
             * for subsequent integration into the surrounding assignment chain.
             */

            BLOCK_ASSIGNS (COND_ELSE (arg_node)) = NULL;
            /*
             * We must restore a correct conditional node for later de-allocation.
             */
        } else {
            /*
             * The conditional will remain, so we now lift the condition out into a
             * separate assignment.
             */
            node *new_avis;

            new_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                             SHmakeShape (0)));

            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_SPMDCONDITION (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                         COND_COND (arg_node)),
                              NULL);
            COND_COND (arg_node) = TBmakeId (new_avis);
        }
    } else {
        /*
         * Normal conditionals are just traversed as usual.
         */
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODwith2( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_with2 node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("traversing with-loop");
    if (INFO_CONTEXT (arg_info) == ST || INFO_CONTEXT (arg_info) == XT) {
        /*
         * We are not yet in a parallel context. Hence, we traverse the SPMD
         * region in MT context and the optional sequential alternative of a
         * conditional SPMD block in sequential mode. We need not to traverse
         * the condition of a conditional SPMD block because for the time being
         * it may not contain applications of defined functions.
         */

        if (WITH2_PARALLELIZE (arg_node)) {
            context_t ctx = INFO_CONTEXT (arg_info);
            INFO_CONTEXT (arg_info) = MT;
            DBUG_PRINT ("switching to context MT");
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
            DBUG_PRINT ("switching back to previous context");
            INFO_CONTEXT (arg_info) = ctx;
        } else {
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        }
    } else {
        /*
         * We are either in an MT or an SEQ context. In either case we do not
         * want to parallelise this with-loop.
         */

        if (WITH2_PARALLELIZE (arg_node)) {
            WITH2_PARALLELIZE (arg_node) = FALSE;
        }
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODassign( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_assign node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SPMDASSIGNS (arg_info) != NULL) {
        /*
         * During traversal of ASSIGN_STMT we have found a special conditional
         * introduced by the cost model that needs to be eliminated now.
         */
        node *further_assigns;

        further_assigns = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeTree (arg_node);

        arg_node = TCappendAssign (INFO_SPMDASSIGNS (arg_info), further_assigns);
        INFO_SPMDASSIGNS (arg_info) = NULL;

        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        if (INFO_SPMDCONDITION (arg_info) != NULL) {
            /*
             * During traversal of ASSIGN_STMT we have found a SPMD conditional
             * whose condition is now being flattened.
             */
            node *preassign;

            preassign = INFO_SPMDCONDITION (arg_info);
            INFO_SPMDCONDITION (arg_info) = NULL;
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            ASSIGN_NEXT (preassign) = arg_node;
            arg_node = preassign;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODap( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_ap node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AP_FUNDEF (arg_node) = HandleApFold (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFMODfold( node *arg_node, info *arg_info)
 *
 *  @brief MTSTFMOD traversal function for N_fold node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFMODfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_FUNDEF (arg_node) = HandleApFold (FOLD_FUNDEF (arg_node), arg_info);

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
