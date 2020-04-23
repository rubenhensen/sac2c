/**
 * @file
 * @defgroup cuade
 *
 * @brief Given CUDA *_start and *_end primitives introduced in CUAD, move these such that
 *        we create a synchronisation window, wherein we push h2d_start up and d2h_end
 *        down.
 *
 * @note Rewriting this traversal more generically could be useful, as the principle
 *       operation here is readily applicable to various node types, not just N_prf. Being
 *       able to move up a given node, with the only constrain being that it does not
 *       occur before the assignment of its arguments, would be good.
 *
 * FIXME the way this traversal is written is, um, painfully complex. In particular our
 *       bottom-up traversing makes for some interesting constructions. It would be nice
 *       to simplify this, but given that there was the intention of having *one* sweep
 *       to do everything, this is the result. We might be able to use a few anonymous
 *       traversals to do this instead...
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
    node *fundef;       /**< holds node to current N_fundef */
    node *curassign;    /**< holds node to current N_assign */
    node *preassign;    /**< holds node to next N_assign */
    node *nextassign;   /**< holds node to previous N_assign */
    node *wnextassign;  /**< holds next N_assign of WL */
    node *upassign;     /**< holds N_assign with h2d, which will be pushed up */
    node *downassign;   /**< holds N_assign with d2h, which will be pushed down */
    node *lhs;          /**< holds N_let N_ids */
    lut_t *h2d_lut;     /**< lut to collect h2d N_assigns */
    lut_t *d2h_lut;     /**< lut to collect d2h N_assigns */
    bool delassign;     /**< flag indicating that current N_assign should be deleted */
    bool inwith;        /**< flag indicating that we are in a WL */
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
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

    INFO_FUNDEF (result) = NULL;
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
 * @brief we traverse the body of functions, we also store some state in case
 *        we traverse by way of an N_ap.
 *
 * @param arg_node
 * @param arg_info
 * @return N_fundef
 */
