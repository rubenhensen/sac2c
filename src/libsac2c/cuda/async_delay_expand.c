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
    node *lhs;
    lut_t *h2d_lut;
    lut_t *d2h_lut;
    bool delassign;
};

/**
 * INFO macros
 */
#define INFO_CURASSIGN(n) ((n)->curassign)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_SETASSIGN(n) ((n)->setassign)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_H2D_LUT(n) ((n)->h2d_lut)
#define INFO_D2H_LUT(n) ((n)->d2h_lut)
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
    INFO_LHS (result) = NULL;
    INFO_H2D_LUT (result) = NULL;
    INFO_D2H_LUT (result) = NULL;
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
        INFO_D2H_LUT (arg_info) = LUTgenerateLut ();
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_H2D_LUT (arg_info) = LUTremoveLut (INFO_H2D_LUT (arg_info));
        INFO_D2H_LUT (arg_info) = LUTremoveLut (INFO_D2H_LUT (arg_info));
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

    /// FIXME: will break on d2h re-assign
    INFO_PREASSIGN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

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
 * @param assign An N_assign node
 * @param prf_type A valid prf
 */
static bool
isAssignPrf (node *assign, prf prf_type)
{
    bool ret = false;

    DBUG_ENTER ();

    DBUG_ASSERT (assign != NULL, "Passed in NULL N_assign node!");

    ret = NODE_TYPE (ASSIGN_STMT (assign)) == N_let
          && NODE_TYPE (LET_EXPR (ASSIGN_STMT (assign))) == N_prf
          && PRF_PRF (LET_EXPR (ASSIGN_STMT (assign))) == prf_type;

    DBUG_RETURN (ret);
}

/**
 *
 */
node *
CUADEid (node *arg_node, info *arg_info)
{
    node *assign;

    DBUG_ENTER ();

    DBUG_PRINT ("Checking if N_assign of N_avis %s has D2H_end on RHS...", ID_NAME (arg_node));
    assign = AVIS_SSAASSIGN (ID_AVIS (arg_node));

    if (assign != NULL && isAssignPrf (assign, F_device2host_end))
    {
        DBUG_PRINT ("Adding N_assign of N_avis %s to LUT...", ID_NAME (arg_node));
        INFO_D2H_LUT (arg_info) = LUTinsertIntoLutP (INFO_D2H_LUT (arg_info), ID_AVIS (arg_node), INFO_PREASSIGN (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADEprf (node *arg_node, info *arg_info)
{
    node *res, *next;

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

    case F_device2host_end:
        DBUG_PRINT ("Found d2h_end...");
        /* We want to push this down as far as possible, in order
         * to ensure that we add sufficent delay. Are only constraint
         * is that we cannot pass the use of our LHS.
         *
         * Concretely, we want:
         *
         * ~~~
         * ...
         * kernel <<<...>>> (a_dev)
         * cuad_tmp = d2h_start (a_dev)
         * b = d2h_end (cuad_tmp, a_dev)
         * ...
         * ...
         * ...
         * c = id (b)
         * ...
         * ~~~
         *
         * and transform this into:
         *
         * ~~~
         * ...
         * kernel <<<...>>> (a_dev)
         * cuad_tmp = d2h_start (a_dev)
         * ...
         * ...
         * ...
         * b = d2h_end (cuad_tmp, a_dev)
         * c = id (b)
         * ...
         * ~~~
         */

        DBUG_PRINT ("Searching for %s LHS of D2H_end...", IDS_NAME (INFO_LHS (arg_info)));
        res = LUTsearchInLutPp (INFO_D2H_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)));

        DBUG_ASSERT (res != IDS_AVIS (INFO_LHS (arg_info)) && res != NULL, "LHS is missing SSA N_assign!");

        // TODO actually move d2h_end DOWN!
        //next = ASSIGN_NEXT (res);
        //ASSIGN_NEXT (res) = INFO_CURASSIGN (arg_info);
        //ASSIGN_NEXT (INFO_CURASSIGN (arg_info)) = next;
        //INFO_DELASSIGN (arg_info) = true;

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
