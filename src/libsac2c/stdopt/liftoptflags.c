#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"

#define DBUG_PREFIX "LOF"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "map_call_graph.h"
#include "ctinfo.h"

#include "liftoptflags.h"

/*
 * INFO structure
 */
struct INFO {
    bool optflag;
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_OPTFLAG(n) (n->optflag)
#define INFO_ONEFUNDEF(n) (n->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_OPTFLAG (result) = FALSE;
    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *LOFdoLiftOptFlags( node *arg_node)
 *
 *****************************************************************************/
node *
LOFdoLiftOptFlags (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));
    TRAVpush (TR_lof);

    arg_node = TRAVopt (arg_node, arg_info);

    TRAVpop ();
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *InferOptFlags( node *arg_node)
 *
 *****************************************************************************/
static node *
InferOptFlag (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_OPTFLAG (arg_info) |= FUNDEF_WASOPTIMIZED (arg_node);
    INFO_OPTFLAG (arg_info) |= FUNDEF_WASUPGRADED (arg_node);

    DBUG_RETURN (arg_node);
}

static node *
SetOptFlag (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (!FUNDEF_ISLACFUN (arg_node)) {
        INFO_OPTFLAG (arg_info) = FUNDEF_WASOPTIMIZED (arg_node);
        arg_info = MCGdoMapCallGraph (arg_node, InferOptFlag, NULL,
                                      MCGcontLacFunAndOneLevel, arg_info);

        if (INFO_OPTFLAG (arg_info)) {
            DBUG_PRINT ("setting opt flag on fundef %s", CTIitemName (arg_node));

            arg_info
              = MCGdoMapCallGraph (arg_node, SetOptFlag, NULL, MCGcontLacFun, arg_info);
            SetOptFlag (arg_node, arg_info);
        }
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
