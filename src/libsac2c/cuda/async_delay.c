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
    node *fundef;
    node *lhs;
    node *nlhs;
    node *prfs;
    node *prfe;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
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

    INFO_FUNDEF (result) = NULL;
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
CUADfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADassign (node *arg_node, info *arg_info)
{
    node *new;

    DBUG_ENTER ();

    /* top-down */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PRFE (arg_info) != NULL)
    {

        new = TBmakeAssign (TBmakeLet (INFO_LHS (arg_info),
                    INFO_PRFE (arg_info)),
                ASSIGN_NEXT (arg_node));
        AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = new;
        ASSIGN_NEXT (arg_node) = new;
        AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))) = arg_node;
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
    node *navis, *nvd;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node))
    {
    case F_host2device:
    case F_device2host:
        DBUG_PRINT ("found cuda memcpy, changing N_prf");
        navis = TBmakeAvis (TRAVtmpVarName (PRF_PRF (arg_node) == F_host2device
                            ? "dev" : "host"),
                            TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));
        nvd = TBmakeVardec (navis, NULL);
        AVIS_DECL (navis) = nvd;
        INFO_NLHS (arg_info) = TBmakeIds (navis, NULL);
        INFO_PRFS (arg_info) = TCmakePrf1 (PRF_PRF (arg_node) == F_host2device
                                           ? F_host2device_start
                                           : F_device2host_start,
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_PRFE (arg_info) = TCmakePrf2 (PRF_PRF (arg_node) == F_host2device
                                           ? F_host2device_end
                                           : F_device2host_end,
                                           TBmakeId (navis),
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), nvd);
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
