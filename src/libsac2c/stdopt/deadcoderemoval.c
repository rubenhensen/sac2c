/*****************************************************************************
 *
 * file:   deadcoderemoval.c
 *
 * prefix: DCR
 *
 * description:
 *    this module traverses one function (and its lifted special fundefs)
 *    and removes all dead code. this transformation is NOT conservative
 *    (it might remove endless loops, if they are not necessary to compute
 *     the result)
 *
 * implementation:
 *    we start at the return statement and do a bottom-up traversal marking
 *    all needed identifiers. If we find a call to a special fundef, we remove
 *    all results not needed before traversing the special fundef. when we
 *    reach the top of a special fundef, we look for the unused args and
 *    remove them, too.
 *    These signature changes are only possible for special fundefs where we
 *    know that there is always exactly one calling application that we can
 *    modifiy, too.
 *    the flags if we need an identifier are stored in the avis nodes
 *
 * remarks:
 *    it turns out this implementation is rather conservative and is not able
 *    to remove superfluous computations performed in loops.
 *    To handle this, it would be necessary to split DCR into two seperate
 *    traversal.
 *    1. Identify all unused variables.
 *       In loops, this means that the recursive application itself does not
 *       induce demand for a computation. However, if non-dead assignments in
 *       the loop body depend on a certain loop argument, the identifier used
 *       in the corresponding argument position of the recursive application
 *       must not be considered dead.
 *       Hence, loops require a fixed point iteration to compute the minimal
 *       demand.
 *    2. Remove all assignments that are not actually needed along with
 *       superfluous return values and arguments of LaC functions.
 *
 *    It might be a good idea to use AVIS_ISDEAD to communicate whether a
 *    variable is needed or not between those traversal.
 *
 *****************************************************************************/
#include "deadcoderemoval.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "DCR"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "deadcodeinference.h"
#include "globals.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO {
    bool remassign;
    node *assign;
    node *fundef;
    node *int_assign;
    node *ext_assign;
    bool condremoved;
};

/*
 * INFO macros
 *
 * INFO_FUNDEF = parent fundef; NULL when on the main spine
 */
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INT_ASSIGN(n) (n->int_assign)
#define INFO_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_CONDREMOVED(n) (n->condremoved)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_REMASSIGN (result) = FALSE;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_INT_ASSIGN (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_CONDREMOVED (result) = FALSE;

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
 *   node *DCRdoDeadCodeRemoval(node *arg_node)
 *
 * description:
 *   applies dead code removal to all functions of the given module
 *   or fundef.
 *
 *   NB. Unnecessary arguments of LAC functions are removed.
 *
 *****************************************************************************/
node *
DCRdoDeadCodeRemoval (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_dcr);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RemoveUnusedReturnValues( node *exprs)
 *
 *****************************************************************************/
