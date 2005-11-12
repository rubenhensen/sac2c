/*
 *
 * $Id$
 *
 */
#include "deadcodeinference.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"

/*
 * INFO structure
 */
struct INFO {
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

    result = ILIBmalloc (sizeof (info));

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

    info = ILIBfree (info);

    DBUG_RETURN (info);
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

        if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info != NULL)) {
            info *info;
            bool fixedpointreached = FALSE;

            info = MakeInfo ();

            INFO_FUNDEF (info) = arg_node;

            /*
             * Traverse ARGS and VARDECS to initialize AVIS_ISDEAD
             */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            if (FUNDEF_VARDEC (arg_node) != NULL) {
                FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), info);
            }

            if (FUNDEF_ISLACFUN (arg_node)) {
                INFO_EXT_ASSIGN (info) = INFO_ASSIGN (arg_info);
            }

            while (!fixedpointreached) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

                fixedpointreached = TRUE;

                if (FUNDEF_ISDOFUN (arg_node)) {
                    node *args, *recexprs;
                    args = FUNDEF_ARGS (arg_node);
                    recexprs = AP_ARGS (ASSIGN_RHS (INFO_INT_ASSIGN (info)));

                    while (args != NULL) {
                        if ((!AVIS_ISDEAD (ARG_AVIS (args)))
                            && (AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (recexprs))))) {
                            AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (recexprs))) = FALSE;
                            DBUG_PRINT ("DCI", ("marking var %s as alive",
                                                ID_NAME (EXPRS_EXPR (recexprs))));
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

    if (FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        AVIS_ISDEAD (ARG_AVIS (arg_node)) = TRUE;
        DBUG_PRINT ("DCI", ("marking argument %s as dead", ARG_NAME (arg_node)));
    } else {
        AVIS_ISDEAD (ARG_AVIS (arg_node)) = FALSE;
        DBUG_PRINT ("DCI", ("marking argument %s as alive", ARG_NAME (arg_node)));
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
    DBUG_PRINT ("DCI", ("marking vardec %s as dead", VARDEC_NAME (arg_node)));

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

    if (FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        node *extids, *retexprs;

        /* mark only those return values as needed that are required in the
           applying context */
        extids = ASSIGN_LHS (INFO_EXT_ASSIGN (arg_info));
        retexprs = RETURN_EXPRS (arg_node);

        while (extids != NULL) {
            if (!AVIS_ISDEAD (IDS_AVIS (extids))) {
                AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (retexprs))) = FALSE;
                DBUG_PRINT ("DCI",
                            ("marking var %s as alive", ID_NAME (EXPRS_EXPR (retexprs))));
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

        if (!((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
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
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
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
                    AVIS_ISDEAD (IDS_AVIS (recids)) = FALSE;
                    DBUG_PRINT ("DCI", ("marking var %s as alive", IDS_NAME (recids)));
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
                    AVIS_ISDEAD (ID_AVIS (EXPRS_EXPR (argexprs))) = FALSE;
                    DBUG_PRINT ("DCI", ("marking var %s as alive",
                                        ID_NAME (EXPRS_EXPR (argexprs))));
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
    AVIS_ISDEAD (ID_AVIS (arg_node)) = FALSE;
    DBUG_PRINT ("DCI", ("marking var %s as alive", ID_NAME (arg_node)));

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
        AVIS_ISDEAD (IDS_AVIS (arg_node)) = FALSE;
        DBUG_PRINT ("DCI", ("marking var %s as alive", IDS_NAME (arg_node)));
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
