/**
 * @file
 * @defgroup cuad
 *
 * @brief
 *
 *
 * @ingroup cuda
 *
 * @{
 */

#include "async_delay.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "print.h"
#include "globals.h"

#define DBUG_PREFIX "CUAD"
#include "debug.h"

#include "cuda_utils.h"
#include "new_types.h"
#include "DupTree.h"

/**
 * INFO structure
 */
struct INFO {
    node *lhs;
    node *nlhs;
    node *prfs;
    node *prfe;
};

/**
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NLHS(n) ((n)->nlhs)
#define INFO_PRFS(n) ((n)->prfs)
#define INFO_PRFE(n) ((n)->prfe)

/**
 * @name INFO functions
 *
 * @{
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_NLHS (result) = NULL;
    INFO_PRFS (result) = NULL;
    INFO_PRFE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** @} */

/**
 *
 */
node *
CUADassign (node *arg_node, info *arg_info)
{
    node *new;

    DBUG_ENTER ();

    /* bottom-down */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PRFE (arg_info) != NULL)
    {
       new = TBmakeAssign (TBmakeLet (INFO_LHS (arg_info),
                                      INFO_PRFE (arg_info)),
                           ASSIGN_NEXT (arg_node));
       ASSIGN_NEXT (arg_node) = new;
       INFO_PRFE (arg_info) = NULL;
       INFO_LHS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_PRFS (arg_info) != NULL)
    {
        LET_IDS (arg_node) = INFO_NLHS (arg_info);
        INFO_NLHS (arg_info) = NULL;
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = INFO_PRFS (arg_info);
        INFO_PRFS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADprf (node *arg_node, info *arg_info)
{
    node *navis;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node))
    {
    case F_host2device:
        navis = TBmakeAvis (TRAVtmpVarName ("dev"),
                            TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));
        INFO_NLHS (arg_info) = TBmakeIds (navis, NULL);
        INFO_PRFS (arg_info) = TCmakePrf1 (F_host2device_start,
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_PRFE (arg_info) = TCmakePrf2 (F_host2device_end,
                                           TBmakeIds (navis, NULL),
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        break;

    case F_device2host:
        navis = TBmakeAvis (TRAVtmpVarName ("dev"),
                            TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));
        INFO_NLHS (arg_info) = TBmakeIds (navis, NULL);
        INFO_PRFS (arg_info) = TCmakePrf1 (F_device2host_start,
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_PRFE (arg_info) = TCmakePrf2 (F_device2host_end,
                                           TBmakeIds (navis, NULL),
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        break;

    default:
        /* do nothing */
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADdoAsyncDelay (node *syntax_tree)
{
    info * arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cuad);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** @} */
#undef DBUG_PREFIX
