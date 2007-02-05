/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "map_call_graph.h"

#include "liftoptflags.h"

/*
 * INFO structure
 */
struct INFO {
    bool optflag;
};

/*
 * INFO macros
 */
#define INFO_OPTFLAG(n) (n->optflag)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_OPTFLAG (result) = FALSE;

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
 * @fn node *LOFdoLiftOptFlags( node *arg_node)
 *
 *****************************************************************************/
node *
LOFdoLiftOptFlags (node *arg_node)
{
    DBUG_ENTER ("LOFdoLiftOptFlags");

    TRAVpush (TR_lof);

    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, NULL);
    }

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

static node *
InferOptFlag (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("InferOptFlag");

    INFO_OPTFLAG (arg_info) |= FUNDEF_WASOPTIMIZED (arg_node);
    INFO_OPTFLAG (arg_info) |= FUNDEF_WASUPGRADED (arg_node);

    DBUG_RETURN (arg_node);
}

static node *
SetOptFlag (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SetOptFlag");

    FUNDEF_WASOPTIMIZED (arg_node) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LOFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LOFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LOFfundef");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        arg_info = MakeInfo ();

        INFO_OPTFLAG (arg_info) = FUNDEF_WASOPTIMIZED (arg_node);
        arg_info = MCGdoMapCallGraph (arg_node, InferOptFlag, NULL,
                                      MCGcontLacFunAndOneLevel, arg_info);

        if (INFO_OPTFLAG (arg_info)) {
            DBUG_PRINT ("LOF", ("setting opt flag on fundef %s", CTIitemName (arg_node)));

            arg_info
              = MCGdoMapCallGraph (arg_node, SetOptFlag, NULL, MCGcontLacFun, arg_info);
            SetOptFlag (arg_node, arg_info);
        }
        arg_info = FreeInfo (arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
