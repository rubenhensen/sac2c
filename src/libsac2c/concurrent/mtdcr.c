/*****************************************************************************
 *
 * $Id$
 *
 * file:   mtdcr.c
 *
 * prefix: MTDCR
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

#include "mtdcr.h"

#include "mtdci.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "memory.h"

/**
 * INFO structure
 */

struct INFO {
    bool check;
    bool kill;
};

/**
 * INFO macros
 */

#define INFO_CHECK(n) ((n)->check)
#define INFO_KILL(n) ((n)->kill)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_CHECK (result) = FALSE;
    INFO_KILL (result) = FALSE;

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
 *
 * @fn node *MTDCRmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRfundef");

    if (FUNDEF_ISSTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may have contained parallel with-loops.
         * Hence, we constrain our activity accordingly.
         */
        DBUG_PRINT ("MTDCR", ("Entering function %s.", FUNDEF_NAME (arg_node)));

        arg_node = MTDCIdoMtDeadCodeInference (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRvardec");

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_KILL (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_KILL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_CHECK (arg_info)) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        INFO_CHECK (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRprf");

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
        INFO_CHECK (arg_info) = TRUE;
        break;
    case F_free:
        if (AVIS_ISDEAD (ID_AVIS (PRF_ARG1 (arg_node)))) {
            INFO_KILL (arg_info) = TRUE;
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRids");

    if (INFO_CHECK (arg_info)) {
        /*
         * We are on the right hand side of N_alloc.
         */
        if (AVIS_ISDEAD (IDS_AVIS (arg_node))) {
            INFO_KILL (arg_info) = TRUE;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTDCRdoMtDeadCodeRemoval( node *syntax_tree)
 *
 *  @brief initiates traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTDCRdoMtDeadCodeRemoval (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MTDCRdoMtDeadCodeRemoval");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtdcr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
