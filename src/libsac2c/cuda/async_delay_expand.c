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
    node *nextassign;
    node *wnextassign;
    node *upassign;
    node *downassign;
    node *lhs;
    lut_t *h2d_lut;
    lut_t *d2h_lut;
    bool delassign;
    bool inwith;
};

/**
 * INFO macros
 */
#define INFO_CURASSIGN(n) ((n)->curassign)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_NEXTASSIGN(n) ((n)->nextassign)
#define INFO_W_NEXTASSIGN(n) ((n)->wnextassign)
#define INFO_UPASSIGN(n) ((n)->upassign)
#define INFO_DOWNASSIGN(n) ((n)->downassign)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_H2D_LUT(n) ((n)->h2d_lut)
#define INFO_D2H_LUT(n) ((n)->d2h_lut)
#define INFO_DELASSIGN(n) ((n)->delassign)
#define INFO_IN_WITH(n) ((n)->inwith)

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
    INFO_NEXTASSIGN (result) = NULL;
    INFO_W_NEXTASSIGN (result) = NULL;
    INFO_UPASSIGN (result) = NULL;
    INFO_DOWNASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_H2D_LUT (result) = NULL;
    INFO_D2H_LUT (result) = NULL;
    INFO_DELASSIGN (result) = false;
    INFO_IN_WITH (result) = false;

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
 * @param arg_node
 * @param arg_info
 * @return N_fundef
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
 * @brief Cause re-ordering of N_assign chain depending on the whether or
 *        not we need to move a h2d/d2h up or down the chain.
 *
 * We traverse the N_assign chain bottom-up. We need to maintain some state,
 * specifically a reference to both the previous N_assign and the next one from
 * the current N_assign.
 *
 * Concretely, if we are at some N_assign (b) where (a) is the N_assign at the
 * top of the chain:
 *
 * ~~~
 *   ---- top of N_assign chain
 *  /
 * (a) -> (b) -> (c) -> (d) -> ...
 *  |      |      |
 *  |      |      ---- PREASSIGN
 *  |      ---- CURASSIGN
 *  ---- NEXTASSIGN
 * ~~~
 *
 * @param arg_node
 * @param arg_info
 * @return N_assign
 */
node *
CUADEassign (node *arg_node, info *arg_info)
{
    node *old_next;

    DBUG_ENTER ();

    /* bottom-up */
    old_next = INFO_NEXTASSIGN (arg_info);
    INFO_NEXTASSIGN (arg_info) = arg_node;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    INFO_NEXTASSIGN (arg_info) = old_next;

    /* Here we remove the reference to a particular N_assign within the
     * chain. This happens whenever we move a h2d/d2h up or down.
     */
    if (INFO_DELASSIGN (arg_info))
    {
        ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (INFO_PREASSIGN (arg_info));
        INFO_PREASSIGN (arg_info) = ASSIGN_NEXT (arg_node);
        INFO_DELASSIGN (arg_info) = false;
    }

    /* in the case of d2h, we move it down, making the next of a N_assign
     * somewhere lower down in the chain.
     */
    if (INFO_DOWNASSIGN (arg_info))
    {
        old_next = ASSIGN_NEXT (INFO_DOWNASSIGN (arg_info));
        ASSIGN_NEXT (INFO_DOWNASSIGN (arg_info)) = INFO_CURASSIGN (arg_info);
        ASSIGN_NEXT (INFO_CURASSIGN (arg_info)) = old_next;
        INFO_DOWNASSIGN (arg_info) = NULL;
    }

    INFO_CURASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* in the case of h2d, we move it up, making it the next of a N_assign
     * somewhere higher up in the chain.
     */
    if (INFO_UPASSIGN (arg_info))
    {
        ASSIGN_NEXT (INFO_UPASSIGN (arg_info)) = INFO_PREASSIGN (arg_info);
        ASSIGN_NEXT (arg_node) = INFO_UPASSIGN (arg_info);
        INFO_UPASSIGN (arg_info) = NULL;
    }

    INFO_PREASSIGN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/**
 *
 * @param arg_node
 * @param arg_info
 * @return N_let
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
 * @brief Check if LHS N_avis is an argument to a h2d, if so collect
 *              the h2d N_assign and prepare to move it to this point
 *              in the N_assign chain.
 *
 * @param arg_node
 * @param arg_info
 * @return N_ids
 */
node *
CUADEids (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = LUTsearchInLutPp (INFO_H2D_LUT (arg_info), IDS_AVIS (arg_node));
    if (res != IDS_AVIS (arg_node) && res != NULL)
    {
        INFO_UPASSIGN (arg_info) = res;
        INFO_H2D_LUT (arg_info) = LUTupdateLutP (INFO_H2D_LUT (arg_info), IDS_AVIS (arg_node), NULL, NULL);
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Test if a given N_assign contains a N_prf of given
 *        type on RHS.
 *
 * @param assign An N_assign node
 * @param prf_type A valid prf
 * @return true if N_prf found, otherwise false
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
 * @brief Check the current N_avis paramater's N_assign, if it contains
 *        a d2h, we collect N_assign above us to move the d2h down to.
 *
 * @param arg_node
 * @param arg_info
 * @return N_id
 */
node *
CUADEid (node *arg_node, info *arg_info)
{
    node *assign;

    DBUG_ENTER ();

    DBUG_PRINT ("Checking if N_assign of N_avis %s has D2H_end on RHS...", ID_NAME (arg_node));
    assign = AVIS_SSAASSIGN (ID_AVIS (arg_node));

    if (assign != NULL
        && isAssignPrf (assign, F_device2host_end)
        && assign != INFO_NEXTASSIGN (arg_info))
    {
        DBUG_PRINT ("Adding N_assign of N_avis %s to LUT...", ID_NAME (arg_node));
        // We want to only ever use the last referenced point of N_avis, as such we
        // overwrite the mapping with the new N_assign.
        // We have one *special* case, which is when we are in a N_with. Here we
        // do not provide the next assign, but the next assign of the N_with.
        INFO_D2H_LUT (arg_info) = LUTupdateLutP (INFO_D2H_LUT (arg_info),
                                                 ID_AVIS (arg_node),
                                                 INFO_IN_WITH (arg_info)
                                                 ? INFO_W_NEXTASSIGN (arg_info)
                                                 : INFO_NEXTASSIGN (arg_info),
                                                 NULL);
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 * @param arg_node
 * @param arg_info
 * @return N_with
 */
node *
CUADEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    INFO_IN_WITH (arg_info) = true;
    INFO_W_NEXTASSIGN (arg_info) = INFO_NEXTASSIGN (arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    INFO_IN_WITH (arg_info) = false;
    INFO_W_NEXTASSIGN (arg_info) = NULL;

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 * @param arg_node
 * @param arg_info
 * @return N_prf
 */
node *
CUADEprf (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    // we need to traverse through arguments here as well
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

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
         * is pushing START to before assignment OK?
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

        if (res != IDS_AVIS (INFO_LHS (arg_info)) && res != NULL)
        {
            INFO_D2H_LUT (arg_info) = LUTupdateLutP (INFO_D2H_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)), NULL, NULL);
            DBUG_EXECUTE (PRTdoPrintNode (res););
            INFO_DOWNASSIGN (arg_info) = res;
            INFO_DELASSIGN (arg_info) = true;
        }
        break;

    default:

        // do nothing
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 * @param syntax_tree
 * @return AST node
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
