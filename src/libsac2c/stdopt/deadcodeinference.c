#include "deadcodeinference.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "DCI"
#include "debug.h"

#include "traverse.h"
#include "ctinfo.h"
#include "free.h"

/*
 * DCI: dead code inference
 *
 * This traversal is setting AVIS_ISDEAD for all variables of a given
 * function body.
 *
 * Implementation:
 *    a) core mechanism:
 *    First all AVIS_ISDEAD values are set to TRUE. This happens in DCIarg and
 *    in DCIvardec. For non-lac functions, we mark all arguments as FALSE!
 *    (sbs 2024: not sure how this interacts with UAR....)
 *    Then, we make a bottom-up traversal (ensured by DCIassign). Starting
 *    from DCIreturn, we traverse only identifiers that are needed, marking
 *    AVIS_ISDEAD as FALSE in DCIid.
 *    When traversing a LHS (DCIids), we signal to DCIlet through
 *    INFO_ONEIDSNEEDED that at least one result is needed. Then, and only
 *    then, we traverse the RHS which can mark further variables as alive!
 *    There are two exceptions: F_accu enforceis the liveness of its arguments.
 *    Likewise, F_guard enforces the liveness of its arguments, if and only if
 *    its LHS is void! The latter is needed as mem:racc makes _guard_
 *    void but we still need to keep the guards!
 *    b) fixpoint iteration:
 *    On top of the core mechanism, we perform a fixpoint iteration to
 *    minimise the number of arguments / return values in loops and
 *    conditionals. Here, we only mark arguments as alive (AVIS_ISDEAD == FALSE)
 *    when there is demand coming from the uses within the LaCfun body.
 */




/*
 * INFO structure
 */
/* TODO: completely remove TS_fundef and simplify the code,
 * if DCIdoDeadCodeInferenceOneFundef() is not indeed needed */
enum travscop_t { TS_function, TS_fundef };
struct INFO {
    enum travscop_t travscope;
    node *assign;
    node *fundef;
    node *int_assign;
    node *ext_assign;
    bool oneidsneeded;
    bool allidsneeded;
};

/*
 * INFO macros
 * INFO_TRAVSCOPE           TS_function = normal fun, including its lacfuns
 *                          TS_fundef = only the single given lacfun
 */
#define INFO_TRAVSCOPE(n) (n->travscope)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INT_ASSIGN(n) (n->int_assign)
#define INFO_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_ONEIDSNEEDED(n) (n->oneidsneeded)
#define INFO_ALLIDSNEEDED(n) (n->allidsneeded)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    DBUG_ENTER ();

    info *result = (info *)MEMmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_function;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_INT_ASSIGN (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_ONEIDSNEEDED (result) = FALSE;
    INFO_ALLIDSNEEDED (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *DCIdoDeadCodeInferenceOneFundef(node *fundef)
 *
 * description:
 *   starting point of dead code inference for one fundef.
 *   lacfuns are not traversed.
 *   This is suitable if the fundef itself is a lacfun.
 *
 *****************************************************************************/
node *
DCIdoDeadCodeInferenceOneFundef (node *fundef)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "DCIdoDeadCodeInferenceOneFunction called for non-fundef node");

    info *info = MakeInfo ();
    INFO_TRAVSCOPE (info) = TS_fundef;

    TRAVpush (TR_dci);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *DCIdoDeadCodeInferenceOneFunction(node *fundef)
 *
 * description:
 *   starting point of dead code inference for one function (including lacfuns)
 *
 *****************************************************************************/
node *
DCIdoDeadCodeInferenceOneFunction (node *fundef)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "DCIdoDeadCodeInferenceOneFunction called for non-fundef node");

    info *info = MakeInfo ();
    INFO_TRAVSCOPE (info) = TS_function;

    TRAVpush (TR_dci);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * function:
 *     static void MarkAvisAlive( node *avis, info *arg_info)
 *
 * description:
 *     If someone wants this AVIS to be kept alive,
 *     we mark it so. We also traverse the SAA and EXTREMA
 *     links, to keep their referents alive, too.
 *
 *****************************************************************************/