node *
CUADEfundef (node *arg_node, info *arg_info)
{
    bool olddelassign;
    node *oldfundef;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node))
    {
        DBUG_PRINT ("inspecting %s...", FUNDEF_NAME (arg_node));

        INFO_H2D_LUT (arg_info) = LUTgenerateLut ();
        INFO_D2H_LUT (arg_info) = LUTgenerateLut ();

        INFO_PREASSIGN (arg_info) = NULL;
        INFO_NEXTASSIGN (arg_info) = NULL;
        INFO_CURASSIGN (arg_info) = NULL;

        // we need to store state here as we might be coming from an N_ap
        olddelassign = INFO_DELASSIGN (arg_info);
        oldfundef = INFO_FUNDEF (arg_info);
        INFO_DELASSIGN (arg_info) = false;
        INFO_FUNDEF (arg_info) = arg_node;

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DELASSIGN (arg_info) = olddelassign;
        INFO_FUNDEF (arg_info) = oldfundef;

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
        DBUG_PRINT ("Deleting current N_assign...");
        DBUG_EXECUTE (PRTdoPrintNode (INFO_CURASSIGN (arg_info)););
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

    /* we need to handle the case where the RHS of a h2d is a argument of the
     * current fundef or is a lifted variable (as is done in the EMR traversal).
     * This means that we can freely place the h2d_start, as such
     * we wait till reach the top of the N_assign chain, and add any h2d_start that
     * is still in the LUT
     */
    if (INFO_NEXTASSIGN (arg_info) == NULL)
    {
        DBUG_PRINT ("Reached top of N_assign chain, checking if fundef arguments "
                    "are RHS to h2d");
        /* search for h2d using function arguments */
        node *res = NULL, *preassign = INFO_PREASSIGN (arg_info);
        node *args = FUNDEF_ARGS (INFO_FUNDEF (arg_info));
        while (args != NULL) {
            DBUG_PRINT ("Checking if N_avis %s is RHS of h2d...", ARG_NAME (args));
            res = LUTsearchInLutPp (INFO_H2D_LUT (arg_info), ARG_AVIS (args));
            if (res != ARG_AVIS (args) && res != NULL)
            {
                DBUG_PRINT ("...placing h2d at top of N_assign chain");
                ASSIGN_NEXT (res) = preassign;
                ASSIGN_NEXT (arg_node) = res;
                /* we need to update the preassign to be our new next */
                preassign = res;
                INFO_H2D_LUT (arg_info) = LUTupdateLutP (INFO_H2D_LUT (arg_info), ARG_AVIS (args), NULL, NULL);
            }
            args = ARG_NEXT (args);
        }

        DBUG_PRINT ("Now checking for h2d RHS which are ERC lifted arguments");
        node *vardecs = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));
        while (vardecs != NULL) {
            DBUG_PRINT ("Checking if N_avis %s is RHS of h2d...", VARDEC_NAME (vardecs));
            res = LUTsearchInLutPp (INFO_H2D_LUT (arg_info), VARDEC_AVIS (vardecs));
            if (res != VARDEC_AVIS (vardecs) && res != NULL)
            {
                DBUG_PRINT ("...placing h2d at top of N_assign chain");
                ASSIGN_NEXT (res) = preassign;
                ASSIGN_NEXT (arg_node) = res;
                /* we need to update the preassign to be our new next */
                preassign = res;
                INFO_H2D_LUT (arg_info) = LUTupdateLutP (INFO_H2D_LUT (arg_info), VARDEC_AVIS (vardecs), NULL, NULL);
            }

            vardecs = VARDEC_NEXT (vardecs);
        }
    }

    INFO_PREASSIGN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/**
 * @brief we traverse both RHS and LHS, collecting the LHS
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

    DBUG_PRINT ("Checking if N_avis %s is RHS of h2d...", IDS_NAME (arg_node));
    res = LUTsearchInLutPp (INFO_H2D_LUT (arg_info), IDS_AVIS (arg_node));
    if (res != IDS_AVIS (arg_node) && res != NULL)
    {
        DBUG_PRINT ("...preparing to move h2d up...");
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
    node *assign, *nassign;

    DBUG_ENTER ();

    DBUG_PRINT ("Checking if N_assign of N_avis %s has D2H_end on RHS...", ID_NAME (arg_node));
    assign = AVIS_SSAASSIGN (ID_AVIS (arg_node));

    if (assign != NULL
        && (isAssignPrf (assign, F_device2host_end)
            || isAssignPrf (assign, F_prefetch2host)))
    {
        if (assign != INFO_NEXTASSIGN (arg_info))
        {
            DBUG_PRINT ("...adding N_assign of N_avis to d2h LUT...");
            nassign = INFO_IN_WITH (arg_info)
                      ? INFO_W_NEXTASSIGN (arg_info)
                      : INFO_NEXTASSIGN (arg_info);
        } else {
            DBUG_PRINT ("...assign is directly after d2h, nor performing propagation.");
            nassign = NULL;
        }

        // We want to only ever use the last referenced point of N_avis, as such we
        // overwrite the mapping with the new N_assign.
        // We have one *special* case, which is when we are in a N_with. Here we
        // do not provide the next assign, but the next assign of the N_with.
        INFO_D2H_LUT (arg_info) = LUTupdateLutP (INFO_D2H_LUT (arg_info),
                                                 ID_AVIS (arg_node),
                                                 nassign,
                                                 NULL);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief we traverse withloop code block, creating a new info and passing the
 *        next assign of the withloop.
 *
 * @param arg_node
 * @param arg_info
 * @return N_with
 */
node *
CUADEwith (node *arg_node, info *arg_info)
{
    info *wlinfo;

    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    // we need to create a new info as we do not want to propogate
    // any other state then NEXTASSIGN and the LUTs.
    wlinfo = MakeInfo ();
    INFO_IN_WITH (wlinfo) = true;
    INFO_FUNDEF (wlinfo) = INFO_FUNDEF (arg_info);
    INFO_W_NEXTASSIGN (wlinfo) = INFO_NEXTASSIGN (arg_info);
    INFO_H2D_LUT (wlinfo) = INFO_H2D_LUT (arg_info);
    INFO_D2H_LUT (wlinfo) = INFO_D2H_LUT (arg_info);

    DBUG_PRINT ("Entering N_code of N_with...");
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), wlinfo);
    DBUG_PRINT ("Exiting N_code of N_with...");

    // we don't care for any values here, so we free.
    wlinfo = FreeInfo (wlinfo);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse conditional
 *
 * FIXME as this is currently written, we may run into problems if there are any
 * h2d/d2h preceding/after the conditional --- this however is not likely as its
 * within a seperate conditional function.
 *
 * @param arg_node
 * @param arg_info
 * @return N_cond
 */