static node *
RemoveUnusedReturnValues (node *exprs)
{
    DBUG_ENTER ();

    if (EXPRS_NEXT (exprs) != NULL) {
        EXPRS_NEXT (exprs) = RemoveUnusedReturnValues (EXPRS_NEXT (exprs));
    }

    if (AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (exprs)))) {
        DBUG_PRINT ("Removing dead return value %s",
                    AVIS_NAME (ID_AVIS (EXPRS_EXPR (exprs))));
        exprs = FREEdoFreeNode (exprs);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *DCRfundef(node *arg_node , info *arg_info)
 *
 * description:
 *   Starts the traversal of a given fundef. Does NOT traverse to
 *   next fundef in chain!
 *   If arg_node happens to be a LACFUN, the unused arguments are cleared.
 *
 *****************************************************************************/
node *
DCRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("\nStarting dead code removal in %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node)
                   ? "wrapper"
                   : (FUNDEF_ISLACFUN (arg_node) ? "lacfun" : "function")),
                CTIitemName (arg_node));

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Infer dead variables
         */
        if (!FUNDEF_ISLACFUN (arg_node)) {
            /* normal fun: go through the lacfuns as well */
            arg_node = DCIdoDeadCodeInferenceOneFunction (arg_node);
        } else {
            /* do DCI in lacfuns only when the DCR was directly invoked on this lacfun */
            if (INFO_FUNDEF (arg_info) == NULL) {
                /* lacfun: confine the DCI to the single fundef only */
                arg_node = DCIdoDeadCodeInferenceOneFundef (arg_node);
            }
        }

        info *info = MakeInfo ();
        INFO_FUNDEF (info) = arg_node;

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

        if (FUNDEF_ISLACFUN (arg_node) && (INFO_FUNDEF (arg_info) != NULL)) {
            /*
             * traverse args and rets to remove unused ones from signature,
             * only when this is a lacfun and we have come into it from
             * a parent fun (i.e. the DCR is *not* run directly on the lacfun)
             */
            INFO_EXT_ASSIGN (info) = INFO_ASSIGN (arg_info);
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

            FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), info);

            if (INFO_CONDREMOVED (arg_info)) {
                /*
                 * Conditional was removed: Convert to regular function
                 */
                FUNDEF_ISCONDFUN (arg_node) = FALSE;
                FUNDEF_ISLOOPFUN (arg_node) = FALSE;
                FUNDEF_ISLACINLINE (arg_node) = TRUE;
                DBUG_ASSERT (global.local_funs_grouped == FALSE,
                             "glf form found during whole-module traversal");
            }
        }

        info = FreeInfo (info);
    }

    /* Traverse to the next function in the chain only when on the main spine,
     * not when currently in a local lacfun.
     * Lacfuns are discovered and traversed via the ap in this pass,
     * not via the FUNDEF_LOCALFUNS. */
    if (INFO_FUNDEF (arg_info) == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRarg(node *arg_node , info *arg_info)
 *
 * description:
 *   removes all args from function signature that have not been used in the
 *   function.
 *
 *****************************************************************************/
node *
DCRarg (node *arg_node, info *arg_info)
{
    node *intap = NULL;
    node *extap;

    DBUG_ENTER ();

    /*
     * Store the internal and external N_ap nodes
     */
    extap = ASSIGN_RHS (INFO_EXT_ASSIGN (arg_info));
    if (INFO_INT_ASSIGN (arg_info) != NULL) {
        intap = ASSIGN_RHS (INFO_INT_ASSIGN (arg_info));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        node *intarg, *extarg;
        /*
         * stack internal and external arguments corresponding to this arg node
         */
        intarg = NULL;
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
            intarg = AP_ARGS (intap);
            AP_ARGS (intap) = EXPRS_NEXT (intarg);
        }

        extarg = AP_ARGS (extap);
        AP_ARGS (extap) = EXPRS_NEXT (extarg);

        /*
         * Traverse next node
         */
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);

        /*
         * Restore internal and external arguments
         */
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
            EXPRS_NEXT (intarg) = AP_ARGS (intap);
            AP_ARGS (intap) = intarg;
        }

        EXPRS_NEXT (extarg) = AP_ARGS (extap);
        AP_ARGS (extap) = extarg;
    }

    /*
     * If this argument is not actually needed, it is removed from
     * the ARG list along with the concrete argument in the function applications
     */
    if (AVIS_ISDEAD (ARG_AVIS (arg_node))) {
        DBUG_PRINT ("remove arg %s", ARG_NAME (arg_node));

        arg_node = FREEdoFreeNode (arg_node);
        AP_ARGS (extap) = FREEdoFreeNode (AP_ARGS (extap));
        if (INFO_INT_ASSIGN (arg_info) != NULL) {

            // FIXME something wrong here. Crashes...
            // DBUG_PRINT ("removing corresponding concrete arg in N_ap %s",
            //  AVIS_NAME( ID_AVIS( EXPRS_EXPR(( AP_ARGS( intap))))));

            AP_ARGS (intap) = FREEdoFreeNode (AP_ARGS (intap));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRret(node *arg_node , info *arg_info)
 *
 * description:
 *   removes all rets from function signature that have not been used in the
 *   function.
 *
 *****************************************************************************/
node *
DCRret (node *arg_node, info *arg_info)
{
    node *intlet = NULL;
    node *extlet;

    DBUG_ENTER ();

    /*
     * Store the internal and external N_ap nodes
     */
    extlet = ASSIGN_STMT (INFO_EXT_ASSIGN (arg_info));
    if (INFO_INT_ASSIGN (arg_info) != NULL) {
        intlet = ASSIGN_STMT (INFO_INT_ASSIGN (arg_info));
    }

    if (RET_NEXT (arg_node) != NULL) {
        node *intids, *extids;
        /*
         * stack internal and external arguments corresponding to this arg node
         */
        intids = NULL;
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
            intids = LET_IDS (intlet);
            LET_IDS (intlet) = IDS_NEXT (intids);
        }

        extids = LET_IDS (extlet);
        LET_IDS (extlet) = IDS_NEXT (extids);

        /*
         * Traverse next node
         */
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);

        /*
         * Restore internal and external arguments
         */
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
            IDS_NEXT (intids) = LET_IDS (intlet);
            LET_IDS (intlet) = intids;
        }

        IDS_NEXT (extids) = LET_IDS (extlet);
        LET_IDS (extlet) = extids;
    }

    /*
     * If this argument is not actually needed, it is removed from
     * the RET list along with the concrete argument in the function applications
     */
    if (AVIS_ISDEAD (IDS_AVIS (LET_IDS (extlet)))) {
        DBUG_PRINT ("removing ret for %s", AVIS_NAME (IDS_AVIS (LET_IDS (extlet))));

        arg_node = FREEdoFreeNode (arg_node);
        LET_IDS (extlet) = FREEdoFreeNode (LET_IDS (extlet));
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
            LET_IDS (intlet) = FREEdoFreeNode (LET_IDS (intlet));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRblock(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses instructions and vardecs in this order.
 *
 *****************************************************************************/
node *
DCRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse assignment chain in block */
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /*
     * traverse all vardecs in block to remove useless ones
     */
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRvardec(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses vardecs and removes unused ones.
 *
 *****************************************************************************/
node *
DCRvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse to next vardec */
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    /* process vardec and remove it, if dead code */
    if (!VARDEC_ISSTICKY (arg_node) && AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("remove unused vardec %s", VARDEC_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        global.optcounters.dead_var++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRassign(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses assignment chain bottom-up and removes all assignments not
 *  needed (marked by INFO_REMASSIGN)
 *
 *****************************************************************************/
node *
DCRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* traverse instruction */
    INFO_REMASSIGN (arg_info) = TRUE;
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* free this assignment if unused anymore */
    if (INFO_REMASSIGN (arg_info)) {
        DBUG_PRINT ("removing assignment for %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))));
        arg_node = FREEdoFreeNode (arg_node);
        global.optcounters.dead_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRannotate(node *arg_node , info *arg_info)
 *
 * description:
 *  mark this RHS to be kept!
 *
 *****************************************************************************/
node *
DCRannotate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRreturn(node *arg_node , info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
DCRreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = RemoveUnusedReturnValues (RETURN_EXPRS (arg_node));
    }
    /* do not remove return instruction */
    INFO_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRlet(node *arg_node , info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
DCRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse lhs identifiers
     */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    // ensure we never remove an application of F_guard if the LHS is void!
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
        && (PRF_PRF (LET_EXPR (arg_node)) == F_guard)
        && TCcountIds (LET_IDS (arg_node)) == 0) {
        INFO_REMASSIGN (arg_info) = FALSE;
    }

    if (!INFO_REMASSIGN (arg_info)) {
        /* traverse right side of let */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /* Restore remassign information */
        INFO_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRap(node *arg_node , info *arg_info)
 *
 * description:
 *   if application of lacfun (cond, do), traverse into this
 *   function, except for recursive calls of the current function.
 *
 *****************************************************************************/
node *
DCRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * traverse special fundef without recursion
     */
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
            /* remember internal assignment */
            INFO_INT_ASSIGN (arg_info) = INFO_ASSIGN (arg_info);
        } else {
            DBUG_PRINT ("traverse in lacfun %s", CTIitemName (AP_FUNDEF (arg_node)));

            /* start traversal of special fundef (and maybe reduce parameters!) */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("traversal of lacfun %s finished.",
                        CTIitemName (AP_FUNDEF (arg_node)));
            DBUG_PRINT ("continuing with function %s...",
                        CTIitemName (INFO_FUNDEF (arg_info)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRids(node *arg_node , info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
DCRids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse ids bottom-up
     */
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    if (!AVIS_ISDEAD (IDS_AVIS (arg_node))) {
        DBUG_PRINT ("%s is alive", AVIS_NAME (IDS_AVIS (arg_node)));

        INFO_REMASSIGN (arg_info) = FALSE;
#ifndef DBUG_OFF
    } else {
        DBUG_PRINT ("%s is dead", AVIS_NAME (IDS_AVIS (arg_node)));
#endif
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRcode(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses block and next in this order
 *
 *****************************************************************************/
node *
DCRcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse code block */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /* traverse expression */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRcond(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses both conditional blocks.
 *
 *****************************************************************************/
node *
DCRcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_TYPE (ASSIGN_STMT (ASSIGN_NEXT (INFO_ASSIGN (arg_info)))) != N_return) {

        /* Traverse branches and predicate */
        arg_node = TRAVcont (arg_node, arg_info);

        INFO_REMASSIGN (arg_info) = FALSE;
    } else {
        /* There is no subsequent funcond: conditional is dead */
        INFO_CONDREMOVED (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRmodule(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses only funs in the module.
 *
 *****************************************************************************/
node *
DCRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
