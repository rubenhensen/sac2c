/**
 * @file
 * @defgroup cuade
 *
 * @brief
 *
 *
 * @ingroup cuda
 *
 * @{
 */
#include "async_delay_expand.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "print.h"
#include "globals.h"

#define DBUG_PREFIX "CUADE"
#include "debug.h"

#include "LookUpTable.h"
#include "DupTree.h"

/**
 * INFO structure
 */
struct INFO {
    node *curassign;
    node *preassign;
    node *setassign;
    lut_t *h2d_lut;
    bool delassign;
};

/**
 * INFO macros
 */
#define INFO_CURASSIGN(n) ((n)->curassign)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_SETASSIGN(n) ((n)->setassign)
#define INFO_H2D_LUT(n) ((n)->h2d_lut)
#define INFO_DELASSIGN(n) ((n)->delassign)

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

    INFO_CURASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_SETASSIGN (result) = NULL;
    INFO_H2D_LUT (result) = NULL;
    INFO_DELASSIGN (result) = false;

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
CUADEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node))
    {
        INFO_H2D_LUT (arg_info) = LUTgenerateLut ();
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_H2D_LUT (arg_info) = LUTremoveLut (INFO_H2D_LUT (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* bottom-up */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (INFO_DELASSIGN (arg_info))
    {
        ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (INFO_PREASSIGN (arg_info));
        INFO_PREASSIGN (arg_info) = ASSIGN_NEXT (arg_node);
        INFO_DELASSIGN (arg_info) = false;
    }

    INFO_CURASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SETASSIGN (arg_info))
    {
        ASSIGN_NEXT (INFO_SETASSIGN (arg_info)) = INFO_PREASSIGN (arg_info);
        ASSIGN_NEXT (arg_node) = INFO_SETASSIGN (arg_info);
        INFO_SETASSIGN (arg_info) = NULL;
    }

    INFO_PREASSIGN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADEids (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = LUTsearchInLutPp (INFO_H2D_LUT (arg_info), IDS_AVIS (arg_node));
    if (res != IDS_AVIS (arg_node) && res != NULL)
    {
        INFO_SETASSIGN (arg_info) = res;
        INFO_H2D_LUT (arg_info) = LUTupdateLutP (INFO_H2D_LUT (arg_info), IDS_AVIS (arg_node), NULL, NULL);
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node))
    {
    case F_host2device_start:
        DBUG_PRINT ("Found h2d_start...");
        /* In this instance we have found a H2D pair, and wish
         * to push the START as far up the current context as
         * possible. Our constraint is that we cannot move higher
         * then the assignment of the RHS. We do not need to move
         * the END down as this is likely the lowest point already
         * before the kernel invocation.
         *
         * Concretely, given the following:
         *
         * ~~~
         * {
         *   ...
         *   a = ...
         *   ...
         *   ...
         *   ...
         *   cuad_tmp = host2device_start (a)
         *   a_dev = host2device_end (cuad_tmp, a)
         *   kernel <<<...>>> (a_dev)
         *   ...
         * ~~~
         *
         * We want to transfor this into:
         *
         * ~~~
         * {
         *   ...
         *   a = ...
         *   cuad_tmp = host2device_start (a)
         *   ...
         *   ...
         *   ...
         *   a_dev = host2device_end (cuad_tmp, a)
         *   kernel <<<...>>> (a_dev)
         *   ...
         * ~~~
         *
         * FIXME Do we need to consider the case were we
         *       pass 'a' as an argument to some other call
         *       before we do START? E.g.
         * ~~~
         * a = ...
         * b = id (a)
         * cuad_tmp = host2device_start (a)
         * a_dev = host2device_end (cuad_tmp, a)
         * ~~~
         *
         * is push START to before assignment OK?
         */

        INFO_H2D_LUT (arg_info) = LUTinsertIntoLutP (INFO_H2D_LUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)), INFO_CURASSIGN (arg_info));
        INFO_DELASSIGN (arg_info) = true;
        break;

    default:
        // do nothing
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADEdoAsyncDelayExpand (node *syntax_tree)
{
    info * arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cuade);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** @} */
#undef DBUG_PREFIX
