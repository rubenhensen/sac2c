/*****************************************************************************
 *
 * $Id$
 *
 * file:   deadcoderemoval.c
 *
 * prefix: DCR
 *
 * description:
 *    this module traverses one function (and its liftet special fundefs)
 *    and removes all dead code. this transformation is NOT conservative
 *    (it might remove endless loops, if they are not necessary to compute
 *     the result)
 *
 * implementation:
 *    we start at the return statement and do a bottom-up traversal marking
 *    all needed identifier. if we find a call to a special fundef, we remove
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
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "deadcodeinference.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    enum { TS_module, TS_fundef } travscope;
    bool remassign;
    node *assign;
    node *fundef;
    node *int_assign;
    node *ext_assign;
    bool condremoved;
};

/*
 * INFO macros
 */
#define INFO_TRAVSCOPE(n) (n->travscope)
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_module;
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
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *DCRdoDeadCodeRemovalOneFundef(node *fundef)
 *
 * description:
 *   starting point of dead code removal.
 *   applies dead code removal to the designated function only.
 *
 *****************************************************************************/
node *
DCRdoDeadCodeRemovalOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("DCRdoDeadCodeRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "DCRdoDeadCodeRemovalOneFunction called for non-fundef node");

    info = MakeInfo ();
    INFO_TRAVSCOPE (info) = TS_fundef;

    TRAVpush (TR_dcr);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *DCRdoDeadCodeRemovalModule(node *module)
 *
 * description:
 *   applies dead code removal to all functions of the given module.
 *   Thereby, unnecessary arguments of LAC functions are removed.
 *
 *****************************************************************************/
node *
DCRdoDeadCodeRemovalModule (node *module)
{
    info *info;

    DBUG_ENTER ("DCRdoDeadCodeRemovalModule");

    info = MakeInfo ();
    INFO_TRAVSCOPE (info) = TS_module;

    TRAVpush (TR_dcr);
    module = TRAVdo (module, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (module);
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
    DBUG_ENTER ("RemoveUnusedReturnValues");

    if (EXPRS_NEXT (exprs) != NULL) {
        EXPRS_NEXT (exprs) = RemoveUnusedReturnValues (EXPRS_NEXT (exprs));
    }

    if (AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (exprs)))) {
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
    DBUG_ENTER ("DCRfundef");

    DBUG_PRINT ("DCR", ("\nStarting dead code removal in fundef %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node)));

    if ((INFO_TRAVSCOPE (arg_info) == TS_fundef)
        || ((INFO_TRAVSCOPE (arg_info) == TS_module)
            && (((!FUNDEF_ISLACFUN (arg_node)) || (INFO_FUNDEF (arg_info) != NULL))))) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            /*
             * Infere dead variables
             */
            if (INFO_TRAVSCOPE (arg_info) == TS_fundef) {
                arg_node = DCIdoDeadCodeInferenceOneFundef (arg_node);
            }

            if ((INFO_TRAVSCOPE (arg_info) == TS_module)
                && (!FUNDEF_ISLACFUN (arg_node))) {
                arg_node = DCIdoDeadCodeInferenceOneFunction (arg_node);
            }

            info = MakeInfo ();
            INFO_TRAVSCOPE (info) = INFO_TRAVSCOPE (arg_info);
            INFO_FUNDEF (info) = arg_node;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if ((INFO_TRAVSCOPE (info) == TS_module) && (FUNDEF_ISLACFUN (arg_node))) {
                /*
                 * traverse args and rets to remove unused ones from signature
                 */
                INFO_EXT_ASSIGN (info) = INFO_ASSIGN (arg_info);
                FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

                FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), info);

                if (INFO_CONDREMOVED (arg_info)) {
                    /*
                     * Conditional was removed: Convert to regular function
                     */
                    FUNDEF_ISCONDFUN (arg_node) = FALSE;
                    FUNDEF_ISDOFUN (arg_node) = FALSE;
                    FUNDEF_ISLACINLINE (arg_node) = TRUE;
                }
            }

            info = FreeInfo (info);
        }
    }

    if ((INFO_TRAVSCOPE (arg_info) == TS_module) && (INFO_FUNDEF (arg_info) == NULL)) {
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

    DBUG_ENTER ("DCRarg");

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
        DBUG_PRINT ("DCR", ("remove arg %sa", ARG_NAME (arg_node)));

        arg_node = FREEdoFreeNode (arg_node);
        AP_ARGS (extap) = FREEdoFreeNode (AP_ARGS (extap));
        if (INFO_INT_ASSIGN (arg_info) != NULL) {
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

    DBUG_ENTER ("DCRret");

    /*
     * Store the internal and external N_ap nodes
     */
    extlet = ASSIGN_INSTR (INFO_EXT_ASSIGN (arg_info));
    if (INFO_INT_ASSIGN (arg_info) != NULL) {
        intlet = ASSIGN_INSTR (INFO_INT_ASSIGN (arg_info));
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
        DBUG_PRINT ("DCR", ("removing ret"));

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
    DBUG_ENTER ("DCRblock");

    /* traverse assignmentchain in block */
    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* the complete block is empty -> create N_empty node */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    /*
     * traverse all vardecs in block to remove useless ones
     */
    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);

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
    DBUG_ENTER ("DCRvardec");

    /* traverse to next vardec */
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    /* process vardec and remove it, if dead code */
    if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("DCR", ("remove unused vardec %s", VARDEC_NAME (arg_node)));
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
    DBUG_ENTER ("DCRassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* traverse instruction */
    INFO_REMASSIGN (arg_info) = TRUE;
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* free this assignment if unused anymore */
    if (INFO_REMASSIGN (arg_info)) {
        DBUG_PRINT ("DCR", ("removing assignment"));
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
    DBUG_ENTER ("DCRannotate");

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
    DBUG_ENTER ("DCRreturn");

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
    DBUG_ENTER ("DCRlet");

    /*
     * Traverse lhs identifiers
     */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

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
 *   if application of special function (cond, do) traverse into this
 *   function except for recursive calls of the current function.
 *
 *****************************************************************************/
node *
DCRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRap");

    if (INFO_TRAVSCOPE (arg_info) == TS_module) {
        /*
         * traverse special fundef without recursion
         */
        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
            if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
                /* remember internal assignment */
                INFO_INT_ASSIGN (arg_info) = INFO_ASSIGN (arg_info);
            } else {
                DBUG_PRINT ("DCR", ("traverse in special fundef %s",
                                    FUNDEF_NAME (AP_FUNDEF (arg_node))));

                /* start traversal of special fundef (and maybe reduce parameters!) */
                AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

                DBUG_PRINT ("DCR", ("traversal of special fundef %s finished"
                                    "continue in fundef %s\n",
                                    FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                    FUNDEF_NAME (INFO_FUNDEF (arg_info))));
            }
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
    DBUG_ENTER ("DCRids");

    /*
     * Traverse ids bottom-up
     */
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    if (!AVIS_ISDEAD (IDS_AVIS (arg_node))) {
        INFO_REMASSIGN (arg_info) = FALSE;
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
    DBUG_ENTER ("DCRcode");

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
    DBUG_ENTER ("DCRcond");

    if (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (INFO_ASSIGN (arg_info)))) != N_return) {

        /* Traverse branches and predicate */
        arg_node = TRAVcont (arg_node, arg_info);

        INFO_REMASSIGN (arg_info) = FALSE;
    } else {
        /* There is no subsequent funcond: conditional is dead */
        INFO_CONDREMOVED (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
