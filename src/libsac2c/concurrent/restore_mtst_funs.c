/*****************************************************************************
 *
 * $Id$
 *
 * file:   restore_mtst_funs.c
 *
 * prefix: RMTSTF
 *
 * description:
 *
 *   This file implements a program traversal that creates MT and ST-variants
 *   of all functions in a module or class definition. Note that we deal with
 *   three dfferent classes of functions wrt multithreaded execution:
 *
 *    SEQ : These functions will definitely be executed sequentially.
 *    ST  : These functions are called in a sequential context, but may contain
 *          parallelised with-loops.
 *    XT  : These functions are called in multi-threaded context and may
 *          contain parallelized with-loops.
 *
 *    We distinguish between such functions within this module through the
 *    flags FUNDEF_ISSTFUN and FUNDEF_ISXTFUN and later on through the name
 *    spaces ST and XT. Sequential functions stay in the standard name space.
 *
 *    We use this specialised version for modules and classes because we
 *    do not want to trace the static call graph of functions in this trivial
 *    case.
 *
 *****************************************************************************/

#include "restore_mtst_funs.h"

#define DBUG_PREFIX "RMTSTF"
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

typedef enum { SEQ, ST, XT } context_t;

struct INFO {
    context_t context;
    node *stcompanions;
    node *xtcompanions;
};

/**
 * INFO macros
 */

#define INFO_CONTEXT(n) ((n)->context)
#define INFO_STCOMPANIONS(n) ((n)->stcompanions)
#define INFO_XTCOMPANIONS(n) ((n)->xtcompanions)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = SEQ;
    INFO_STCOMPANIONS (result) = NULL;
    INFO_XTCOMPANIONS (result) = NULL;

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
 * @fn node *RMTSTFdoCreateMtFuns( node *syntax_tree)
 *
 *  @brief initiates RMTSTF traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
RMTSTFdoCreateMtFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rmtstf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * @fn node *RMTSTFmodule( node *arg_node, info *arg_info)
 *
 *  @brief RMTSTF traversal function for N_module node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMTSTFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RMTSTFfundef( node *arg_node, info *arg_info)
 *
 *  @brief RMTSTF traversal function for N_fundef node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
RMTSTFfundef (node *arg_node, info *arg_info)
{
    node *companion;
    namespace_t *old_namespace;

    DBUG_ENTER ();

    if (!FUNDEF_ISSTFUN (arg_node) && !FUNDEF_ISXTFUN (arg_node)
        && !FUNDEF_ISEXTERN (arg_node)) {

        /*
         * Create the ST companion:
         */

        companion = DUPdoDupNode (arg_node);

        FUNDEF_ISSTFUN (companion) = TRUE;

        old_namespace = FUNDEF_NS (companion);
        FUNDEF_NS (companion) = NSgetSTNamespace (old_namespace);
        old_namespace = NSfreeNamespace (old_namespace);

        FUNDEF_COMPANION (arg_node) = companion;

        FUNDEF_NEXT (companion) = INFO_STCOMPANIONS (arg_info);
        INFO_STCOMPANIONS (arg_info) = companion;

        /*
         * Create the XT companion:
         */

        companion = DUPdoDupNode (arg_node);

        FUNDEF_ISXTFUN (companion) = TRUE;

        old_namespace = FUNDEF_NS (companion);
        FUNDEF_NS (companion) = NSgetXTNamespace (old_namespace);
        old_namespace = NSfreeNamespace (old_namespace);

        FUNDEF_XTCOMPANION (arg_node) = companion;

        FUNDEF_NEXT (companion) = INFO_XTCOMPANIONS (arg_info);
        INFO_XTCOMPANIONS (arg_info) = companion;
    }

    /*
     * Recursively traverse through fundef chain.
     * Append ST and XT companions from info structure as created so far.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        FUNDEF_NEXT (arg_node) = INFO_STCOMPANIONS (arg_info);
        INFO_STCOMPANIONS (arg_info) = NULL;

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = INFO_XTCOMPANIONS (arg_info);
            INFO_XTCOMPANIONS (arg_info) = NULL;

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