node *
CUADEcond (node *arg_node, info *arg_info)
{
    info *condinfo;

    DBUG_ENTER ();

    condinfo = MakeInfo ();
    INFO_FUNDEF (condinfo) = INFO_FUNDEF (arg_info);
    INFO_H2D_LUT (condinfo) = LUTgenerateLut ();
    INFO_D2H_LUT (condinfo) = LUTgenerateLut ();

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), condinfo);

    // reset the info object
    INFO_DELASSIGN (condinfo) = false;
    INFO_PREASSIGN (condinfo) = NULL;
    INFO_NEXTASSIGN (condinfo) = NULL;
    INFO_CURASSIGN (condinfo) = NULL;
    INFO_H2D_LUT (condinfo) = LUTremoveContentLut (INFO_H2D_LUT (condinfo));
    INFO_D2H_LUT (condinfo) = LUTremoveContentLut (INFO_D2H_LUT (condinfo));

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), condinfo);

    // free the info and LUTs
    INFO_H2D_LUT (condinfo) = LUTremoveLut (INFO_H2D_LUT (condinfo));
    INFO_D2H_LUT (condinfo) = LUTremoveLut (INFO_D2H_LUT (condinfo));
    condinfo = FreeInfo (condinfo);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse primitives h2d/d2h.
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

    switch (PRF_PRF (arg_node)) {
    case F_prefetch2device:
        /**
         * We can handle the prefetch operations from CUDA managed memory
         * (@see cumm).
         */
    case F_host2device_start:
        DBUG_PRINT ("Found h2d_start...");
        /**
         * In this instance we have found a H2D pair, and wish
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
         * We want to transform this into:
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
        node *assign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)));

        /**
         * In some rare situations we will have a d2h followed eventually by a
         * h2d, where the RHS is the LHS of the d2h. This happens whenever we
         * need both the memory on the host and device. See the following:
         *
         * ~~~
         * a = d2h_end (a_dev);
         * b_dev = h2d_start (a);
         * ...
         * k, c_dev = some_function (a, b_dev);
         * ~~~
         *
         * We must be careful to not move one of the statements, as we will
         * loss its positition potentially leading to the statements being swapped.
         *
         * To prevent this, we choose to not lift the h2d, leaving it where it is.
         * We check the RHS of the h2d, and if it is a d2h, we do not allow the h2d
         * to be moved further up.
         *
         * Regarding the look of the example and its obvious optertunnity for
         * improvement, note that in the memory phase we deal with such a situation
         * through ERCs.
         */
        if (assign != NULL
            && (isAssignPrf (assign, F_device2host_end)
                || isAssignPrf (assign, F_prefetch2host)))
        {
            DBUG_PRINT ("RHS N_avis %s of h2d is LHS of d2h, we will not move this up", ID_NAME (PRF_ARG1 (arg_node)));
        } else {
            DBUG_PRINT ("Adding N_assign of N_avis %s to h2d LUT...", ID_NAME (PRF_ARG1 (arg_node)));
            INFO_H2D_LUT (arg_info) = LUTinsertIntoLutP (INFO_H2D_LUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)), INFO_CURASSIGN (arg_info));
            INFO_DELASSIGN (arg_info) = true;
        }

        break;

    case F_prefetch2host:
        /**
         * We can handle the prefetch operations from CUDA managed memory
         * (@see cumm). Additionally we extend CUMMid to support looking for
         * this primitive.
         */
    case F_device2host_end:
        DBUG_PRINT ("Found d2h_end...");
        /**
         * We want to push this down as far as possible, in order
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

        // we must not match with the current assignment
        if (res != IDS_AVIS (INFO_LHS (arg_info))
            && res != INFO_CURASSIGN (arg_info)
            && res != NULL)
        {
            DBUG_PRINT ("...found matching");
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
