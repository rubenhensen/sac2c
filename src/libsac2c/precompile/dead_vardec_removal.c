/*****************************************************************************
 *
 * file:   dead_vardec_removal.c
 *
 * prefix: DVR
 *
 * description:
 *
 *   This is a special variant of dead code removal tailor made to be run
 *   in the multithreading phase after creation of spmd functions.
 *
 *   Creation of spmd functions leaves a certain kind of dead code in the
 *   ST function it is lifted from:
 *    - variable declarations of with-loop local identifiers now residing in
 *      the spmd function
 *    - alloc and free statements for the withid variables that are now
 *      handled thread-locally within the spmd function
 *
 *   Avis nodes of identifiers to be removed are already tagged by the
 *   previous phase.
 *
 *****************************************************************************/

#include "dead_vardec_removal.h"

#define DBUG_PREFIX "DVR"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "print.h"
#include "memory.h"

/**
 * INFO structure
 */

struct INFO {
    bool reset;
    bool kill;
};

/**
 * INFO macros
 */

#define INFO_RESET(n) ((n)->reset)
#define INFO_KILL(n) ((n)->kill)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_RESET (result) = FALSE;
    INFO_KILL (result) = FALSE;

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
 *
 * @fn node *DVRmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DVRfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DVRblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RESET (arg_info) = TRUE;
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    INFO_RESET (arg_info) = FALSE;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_KILL (arg_info) = TRUE;
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    INFO_KILL (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DVRvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_RESET (arg_info)) {
        DBUG_PRINT ("Marking as dead:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node););
        AVIS_ISDEAD (VARDEC_AVIS (arg_node)) = TRUE;
    }

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    if (INFO_KILL (arg_info)) {
        if (!VARDEC_ISSTICKY (arg_node) && AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
            DBUG_PRINT ("Removing:");
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node););
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DVRids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISDEAD (IDS_AVIS (arg_node)) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DVRid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
DVRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISDEAD (ID_AVIS (arg_node)) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn DVRdoDeadVardecRemoval( node *syntax_tree)
 *
 *  @brief initiates traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
DVRdoDeadVardecRemoval (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_dvr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