static void
MarkAvisAlive (node *avis, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at %s", AVIS_NAME (avis));

    if (AVIS_ISDEAD (avis)) {
        AVIS_ISDEAD (avis) = FALSE;
        DBUG_PRINT ("marking var %s as alive", AVIS_NAME (avis));
        avis = TRAVsons (avis, arg_info);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * function:
 *     static node *FreeAvisSons( node *arg_node)
 *
 * description:
 *     Free the N_avis son nodes associated with arg_node, which is
 *     an N_return value. If we do not do this, things get
 *     very confused.
 *
 * Result: arg_node.
 *
 *****************************************************************************/
static node *
FreeAvisSons (node *arg_node)
{
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);
    DBUG_PRINT ("Freeing avis sons for %s", AVIS_NAME (avis));

    // This is so ugly, I could spit...
    AVIS_DIM (avis) = FREEoptFreeNode(AVIS_DIM (avis));

    AVIS_SHAPE (avis) = FREEoptFreeNode(AVIS_SHAPE (avis));

    AVIS_MIN (avis) = FREEoptFreeNode(AVIS_MIN (avis));

    AVIS_MAX (avis) = FREEoptFreeNode(AVIS_MAX (avis));

    AVIS_SCALARS (avis) = FREEoptFreeNode(AVIS_SCALARS (avis));

    AVIS_LACSO (avis) = FREEoptFreeNode(AVIS_LACSO (avis));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIfundef(node *arg_node , info *arg_info)
 *
 * description:
 *   Starts the traversal of a given fundef. Does NOT traverse to
 *   next fundef in chain!
 *
 *****************************************************************************/
node *
DCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("\nstarting dead code inference in fundef %s.", CTIitemName (arg_node));

    if (FUNDEF_BODY (arg_node) != NULL) {

        if ((INFO_TRAVSCOPE (arg_info) == TS_fundef)
            || ((INFO_TRAVSCOPE (arg_info) == TS_function)
                && (((!FUNDEF_ISLACFUN (arg_node))
                     || (INFO_FUNDEF (arg_info) != NULL))))) {
            info *info;
            bool fixedpointreached = FALSE;

            info = MakeInfo ();

            INFO_FUNDEF (info) = arg_node;
            INFO_TRAVSCOPE (info) = INFO_TRAVSCOPE (arg_info);

            DBUG_PRINT ("...processing %s.", CTIitemName (arg_node));

            /*
             * Traverse ARGS and VARDECS to initialize AVIS_ISDEAD
             */
            FUNDEF_VARDECS (arg_node) = TRAVopt (FUNDEF_VARDECS (arg_node), info);
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

            if (FUNDEF_ISLACFUN (arg_node)) {
                INFO_EXT_ASSIGN (info) = INFO_ASSIGN (arg_info);
            }

            while (!fixedpointreached) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

                fixedpointreached = TRUE;

                if ((INFO_TRAVSCOPE (info) == TS_function)
                    && (FUNDEF_ISLOOPFUN (arg_node))) {
                    node *args, *recexprs;
                    args = FUNDEF_ARGS (arg_node);
                    recexprs = AP_ARGS (ASSIGN_RHS (INFO_INT_ASSIGN (info)));

                    while (args != NULL) {
                        if ((!AVIS_ISDEAD (ARG_AVIS (args)))
                            && (AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (recexprs))))) {
                            MarkAvisAlive (ID_AVIS (EXPRS_EXPR (recexprs)), arg_info);
                            fixedpointreached = FALSE;
                        }
                        args = ARG_NEXT (args);
                        recexprs = EXPRS_NEXT (recexprs);
                    }
                }
            }

            info = FreeInfo (info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIarg(node *arg_node , info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
DCIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISDEAD (ARG_AVIS (arg_node)) = TRUE;
    DBUG_PRINT ("marking arg %s as potentially dead", AVIS_NAME (ARG_AVIS (arg_node)));

    if (!FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        MarkAvisAlive (ARG_AVIS (arg_node), arg_info);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIvardec(node *arg_node , info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
DCIvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("marking var %s as potentially dead", AVIS_NAME (VARDEC_AVIS (arg_node)));
    AVIS_ISDEAD (VARDEC_AVIS (arg_node)) = TRUE;

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIblock(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses instructions
 *
 *****************************************************************************/
node *
DCIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIassign(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses assignment chain bottom-up
 *
 *****************************************************************************/
node *
DCIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* traverse instruction */
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIreturn(node *arg_node , info *arg_info)
 *
 * description:
 *   performs traversal of return expressions to mark them as needed.
 *
 *   At this point, any N_avis son nodes associated with the N_return
 *   values are freed.
 *
 *****************************************************************************/
node *
DCIreturn (node *arg_node, info *arg_info)
{
    node *extids;
    node *retexprs;

    DBUG_ENTER ();

    if ((INFO_TRAVSCOPE (arg_info) == TS_function)
        && (FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))) {

        /* mark only those return values as needed that are required in the
           applying context */
        extids = ASSIGN_LHS (INFO_EXT_ASSIGN (arg_info));
        retexprs = RETURN_EXPRS (arg_node);

        while (extids != NULL) {
            DBUG_PRINT ("caller result value %s is for return value %s",
                        AVIS_NAME (IDS_AVIS (extids)),
                        AVIS_NAME (ID_AVIS (EXPRS_EXPR (retexprs))));
            if (!AVIS_ISDEAD (IDS_AVIS (extids))) {
                DBUG_PRINT ("Marking return value %s alive",
                            AVIS_NAME (ID_AVIS (EXPRS_EXPR (retexprs))));
                AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (retexprs))) = FALSE;
                EXPRS_EXPR (retexprs) = FreeAvisSons (EXPRS_EXPR (retexprs));
            }
            extids = IDS_NEXT (extids);
            retexprs = EXPRS_NEXT (retexprs);
        }
    } else {
        /* mark all returned identifiers as needed */
        RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIcond(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses both conditional blocks.
 *
 *****************************************************************************/
node *
DCIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Do not traverse COND_COND as the demand for the predicate should have been
     * created by live FUNCONDs before
     */
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIlet(node *arg_node , info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
DCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ONEIDSNEEDED (arg_info) = FALSE;
    INFO_ALLIDSNEEDED (arg_info) = FALSE;

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    /**
     * F_accu, F_guard, and F_conditional_error must never become dead code.
     * This case distinction is needed because the LHS of these primitive
     * functions has become void at this point.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        switch (PRF_PRF (LET_EXPR (arg_node))) {
        case F_accu:
        case F_guard:
        case F_conditional_error:
            INFO_ONEIDSNEEDED (arg_info) = TRUE;
            break;

        default:
            break;
        }
    }

    if (INFO_ONEIDSNEEDED (arg_info)) {

        if (!((INFO_TRAVSCOPE (arg_info) == TS_function)
              && (NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
              && (FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (arg_node)))))) {
            /*
             * IDS can only be removed individually if they are returned from
             * LAC functions. In all other cases, one live IDS keeps all other IDS
             * alive
             */
            INFO_ALLIDSNEEDED (arg_info) = TRUE;
            LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        }

        /*
         * Traverse RHS in order to generate more demand
         */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIap(node *arg_node , info *arg_info)
 *
 * description:
 *   if application of lacfun function (cond, do) traverse into this
 *   function except for recursive calls of the current function.
 *   traverse all arguments to mark them as needed
 *
 *****************************************************************************/
node *
DCIap (node *arg_node, info *arg_info)
{
    node *extids, *recids;

    DBUG_ENTER ();

    /* traverse lacfun without recursion */
    if ((INFO_TRAVSCOPE (arg_info) == TS_function)
        && (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))) {
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
            /* remember internal assignment */
            INFO_INT_ASSIGN (arg_info) = INFO_ASSIGN (arg_info);

            /**
             * mark all return values that are needed at the
             * EXT_ASSIGN needed for the INT_ASSIGN as well.
             * this prevents the unwanted deletion of return
             * ids in the case they are not needed within the
             * loops body. as an example try:
             *
             *   while( _lt_(res, 10)) {
             *     res = _add_SxS_(res, _sel_( iv, a));
             *     iv = [2];
             *   }
             */
            extids = ASSIGN_LHS (INFO_EXT_ASSIGN (arg_info));
            recids = ASSIGN_LHS (INFO_INT_ASSIGN (arg_info));

            while (extids != NULL) {
                if ((!AVIS_ISDEAD (IDS_AVIS (extids)))
                    && (AVIS_ISDEAD (IDS_AVIS (recids)))) {
                    DBUG_PRINT ("Marking fn argument %s alive in function %s",
                                AVIS_NAME (IDS_AVIS (recids)),
                                FUNDEF_NAME (AP_FUNDEF (arg_node)));
                    MarkAvisAlive (IDS_AVIS (recids), arg_info);
                }
                extids = IDS_NEXT (extids);
                recids = IDS_NEXT (recids);
            }

            /* do not mark the recursive parameters as needed in order not to
               create excessive demand */
        } else {
            node *args, *argexprs;

            DBUG_PRINT ("traverse in lacfun %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

            /* start traversal of lacfun (and maybe reduce parameters!) */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("traversal of lacfun %s finished;\n"
                        "continue in fundef %s\n",
                        FUNDEF_NAME (AP_FUNDEF (arg_node)),
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)));

            /* mark only those variables needed by the lacfun as live */
            args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            argexprs = AP_ARGS (arg_node);

            while (args != NULL) {
                DBUG_ASSERT (argexprs != NULL, "number of arguments at function declaration and application do not match!");
                if (!AVIS_ISDEAD (ARG_AVIS (args))) {
                    DBUG_PRINT ("Marking fn argument %s alive in function %s",
                                AVIS_NAME (ID_AVIS (EXPRS_EXPR (argexprs))),
                                FUNDEF_NAME (AP_FUNDEF (arg_node)));
                    MarkAvisAlive (ID_AVIS (EXPRS_EXPR (argexprs)), arg_info);
                }

                args = ARG_NEXT (args);
                argexprs = EXPRS_NEXT (argexprs);
            }
        }
    } else {
        /* mark all parameters of regular functions as needed */
        AP_ARGS (arg_node) = TRAVopt(AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIid(node *arg_node , info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
DCIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Mark identifier as needed */
    MarkAvisAlive (ID_AVIS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIids(node *arg_node , info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
DCIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ALLIDSNEEDED (arg_info)) {
        /* Mark identifier as needed */
        MarkAvisAlive (IDS_AVIS (arg_node), arg_info);
    }

    if (!AVIS_ISDEAD (IDS_AVIS (arg_node))) {
        INFO_ONEIDSNEEDED (arg_info) = TRUE;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIcode(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses exprs, block and next in this order
 *
 *****************************************************************************/
node *
DCIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse expression */
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);

    /* traverse code block */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /* traverse next block */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIwithid(node *arg_node , info *arg_info)
 *
 * description:
 *   marks index vector and identifier as needed to preserve them
 *   if they are not explicit used in Withloop.
 *
 *****************************************************************************/
node *
DCIwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ALLIDSNEEDED (arg_info) = TRUE;

    /* Traverse sons */
    arg_node = TRAVcont (arg_node, arg_info);

    INFO_ALLIDSNEEDED (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Marks the index of the range as alive, whether it is used or not.
 *        Furthermore, all sons are traversed.
 *
 * @param arg_node N_range node
 * @param arg_info info structure
 *
 * @return N_range node
 ******************************************************************************/
node *
DCIrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* mark index and offsets alive */
    INFO_ALLIDSNEEDED (arg_info) = TRUE;
    RANGE_INDEX (arg_node) = TRAVdo (RANGE_INDEX (arg_node), arg_info);
    RANGE_IDXS (arg_node) = TRAVopt (RANGE_IDXS (arg_node), arg_info);
    RANGE_IIRR (arg_node) = TRAVopt (RANGE_IIRR (arg_node), arg_info);
    INFO_ALLIDSNEEDED (arg_info) = FALSE;

    /* generate demand for the identifiers in the body */
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    /* generate demand for the arguments */
    RANGE_LOWERBOUND (arg_node) = TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);

    /* do the next range */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
