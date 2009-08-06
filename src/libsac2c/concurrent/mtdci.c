/*****************************************************************************
 *
 * $Id$
 *
 * file:   mtdci.c
 *
 * prefix: MTDCI
 *
 * description:
 *
 *  This traversal is auxiliary to MT dead code removal. Despite the name,
 *  this is not a fully fledged dead code inference. In fact, potential dead
 *  code is already flagged during creation of spmd functions. This traversal
 *  rather checks that the flagged code is really dead, i.e. the a certain
 *  identifier does not occur anywhere else, but in a a free or alloc
 *  statement. These stem from with-loop index variables that have been moved
 *  into the spmd function.
 *
 *****************************************************************************/

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
    bool ignore;
};

/**
 * INFO macros
 */

#define INFO_IGNORE(n) ((n)->ignore)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_IGNORE (result) = FALSE;

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
 * @fn node *MTDCIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIfundef");

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCIblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCIlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_IGNORE (arg_info)) {
        INFO_IGNORE (arg_info) = FALSE;
    } else {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCIprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIprf");

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
    case F_free:
        INFO_IGNORE (arg_info) = TRUE;
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCIids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIids");

    AVIS_ISDEAD (IDS_AVIS (arg_node)) = FALSE;
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCIid");

    AVIS_ISDEAD (ID_AVIS (arg_node)) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTDCIdoMtDeadCodeInference( node *syntax_tree)
 *
 *  @brief initiates traversal starting with N_fundef node
 *
 *  @param fundef
 *
 *  @return fundef
 *
 *****************************************************************************/

node *
MTDCIdoMtDeadCodeInference (node *fundef)
{
    info *info;

    DBUG_ENTER ("MTDCIdoMtDeadCodeInference");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "Illegal argument node!");
    DBUG_PRINT ("MTDCI", ("Entering function %s.", FUNDEF_NAME (fundef)));

    info = MakeInfo ();

    TRAVpush (TR_mtdci);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("MTDCI", ("Leaving function %s.", FUNDEF_NAME (fundef)));

    DBUG_RETURN (fundef);
}
