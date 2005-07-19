/*
 *
 * $Log$
 * Revision 1.1  2005/07/19 16:56:48  ktr
 * Initial revision
 *
 */
/*****************************************************************************
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
 *    know that there is always exactly on calling application that we can
 *    modifiy, too.
 *    the flags if we need an identifier are stored in the avis nodes
 *
 *****************************************************************************/
#include "deadcoderemoval.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "optimize.h"
#include "DataFlowMask.h"

/*
 * INFO structure
 */
struct INFO {
    bool remassign;
    dfmask_t *usedmask;
    node *assign;
    node *fundef;
    node *int_assign;
    node *ext_assign;
    node *lacrets;
    bool idsneeded;
    bool condremoved;
};

/*
 * INFO macros
 */
#define INFO_DCR_REMASSIGN(n) (n->remassign)
#define INFO_DCR_USEDMASK(n) (n->usedmask)
#define INFO_DCR_ASSIGN(n) (n->assign)
#define INFO_DCR_FUNDEF(n) (n->fundef)
#define INFO_DCR_INT_ASSIGN(n) (n->int_assign)
#define INFO_DCR_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_DCR_LACRETS(n) (n->lacrets)
#define INFO_DCR_IDSNEEDED(n) (n->idsneeded)
#define INFO_DCR_CONDREMOVED(n) (n->condremoved)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DCR_REMASSIGN (result) = FALSE;
    INFO_DCR_USEDMASK (result) = NULL;
    INFO_DCR_ASSIGN (result) = NULL;
    INFO_DCR_FUNDEF (result) = NULL;
    INFO_DCR_INT_ASSIGN (result) = NULL;
    INFO_DCR_EXT_ASSIGN (result) = NULL;
    INFO_DCR_LACRETS (result) = NULL;
    INFO_DCR_IDSNEEDED (result) = FALSE;
    INFO_DCR_CONDREMOVED (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *DCRdoDeadCodeRemoval(node *fundef, node *modul)
 *
 * description:
 *   starting point of DeadCodeRemoval for SSA form.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in
 *   their order of usage. The traversal mode (on toplevel, in special
 *   function) is annotated in the stacked INFO_DCR_DEPTH attribute.
 *
 *****************************************************************************/
node *
DCRdoDeadCodeRemoval (node *fundef, node *module)
{
    DBUG_ENTER ("DCRdoDeadCodeRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "DCRdoDeadCodeRemoval called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting dead code removal (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    TRAVpush (TR_dcr);
    fundef = TRAVdo (fundef, NULL);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *RemoveUnusedReturnValues( node *exprs, node *rets)
 *
 *****************************************************************************/
node *
RemoveUnusedReturnValues (node *exprs, node *rets)
{
    DBUG_ENTER ("RemoveUnusedReturnValues");

    if (EXPRS_NEXT (exprs) != NULL) {
        EXPRS_NEXT (exprs)
          = RemoveUnusedReturnValues (EXPRS_NEXT (exprs), RET_NEXT (rets));
    }

    if (RET_WASREMOVED (rets)) {
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

    DBUG_PRINT ("DCR",
                ("\nstarting dead code removal in fundef %s.", FUNDEF_NAME (arg_node)));

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info != NULL)) {
        info *info;
        dfmask_base_t *maskbase;

        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        info = MakeInfo ();
        INFO_DCR_FUNDEF (info) = arg_node;
        INFO_DCR_USEDMASK (info) = DFMgenMaskClear (maskbase);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
        }

        if (FUNDEF_ISLACFUN (arg_node)) {
            /*
             * traverse args and rets to remove unused ones from signature
             */
            INFO_DCR_EXT_ASSIGN (info) = INFO_DCR_ASSIGN (arg_info);
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            if (FUNDEF_RETS (arg_node) != NULL) {
                FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), info);
            }

            if (INFO_DCR_CONDREMOVED (arg_info)) {
                /*
                 * Conditional was removed: Convert to regular function
                 */
                FUNDEF_ISCONDFUN (arg_node) = FALSE;
                FUNDEF_ISDOFUN (arg_node) = FALSE;
                FUNDEF_ISINLINE (arg_node) = TRUE;
            }
        }

        INFO_DCR_USEDMASK (info) = DFMremoveMask (INFO_DCR_USEDMASK (info));
        info = FreeInfo (info);

        maskbase = DFMremoveMaskBase (maskbase);
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
    extap = ASSIGN_RHS (INFO_DCR_EXT_ASSIGN (arg_info));
    if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
        intap = ASSIGN_RHS (INFO_DCR_INT_ASSIGN (arg_info));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        node *intarg, *extarg;
        /*
         * stack internal and external arguments corresponding to this arg node
         */
        intarg = NULL;
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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
    if (!DFMtestMaskEntry (INFO_DCR_USEDMASK (arg_info), NULL, ARG_AVIS (arg_node))) {

        DBUG_PRINT ("DCR", ("remove arg %sa", ARG_NAME (arg_node)));

        arg_node = FREEdoFreeNode (arg_node);
        AP_ARGS (extap) = FREEdoFreeNode (AP_ARGS (extap));
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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
    extlet = ASSIGN_INSTR (INFO_DCR_EXT_ASSIGN (arg_info));
    if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
        intlet = ASSIGN_INSTR (INFO_DCR_INT_ASSIGN (arg_info));
    }

    if (RET_NEXT (arg_node) != NULL) {
        node *intids, *extids;
        /*
         * stack internal and external arguments corresponding to this arg node
         */
        intids = NULL;
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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
    if (RET_WASREMOVED (arg_node)) {
        DBUG_PRINT ("DCR", ("removing ret"));

        arg_node = FREEdoFreeNode (arg_node);
        LET_IDS (extlet) = FREEdoFreeNode (LET_IDS (extlet));
        if (INFO_DCR_INT_ASSIGN (arg_info) != NULL) {
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

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignmentchain in block */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* the complete block is empty -> create N_empty node */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * traverse all vardecs in block to remove useless ones
         */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

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
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    /* process vardec and remove it, if dead code */
    if (!DFMtestMaskEntry (INFO_DCR_USEDMASK (arg_info), NULL, VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("DCR", ("remove unused vardec %s", VARDEC_NAME (arg_node)));
        arg_node = FREEdoFreeNode (arg_node);
        dead_var++;
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
 *  needed (marked by INFO_DCR_REMASSIGN)
 *
 *****************************************************************************/
node *
DCRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* traverse instruction */
    INFO_DCR_REMASSIGN (arg_info) = TRUE;
    INFO_DCR_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* free this assignment if unused anymore */
    if (INFO_DCR_REMASSIGN (arg_info)) {
        DBUG_PRINT ("DCR", ("removing assignment"));
        arg_node = FREEdoFreeNode (arg_node);
        dead_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRreturn(node *arg_node , info *arg_info)
 *
 * description:
 *   starts traversal of return expressions to mark them as needed.
 *
 *****************************************************************************/
node *
DCRreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRreturn");

    if (FUNDEF_ISLACFUN (INFO_DCR_FUNDEF (arg_info))) {
        RETURN_EXPRS (arg_node)
          = RemoveUnusedReturnValues (RETURN_EXPRS (arg_node),
                                      FUNDEF_RETS (INFO_DCR_FUNDEF (arg_info)));
    }

    /* mark returned identifiers as needed */
    arg_node = TRAVcont (arg_node, arg_info);

    /* do not remove return instruction */
    INFO_DCR_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRlet(node *arg_node , info *arg_info)
 *
 * description:
 *  checks, if at least one of the left ids vardecs are needed. then this
 *  let is needed.
 *  a functions application of a special function requieres a special
 *  handling because you can remove parts of the results an modify the
 *  functions signature
 *  a RHS containing a primitive function application of type F_accu
 *  must never become dead code as we would lose the handle to the
 *  intermediate fold result (FIXES BUG #43)
 *
 *****************************************************************************/
node *
DCRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRlet");

    /*
     * check for special function as N_ap expression
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        if ((FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (arg_node))))
            && (AP_FUNDEF (LET_EXPR (arg_node)) != INFO_DCR_FUNDEF (arg_info))) {
            INFO_DCR_LACRETS (arg_info) = FUNDEF_RETS (AP_FUNDEF (LET_EXPR (arg_node)));
        }
    }

    /*
     * Traverse lhs identifiers
     */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }
    INFO_DCR_LACRETS (arg_info) = NULL;

    /*
     * accu() must never become dead code
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
        && (PRF_PRF (LET_EXPR (arg_node)) == F_accu)) {
        INFO_DCR_REMASSIGN (arg_info) = FALSE;
    }

    if (!INFO_DCR_REMASSIGN (arg_info)) {
        /* traverse right side of let (mark needed variables) */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /* mark ALL left hand side identifiers as needed */
        if (LET_IDS (arg_node) != NULL) {
            INFO_DCR_IDSNEEDED (arg_info) = TRUE;
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
            INFO_DCR_IDSNEEDED (arg_info) = FALSE;
        }

        /* Restore remassign information */
        INFO_DCR_REMASSIGN (arg_info) = FALSE;
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
 *   traverse all arguments to marks them as needed
 *
 *****************************************************************************/
node *
DCRap (node *arg_node, info *arg_info)
{
    bool travargs = TRUE;

    DBUG_ENTER ("DCRap");

    /* traverse special fundef without recursion */
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if (AP_FUNDEF (arg_node) == INFO_DCR_FUNDEF (arg_info)) {
            /* remember internal assignment */
            INFO_DCR_INT_ASSIGN (arg_info) = INFO_DCR_ASSIGN (arg_info);

            /*
             * arguments of recursive args do not need to be traversed
             * as they must create a demand otherwise not present.
             *
             * Ex.:
             * int Loop_0( int c, int d)
             * {
             *   _flat_0 = 1;
             *   c__SSA0_1 = (c + _flat_0);
             *   _flat_1 = 1;
             *   d__SSA0_1 = (d + _flat_1);
             *   if (true)
             *   {
             *     c__SSA0_2 = Loop_0( c__SSA0_1, d__SSA0_1);
             *   }
             *   else {
             *   }
             *   c__SSA0_3 = ( true ? c__SSA0_2 : c__SSA0_1 );
             *   return( c__SSA0_3);
             * }
             *
             * All arguments of the recursive application also demanded
             * - in the funcond block   OR
             * - in the loop body
             */
            travargs = FALSE;
        } else {
            DBUG_PRINT ("DCR", ("traverse in special fundef %s",
                                FUNDEF_NAME (AP_FUNDEF (arg_node))));

            /* start traversal of special fundef (and maybe reduce parameters!) */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("DCR", ("traversal of special fundef %s finished"
                                "continue in fundef %s\n",
                                FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                FUNDEF_NAME (INFO_DCR_FUNDEF (arg_info))));
        }
    }

    /* mark all args as needed */
    if ((travargs) && (AP_ARGS (arg_node) != NULL)) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRid(node *arg_node , info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
DCRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRid");

    /* Mark identifier as needed */
    DFMsetMaskEntrySet (INFO_DCR_USEDMASK (arg_info), NULL, ID_AVIS (arg_node));

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
     * Traverse ids bottom-up, always preserving the correct ret node
     */
    if (IDS_NEXT (arg_node) != NULL) {
        node *ret = NULL;

        if (INFO_DCR_LACRETS (arg_info) != NULL) {
            ret = INFO_DCR_LACRETS (arg_info);
            INFO_DCR_LACRETS (arg_info) = RET_NEXT (ret);
        }
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);

        INFO_DCR_LACRETS (arg_info) = ret;
    }

    if (INFO_DCR_IDSNEEDED (arg_info)) {
        /* preserve ids nodes still needed */
        DFMsetMaskEntrySet (INFO_DCR_USEDMASK (arg_info), NULL, IDS_AVIS (arg_node));
    } else {
        if (DFMtestMaskEntry (INFO_DCR_USEDMASK (arg_info), NULL, IDS_AVIS (arg_node))) {
            /* Variable is needed => preserve assignment */
            INFO_DCR_REMASSIGN (arg_info) = FALSE;
        } else {
            /* Variable is not needed */
            if (INFO_DCR_LACRETS (arg_info) != NULL) {
                /* mark ret node of special function for removal */
                RET_WASREMOVED (INFO_DCR_LACRETS (arg_info)) = TRUE;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRcode(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses exprs, block and next in this order
 *
 *****************************************************************************/
node *
DCRcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRcode");

    /* traverse expression */
    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    /* traverse code block */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRwithid(node *arg_node , info *arg_info)
 *
 * description:
 *   marks index vector and identifier as needed to preserve them
 *   if they are not explicit used in Withloop.
 *   to do so these identifier are handeld like ids on the RIGHT side of
 *   an assignment.
 *
 *****************************************************************************/
node *
DCRwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRwithid");

    INFO_DCR_IDSNEEDED (arg_info) = TRUE;

    /* Traverse sons */
    arg_node = TRAVcont (arg_node, arg_info);

    INFO_DCR_IDSNEEDED (arg_info) = FALSE;

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

    if (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (INFO_DCR_ASSIGN (arg_info)))) != N_return) {

        /* Traverse branches and predicate */
        arg_node = TRAVcont (arg_node, arg_info);

        INFO_DCR_REMASSIGN (arg_info) = FALSE;
    } else {
        /* There is no subsequent funcond: conditional is dead */
        INFO_DCR_CONDREMOVED (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
