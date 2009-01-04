/*
 *
 * $Id$
 *
 */
#include "deadcodeinference.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"

/*
 * INFO structure
 */
struct INFO {
    enum { TS_function, TS_fundef } travscope;
    node *assign;
    node *fundef;
    node *int_assign;
    node *ext_assign;
    bool oneidsneeded;
    bool allidsneeded;
};

/*
 * INFO macros
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_fundef;
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
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *DCIdoDeadCodeInferenceOneFundef(node *fundef)
 *
 * description:
 *   starting point of dead code inference for one fundef
 *
 *****************************************************************************/
node *
DCIdoDeadCodeInferenceOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("DCIdoDeadCodeInferenceOneFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "DCIdoDeadCodeInferenceOneFunction called for non-fundef node");

    info = MakeInfo ();
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
    info *info;

    DBUG_ENTER ("DCIdoDeadCodeInferenceOneFunction");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "DCIdoDeadCodeInferenceOneFunction called for non-fundef node");

    info = MakeInfo ();
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
 *     static void MarkAvisAlive( node *avis)
 *
 * description:
 *     If someone wants this AVIS to be kept alive,
 *     we mark it so. We also traverse the SAA and EXTREMA
 *     links, to keep their referents alive, too.
 *
 *     This action is required because the links (e.g., AVIS_MINVAL)
 *     are attributes, not sons, in the ast.
 *
 *****************************************************************************/

