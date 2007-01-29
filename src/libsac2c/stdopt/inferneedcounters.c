/*
 * $Id$
 */
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"

#include "inferneedcounters.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *INFNCdoInferNeedCounters( node *arg_node)
 *
 * @brief starting point of needcount inference
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
INFNCdoInferNeedCounters (node *arg_node)
{
    info *info;

    DBUG_ENTER ("INFNCdoInferNeedCounters");

    info = MakeInfo ();

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INFNCdoInferNeedCountersOneFundef( node *arg_node)
 *
 * @brief starting point of needcount inference. Traverses one function only
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
INFNCdoInferNeedCountersOneFundef (node *arg_node)
{
    info *info;

    DBUG_ENTER ("INFNCdoInferNeedCountersOneFundef");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Needcount inference traversal (inc_tab)
 *
 * prefix: INFNC
 *
 *****************************************************************************/
node *
INFNCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
INFNCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
INFNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCavis");

    AVIS_NEEDCOUNT (arg_node) = 0;

    DBUG_RETURN (arg_node);
}

node *
INFNCid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("INFNCid");

    avis = ID_AVIS (arg_node);

    AVIS_NEEDCOUNT (avis) += 1;

    if (AVIS_DIM (avis) != NULL) {
        AVIS_DIM (avis) = TRAVdo (AVIS_DIM (avis), arg_info);
    }

    if (AVIS_SHAPE (avis) != NULL) {
        AVIS_SHAPE (avis) = TRAVdo (AVIS_SHAPE (avis), arg_info);
    }

    DBUG_RETURN (arg_node);
}
