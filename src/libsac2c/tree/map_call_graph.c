#include "map_call_graph.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/**
 * INFO structure
 */
struct INFO {
    travfun_p mapfundown;
    travfun_p mapfunup;
    info *minfo;
    node *current;
    int level;
    int laclevel;
    bool (*cont) (node *, info *);
};

/**
 * INFO macros
 */
#define INFO_MAPFUNDOWN(n) ((n)->mapfundown)
#define INFO_MAPFUNUP(n) ((n)->mapfunup)
#define INFO_INFO(n) ((n)->minfo)
#define INFO_CURRENT(n) ((n)->current)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_LACLEVEL(n) ((n)->laclevel)
#define INFO_CONT(n) ((n)->cont)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MAPFUNDOWN (result) = NULL;
    INFO_MAPFUNUP (result) = NULL;
    INFO_INFO (result) = NULL;
    INFO_CURRENT (result) = NULL;
    INFO_CONT (result) = NULL;
    INFO_LEVEL (result) = -1;
    INFO_LACLEVEL (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * traversal functions
 */

node *
MCGfundef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ();

    last = INFO_CURRENT (arg_info);
    INFO_CURRENT (arg_info) = arg_node;
    INFO_LEVEL (arg_info)++;

    if (FUNDEF_ISLACFUN (arg_node)) {
        INFO_LACLEVEL (arg_info)++;
    }

    if (INFO_MAPFUNDOWN (arg_info) != NULL) {
        arg_node = INFO_MAPFUNDOWN (arg_info) (arg_node, INFO_INFO (arg_info));
    }

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    if (INFO_MAPFUNUP (arg_info) != NULL) {
        arg_node = INFO_MAPFUNUP (arg_info) (arg_node, INFO_INFO (arg_info));
    }

    if (FUNDEF_ISLACFUN (arg_node)) {
        INFO_LACLEVEL (arg_info)--;
    }

    INFO_LEVEL (arg_info)--;
    INFO_CURRENT (arg_info) = last;

    DBUG_RETURN (arg_node);
}

node *
MCGap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * call the continue function to check whether we have to go on or not.
     */
    if (INFO_CONT (arg_info) (AP_FUNDEF (arg_node), arg_info)) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * cont functions
 */

bool
MCGcontNever (node *fundef, info *info)
{
    return (FALSE);
}

bool
MCGcontAlways (node *fundef, info *info)
{
    return (TRUE);
}

bool
MCGcontLacFun (node *fundef, info *info)
{
    bool result;

    result = FUNDEF_ISLACFUN (fundef) && INFO_CURRENT (info) != fundef;

    return (result);
}

bool
MCGcontOneLevel (node *fundef, info *info)
{
    bool result;

    result = INFO_LEVEL (info) <= 1;

    return (result);
}

bool
MCGcontLacFunAndOneLevel (node *fundef, info *info)
{
    bool result;

    result = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_CURRENT (info))
             || (!FUNDEF_ISLACFUN (fundef)
                 && ((INFO_LEVEL (info) - INFO_LACLEVEL (info)) <= 1));

    return (result);
}

/**
 * traversal start function
 */
info *
MCGdoMapCallGraph (node *arg_node, travfun_p mapfundown, travfun_p mapfunup,
                   bool (*contfun) (node *, info *), info *arg_info)
{
    info *localinfo;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "MCGdoMapLacFuns called on non fundef node");

    localinfo = MakeInfo ();

    INFO_MAPFUNDOWN (localinfo) = mapfundown;
    INFO_MAPFUNUP (localinfo) = mapfunup;
    INFO_INFO (localinfo) = arg_info;
    INFO_CONT (localinfo) = contfun;

    TRAVpush (TR_mcg);

    arg_node = TRAVdo (arg_node, localinfo);

    TRAVpop ();

    localinfo = FreeInfo (localinfo);

    DBUG_RETURN (arg_info);
}

#undef DBUG_PREFIX