static void
MarkAvisAlive (node *avis)
{
    DBUG_ENTER ("MarkAvisAlive");

    if (AVIS_ISDEAD (avis)) {
        AVIS_ISDEAD (avis) = FALSE;
        DBUG_PRINT ("DCI", ("marking var %s as alive", AVIS_NAME (avis)));

        if (AVIS_DIM (avis) != NULL) {
            AVIS_DIM (avis) = TRAVdo (AVIS_DIM (avis), NULL);
        }
        if (AVIS_SHAPE (avis) != NULL) {
            AVIS_SHAPE (avis) = TRAVdo (AVIS_SHAPE (avis), NULL);
        }
        if (AVIS_MINVAL (avis) != NULL) {
            MarkAvisAlive (AVIS_MINVAL (avis));
        }
        if (AVIS_MAXVAL (avis) != NULL) {
            MarkAvisAlive (AVIS_MAXVAL (avis));
        }
    }

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("DCIfundef");

    DBUG_PRINT ("DCI",
                ("\nstarting dead code inference in fundef %s.", FUNDEF_NAME (arg_node)));

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

            /*
             * Traverse ARGS and VARDECS to initialize AVIS_ISDEAD
             */
            if (FUNDEF_VARDEC (arg_node) != NULL) {
                FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), info);
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            if (FUNDEF_ISLACFUN (arg_node)) {
                INFO_EXT_ASSIGN (info) = INFO_ASSIGN (arg_info);
            }

            while (!fixedpointreached) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

                fixedpointreached = TRUE;

                if ((INFO_TRAVSCOPE (info) == TS_function)
                    && (FUNDEF_ISDOFUN (arg_node))) {
                    node *args, *recexprs;
                    args = FUNDEF_ARGS (arg_node);
                    recexprs = AP_ARGS (ASSIGN_RHS (INFO_INT_ASSIGN (info)));

                    while (args != NULL) {
                        if ((!AVIS_ISDEAD (ARG_AVIS (args)))
                            && (AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (recexprs))))) {
                            MarkAvisAlive (ID_AVIS (EXPRS_EXPR (recexprs)));
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
    DBUG_ENTER ("DCIarg");

    AVIS_ISDEAD (ARG_AVIS (arg_node)) = TRUE;

    if (!FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        MarkAvisAlive (ARG_AVIS (arg_node));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("DCIvardec");

    AVIS_ISDEAD (VARDEC_AVIS (arg_node)) = TRUE;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("DCIblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("DCIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* traverse instruction */
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIreturn(node *arg_node , info *arg_info)
 *
 * description:
 *   starts traversal of return expressions to mark them as needed.
 *
 *****************************************************************************/
node *
DCIreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCIreturn");

    if ((INFO_TRAVSCOPE (arg_info) == TS_function)
        && (FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))) {
        node *extids, *retexprs;

        /* mark only those return values as needed that are required in the
           applying context */
        extids = ASSIGN_LHS (INFO_EXT_ASSIGN (arg_info));
        retexprs = RETURN_EXPRS (arg_node);

        while (extids != NULL) {
            if (!AVIS_ISDEAD (IDS_AVIS (extids))) {
                MarkAvisAlive (ID_AVIS (EXPRS_EXPR (retexprs)));
            }
            extids = IDS_NEXT (extids);
            retexprs = EXPRS_NEXT (retexprs);
        }
    } else {
        /* mark all returned identifiers as needed */
        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
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
    DBUG_ENTER ("DCIcond");

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
    DBUG_ENTER ("DCIlet");

    INFO_ONEIDSNEEDED (arg_info) = FALSE;
    INFO_ALLIDSNEEDED (arg_info) = FALSE;

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    /*
     * accu() must never become dead code
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
        && (PRF_PRF (LET_EXPR (arg_node)) == F_accu)) {
        INFO_ONEIDSNEEDED (arg_info) = TRUE;
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
            if (LET_IDS (arg_node) != NULL) {
                LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
            }
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
 *   if application of special function (cond, do) traverse into this
 *   function except for recursive calls of the current function.
 *   traverse all arguments to marks them as needed
 *
 *****************************************************************************/
node *
DCIap (node *arg_node, info *arg_info)
{
    node *extids, *recids;

    DBUG_ENTER ("DCIap");

    /* traverse special fundef without recursion */
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
                    MarkAvisAlive (IDS_AVIS (recids));
                }
                extids = IDS_NEXT (extids);
                recids = IDS_NEXT (recids);
            }

            /* do not mark the recursive parameters as needed in order not to
               create excessive demand */
        } else {
            node *args, *argexprs;

            DBUG_PRINT ("DCI", ("traverse in special fundef %s",
                                FUNDEF_NAME (AP_FUNDEF (arg_node))));

            /* start traversal of special fundef (and maybe reduce parameters!) */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("DCI", ("traversal of special fundef %s finished"
                                "continue in fundef %s\n",
                                FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                FUNDEF_NAME (INFO_FUNDEF (arg_info))));

            /* mark only those variables needed by the special function as live */
            args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            argexprs = AP_ARGS (arg_node);

            while (args != NULL) {
                if (!AVIS_ISDEAD (ARG_AVIS (args))) {
                    MarkAvisAlive (ID_AVIS (EXPRS_EXPR (argexprs)));
                }

                args = ARG_NEXT (args);
                argexprs = EXPRS_NEXT (argexprs);
            }
        }
    } else {
        /* mark all parameters of regular functions as needed */
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        }
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
    DBUG_ENTER ("DCIid");

    /* Mark identifier as needed */
    MarkAvisAlive (ID_AVIS (arg_node));

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
    DBUG_ENTER ("DCIids");

    if (INFO_ALLIDSNEEDED (arg_info)) {
        /* Mark identifier as needed */
        MarkAvisAlive (IDS_AVIS (arg_node));
    }

    if (!AVIS_ISDEAD (IDS_AVIS (arg_node))) {
        INFO_ONEIDSNEEDED (arg_info) = TRUE;
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("DCIcode");

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
    DBUG_ENTER ("DCIwithid");

    INFO_ALLIDSNEEDED (arg_info) = TRUE;

    /* Traverse sons */
    arg_node = TRAVcont (arg_node, arg_info);

    INFO_ALLIDSNEEDED (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCIwlsegvar(node *arg_node , info *arg_info)
 *
 * description:
 *   Traverses sons and additionally all IDX_MIN and IDX_MAX attributes
 *
 *****************************************************************************/
node *
DCIwlsegvar (node *arg_node, info *arg_info)
{
    int d;

    DBUG_ENTER ("DCIwlsegvar");

    DBUG_ASSERT ((WLSEGVAR_IDX_MIN (arg_node) != NULL), "WLSEGVAR_IDX_MIN not found!");
    DBUG_ASSERT ((WLSEGVAR_IDX_MAX (arg_node) != NULL), "WLSEGVAR_IDX_MAX not found!");
    for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
        (WLSEGVAR_IDX_MIN (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MIN (arg_node))[d], arg_info);
        (WLSEGVAR_IDX_MAX (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MAX (arg_node))[d], arg_info);
    }

    /* Traverse sons */
    arg_node = TRAVcont (arg_node, arg_info);

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
    DBUG_ENTER ("DCIrange");

    /* mark index and offsets alive */
    INFO_ALLIDSNEEDED (arg_info) = TRUE;
    RANGE_INDEX (arg_node) = TRAVdo (RANGE_INDEX (arg_node), arg_info);
    RANGE_IDXS (arg_node) = TRAVopt (RANGE_IDXS (arg_node), arg_info);
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
