/*
 * $Log$
 * Revision 1.3  2001/03/29 16:31:21  nmw
 * Constant Folding for Loops and Conditionals implemented
 *
 * Revision 1.2  2001/03/22 14:30:18  nmw
 * constant folding for primitive ari ops implemented
 *
 * Revision 1.1  2001/03/20 16:16:54  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSAConstantFolding.c
 *
 * prefix: SSACF
 *
 * description:
 *   this module does constant folding on code in ssa form.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "SSAConstantFolding.h"
#include "constants.h"
#include "optimize.h"
#include "SSATransform.h"
#include "Inline.h"

/*
 * constant identifiers should be substituted by its constant value
 * also in the arg-chain of N_ap and N_prf nodes.
 */
#define SUBST_ID_WITH_CONSTANT_IN_AP_ARGS TRUE

#define SUBST_NONE 0
#define SUBST_SCALAR 1
#define SUBST_SCALAR_AND_ARRAY 2

#define SSACF_TOPLEVEL 0

/* Prf-function name for debug output */
#ifndef DBUG_OFF
#define PRF_IF(n, s, x, y) x
static char *prf_string[] = {
#include "prf_node_info.mac"
};
#undef PRF_IF
#endif

/* traversal like functions for ids structures */
static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACFids (ids *arg_ids, node *arg_info);
static node *SSACFPropagateConstants2Args (node *arg_chain, node *param_chain);

/* functions for internal use only */
/******************************************************************************
 *
 * function:
 *   node *SSACFPropagateConstants2Args(node *arg_chain, node *const_arg_chain)
 *
 *
 * description:
 *   to propagate constant expressions from the calling context into a special
 *   function, this functions does a parallel traversal of the function args
 *   (stored in arg_chain) and the calling parameters (stored param_chain).
 *
 *
 *
 ******************************************************************************/
static node *
SSACFPropagateConstants2Args (node *arg_chain, node *param_chain)
{
    node *arg;
    DBUG_ENTER ("SSACFPropagateConstants2Args");

    arg = arg_chain;
    while (arg != NULL) {
        DBUG_ASSERT ((param_chain != NULL),
                     "different arg chains in fun definition/fun application");

        if (AVIS_SSACONST (ARG_AVIS (arg)) == NULL) {
            /* arg not marked as constant - try to make new constant */
            AVIS_SSACONST (ARG_AVIS (arg)) = COAST2Constant (EXPRS_EXPR (param_chain));
        }

        /* traverse both chains */
        arg = ARG_NEXT (arg);
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_RETURN (arg_chain);
}

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* SSACFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block in this order.
 *   the args are only traversed in do/while special functions to remove
 *   propagated constants from loop dependend arguments.
 *
 ******************************************************************************/
node *
SSACFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfundef");

    INFO_SSACF_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_ARGS (arg_node) != NULL)
        && ((FUNDEF_STATUS (arg_node) == ST_dofun)
            || (FUNDEF_STATUS (arg_node) == ST_whilefun))) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* save block for inserting vardecs during the traversal */
        INFO_SSACF_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);

        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses vardecs and instructions in this order.
 *
 *
 ******************************************************************************/
node *
SSACFblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFblock");

    /*
    if (BLOCK_VARDEC(arg_node) != NULL) {
      BLOCK_VARDEC(arg_node) = Trav(BLOCK_VARDEC(arg_node), arg_info);
    }
    */

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarg(node *arg_node, node *arg_info)
 *
 * description:
 *   checks if only loop invariant arguments are constant (if special do-loop
 *   or while-loop fundef).
 *
 *
 ******************************************************************************/
node *
SSACFarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarg");

    /* constants for non loop invarinat args are useless */
    if ((!(AVIS_SSALPINV (ARG_AVIS (arg_node)))
         && (AVIS_SSACONST (ARG_AVIS (arg_node)) != NULL))) {
        /* free constant */
        AVIS_SSACONST (ARG_AVIS (arg_node))
          = COFreeConstant (AVIS_SSACONST (ARG_AVIS (arg_node)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFvardec");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFassign(node *arg_node, node *arg_info)
 *
 * description:
 *   top-down traversal of assignments. in bottom-up return traversal remove
 *   marked assignment-nodes from chain and insert moved assignments (e.g.
 *   from constant conditionals)
 *
 *
 ******************************************************************************/
node *
SSACFassign (node *arg_node, node *arg_info)
{
    bool remove_assignment;
    node *tmp;

    DBUG_ENTER ("SSACFassign");

    /* init flags for possible code removal/movement */
    INFO_SSACF_REMASSIGN (arg_info) = FALSE;
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* save removal flag for bottom-up traversal */
    remove_assignment = INFO_SSACF_REMASSIGN (arg_info);

    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = AppendAssign (INFO_SSACF_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remove_assignment) {
        /* skip this assignment and free it */
        DBUG_PRINT ("SSACF", ("remove dead assignment"));
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);

        ASSIGN_NEXT (tmp) = NULL;
        FreeTree (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFcond(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for constant conditional - removes corresponding of the
 *   conditional.
 *   traverses conditional and optional then-part, else-part
 *
 ******************************************************************************/
node *
SSACFcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "missing condition in conditional");
    DBUG_ASSERT ((COND_THEN (arg_node) != NULL), "missing then part in conditional");
    DBUG_ASSERT ((COND_ELSE (arg_node) != NULL), "missing else part in conditional");

    /*
     * traverse condition to analyse for constant expression
     * and substitute constants with their values to get
     * a simple N_bool node for the condition (if constant)
     */
    INFO_SSACF_INSCONST (arg_info) = SUBST_SCALAR;
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    /* check for constant condition */
    if (NODE_TYPE (COND_COND (arg_node)) == N_bool) {
        if (BOOL_VAL (COND_COND (arg_node)) == TRUE) {
            /* select then part */
            INFO_SSACF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_THEN (arg_node));

            if (NODE_TYPE (INFO_SSACF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_SSACF_POSTASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_THEN (arg_node)) = NULL;
            }
        } else {
            /* select else part */
            INFO_SSACF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));

            if (NODE_TYPE (INFO_SSACF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_SSACF_POSTASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_ELSE (arg_node)) = NULL;
            }
        }
        /*
         * mark this assignment for removal, the selected code part will
         * be inserted behind this conditional assignment and traversed
         * for constant folding.
         */
        INFO_SSACF_REMASSIGN (arg_info) = TRUE;

        /*
         * because there can be only one conditional in a special function
         * now this special function contains no conditional and therefore
         * is no special function anymore. this function can now be inlined
         * without any problems.
         * if this is a do- or while function and the condition is evaluated
         * to true we have an endless loop and will rise an error message.
         */
        if ((BOOL_VAL (COND_COND (arg_node)) == TRUE)
            && ((FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_dofun)
                || (FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_whilefun))) {
            WARN (NODE_LINE (arg_node),
                  ("infinite loop detected, program may not terminate"));
            /* ex special function cannot be inlined */
            FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) = ST_regular;
        } else {
            /* ex special function can be simply inlined */
            FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) = ST_inlinefun;
        }

    } else {
        /*
         * no constant condition:
         * do constant folding in conditional
         * traverse then-part
         */
        if (COND_THEN (arg_node) != NULL) {
            COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        }

        /* traverse else-part */
        if (COND_ELSE (arg_node) != NULL) {
            COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   do NOT substitute identifiers in return statement with their values!
 *
 *
 ******************************************************************************/
node *
SSACFreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFreturn");

    /* do NOT substitue constant identifiers with their value */
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFlet(node *arg_node, node *arg_info)
 *
 * description:
 *   checks expression for constant value and sets corresponding AVIS_SSACONST
 *   attribute for later usage.
 *   if constant folding has eliminated the condtional in a special function
 *   this function can be inlined here, because it is no longer a special one.
 *
 ******************************************************************************/
node *
SSACFlet (node *arg_node, node *arg_info)
{
    constant *new_co;
    DBUG_ENTER ("SSACFlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    /* remove tag SSAPHITARGET if conditional has been removed */
    if ((FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_regular)
        || (FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_inlinefun)) {
        AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) = PHIT_NONE;
    }

    /*
     * left side is not marked as constant -> compute expression
     * if there is a special function application with mutiple results
     * the constant results will be removed by dead code removal
     * so this conditions holds here, too. for a general function
     * application there is no constant propagation allowed.
     */
    if ((LET_IDS (arg_node) != NULL)
        && (AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) == NULL)) {

        /* traverse expression to calculate constants */
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            /*
             * handling for constant back-propagation from special fundefs
             * traverse ids chain and result chain of the concering return
             * statement. for each constant identifier add a separate
             * assignent and substitute the result identifier in the
             * function application with a dummy identifier (that will be
             * removed by the dead code removal)
             */
            if (INFO_SSACF_RESULTS (arg_info) != NULL) {
                LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
            }

            /* called function can be inlined */
            if (INFO_SSACF_INLINEAP (arg_info)) {
                DBUG_PRINT ("SSACF", ("inline function %s in %s",
                                      FUNDEF_NAME (AP_FUNDEF (LET_EXPR (arg_node))),
                                      FUNDEF_NAME (INFO_SSACF_FUNDEF (arg_info))));

                INFO_SSACF_POSTASSIGN (arg_info)
                  = AppendAssign (InlineSingleApplication (arg_node,
                                                           INFO_SSACF_FUNDEF (arg_info),
                                                           INL_COUNT),
                                  INFO_SSACF_POSTASSIGN (arg_info));

                FUNDEF_STATUS (AP_FUNDEF (LET_EXPR (arg_node))) = ST_regular;

                /* remove this assignment */
                INFO_SSACF_REMASSIGN (arg_info) = TRUE;

                INFO_SSACF_INLINEAP (arg_info) = FALSE;
            }

        } else {
            /* set AVIS_SSACONST attributes */
            DBUG_ASSERT ((IDS_NEXT (LET_IDS (arg_node)) == NULL),
                         "only one result allowed for non N_ap nodes");

            /*
             * do not set SSACONST in phi target variables due to two different
             * definitions of this variable in one conditional.
             * ##nmw## maybe check for two equal constants?
             */
            if (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_COND) {
                new_co = NULL;
            } else {
                new_co = COAST2Constant (LET_EXPR (arg_node));
            }
            if (new_co != NULL) {
                AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) = new_co;
                DBUG_PRINT ("SSACF", ("identifier %s marked as constant",
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        IDS_AVIS (LET_IDS (arg_node))))));
            } else {
                /* expression is not constant */
                DBUG_PRINT ("SSACF", ("identifier %s is not constant",
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        IDS_AVIS (LET_IDS (arg_node))))));
            }
        }
    } else {
        /* left side is already maked as constant - no further processing needed */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFap(node *arg_node, node *arg_info)
 *
 * description:
 *   propagate constants and traverse in special function
 *
 ******************************************************************************/
node *
SSACFap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSACFap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /* substitute scalar constants in arguments (if no special function)*/
    if ((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
        || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun)
        || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)) {
        INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;
    } else {
        INFO_SSACF_INSCONST (arg_info)
          = SUBST_SCALAR && SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    }

    /* traverse arg chain */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    /* traverse special fundef without recursion */
    if (((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun))
        && (AP_FUNDEF (arg_node) != INFO_SSACF_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSACF", ("traverse in special fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSACF_DEPTH (new_arg_info) = INFO_SSACF_DEPTH (arg_info) + 1;

        /* propagate constant args to called special function */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = SSACFPropagateConstants2Args (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                          AP_ARGS (arg_node));

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        /* save exprs chain of return list for later propagating constants */
        INFO_SSACF_RESULTS (arg_info)
          = RETURN_EXPRS (FUNDEF_RETURN (AP_FUNDEF (arg_node)));

        /* can this special function be inlined? */
        if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_inlinefun) {
            INFO_SSACF_INLINEAP (arg_info) = TRUE;
        } else {
            INFO_SSACF_INLINEAP (arg_info) = FALSE;
        }

        DBUG_PRINT ("SSACF", ("traversal of special fundef %s finished\n",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSACF", ("do not traverse in normal fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        INFO_SSACF_RESULTS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFid(node *arg_node, node *arg_info)
 *
 * description:
 *   substitute identifers with their computed constant
 *   ( only when INFO_SSACF_INSCONST flag is set)
 *   in EXPRS chain of N_ap ARGS ( if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_prf ARGS (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_array AELEMS
 *
 ******************************************************************************/
node *
SSACFid (node *arg_node, node *arg_info)
{
    node *new_node;
    DBUG_ENTER ("SSACFid");

    /* check for constant scalar identifier */
    if ((((TYPES_DIM (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (arg_node))))
           == SCALAR)
          && (INFO_SSACF_INSCONST (arg_info) >= SUBST_SCALAR))
         || ((TYPES_DIM (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (arg_node))))
              > SCALAR)
             && (INFO_SSACF_INSCONST (arg_info)) == SUBST_SCALAR_AND_ARRAY))
        && (AVIS_SSACONST (ID_AVIS (arg_node)) != NULL)) {
        DBUG_PRINT ("SSACF",
                    ("substitue identifier %s through its value",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));

        /* substitute identifier with its value */
        new_node = COConstant2AST (AVIS_SSACONST (ID_AVIS (arg_node)));
        arg_node = FreeTree (arg_node);
        arg_node = new_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarray(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses array elements to propagate constant identifiers
 *
 *
 ******************************************************************************/
node *
SSACFarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarray");

    /* substitute constant identifiers in array elements */
    INFO_SSACF_INSCONST (arg_info) = TRUE;
    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFprf(node *arg_node, node *arg_info)
 *
 * description:
 *   evaluates primitive function with constant paramters and substitutes
 *   the function application with its value.
 *
 *
 ******************************************************************************/
node *
SSACFprf (node *arg_node, node *arg_info)
{
    node *new_node;
    constant *new_co;
    constant *arg1;
    constant *arg2;

    DBUG_ENTER ("SSACFprf");

    DBUG_PRINT ("SSACF", ("evaluating prf %s", prf_string[PRF_PRF (arg_node)]));

    /* substitute constant identifiers in arguments */
    INFO_SSACF_INSCONST (arg_info) = SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    /* init local variables */
    new_node = NULL;
    new_co = NULL;

    /* try to convert args to constants */
    if (EXPRS_EXPR (PRF_ARGS (arg_node)) != NULL) {
        arg1 = COAST2Constant (EXPRS_EXPR (PRF_ARGS (arg_node)));
    } else {
        arg1 = NULL;
    }

    if ((EXPRS_NEXT (PRF_ARGS (arg_node)) != NULL)
        && (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) != NULL)) {
        arg2 = COAST2Constant (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))));
    } else {
        arg2 = NULL;
    }

    /* do constant folding on primitive functions */
    switch (PRF_PRF (arg_node)) {
        /* single-argument functions */
    case F_toi:
    case F_toi_A:
        if (arg1 != NULL) {
            new_co = COToi (arg1);
        }
        break;

    case F_tof:
    case F_tof_A:
        if (arg1 != NULL) {
            new_co = COTof (arg1);
        }
        break;

    case F_tod:
    case F_tod_A:
        if (arg1 != NULL) {
            new_co = COTod (arg1);
        }
        break;

    case F_abs:
        if (arg1 != NULL) {
            new_co = COAbs (arg1);
        }
        break;

    case F_not:
        if (arg1 != NULL) {
            new_co = CONot (arg1);
        }
        break;

        /* two-argument functions */
    case F_min:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMin (arg1, arg2);
        }
        break;

    case F_max:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMax (arg1, arg2);
        }
        break;

    case F_add:
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COAdd (arg1, arg2);
        }
        break;

    case F_sub:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COSub (arg1, arg2);
        }
        break;

    case F_mul:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMul (arg1, arg2);
        }
        break;

    case F_div:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CODiv (arg1, arg2);
        }
        break;

    case F_mod:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMod (arg1, arg2);
        }
        break;

    case F_and:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COAnd (arg1, arg2);
        }
        break;

    case F_or:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COOr (arg1, arg2);
        }
        break;

    case F_le:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COLe (arg1, arg2);
        }
        break;

    case F_lt:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COLt (arg1, arg2);
        }
        break;

    case F_eq:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COEq (arg1, arg2);
        }
        break;

    case F_ge:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COGe (arg1, arg2);
        }
        break;

    case F_gt:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COGt (arg1, arg2);
        }
        break;

    case F_neq:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CONeq (arg1, arg2);
        }
        break;

    case F_dim:
        /* for pure constant arg */
        if (arg1 != NULL) {
            new_co = CODim (arg1);
        }
        break;

    case F_shape:
        /* for pure constant arg */
        if (arg1 != NULL) {
            new_co = COShape (arg1);
        }
        break;

    case F_reshape:
        /* for pure constant arg */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COReshape (arg1, arg2);
        }
        break;

    case F_psi:
    case F_idx_psi:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COPsi (arg1, arg2);
        }
        break;

    case F_modarray:
    case F_idx_modarray:
        break;

    case F_take:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COTake (arg1, arg2);
        }
        break;

    case F_drop:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CODrop (arg1, arg2);
        }
        break;

    case F_cat:
        break;

    case F_rotate:
        break;

    default:
        DBUG_PRINT ("SSACF", ("no implementation in SSAConstantFolding for  prf %s",
                              prf_string[PRF_PRF (arg_node)]));
    }
    /* cf_expr++; */

    /* free used constant data */
    if (arg1 != NULL) {
        arg1 = COFreeConstant (arg1);
    }

    if (arg2 != NULL) {
        arg2 = COFreeConstant (arg2);
    }

    if (new_co != NULL) {
        /* create new node with constant value instead of prf node */
        new_node = COConstant2AST (new_co);
        new_co = COFreeConstant (new_co);

        /* free this primitive function */
        FreeTree (arg_node);
        arg_node = new_node;

        /* increment constant folding counter */
        cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFids(node *arg_ids, node *arg_info)
 *
 * description:
 *   traverse ids chain and return exprs chain (stored in INFO_SSACF_RESULT)
 *   and look for constant results.
 *   each constant identifier will be set in an separate assignment (added to
 *   INFO_SSACF_POSTASSIGN) and substituted in the function application with
 *   a new dummy identifier that can be removed by constant folding later.
 *
 ******************************************************************************/
static ids *
SSACFids (ids *arg_ids, node *arg_info)
{
    constant *new_co;
    node *assign_let;
    node *new_vardec;

    DBUG_ENTER ("SSACFids");

    DBUG_ASSERT ((INFO_SSACF_RESULTS (arg_info) != NULL),
                 "different ids and result chains");

    new_co = COAST2Constant (EXPRS_EXPR (INFO_SSACF_RESULTS (arg_info)));

    if (new_co != NULL) {
        DBUG_PRINT ("SSACF",
                    ("identifier %s marked as constant",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        AVIS_SSACONST (IDS_AVIS (arg_ids)) = new_co;

        /* create one let assign for constant definition */
        assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (
                                      AVIS_VARDECORARG (IDS_AVIS (arg_ids)))),
                                    AVIS_VARDECORARG (IDS_AVIS (arg_ids)),
                                    COConstant2AST (new_co));

        /* append new copy assignment to then-part block */
        INFO_SSACF_POSTASSIGN (arg_info)
          = AppendAssign (INFO_SSACF_POSTASSIGN (arg_info), assign_let);
        /* store definition assignment */
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = assign_let;

        DBUG_PRINT ("SSACF",
                    ("create constant assignment for %s",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        /* create new dummy identifier */
        new_vardec = SSANewVardec (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
        BLOCK_VARDEC (INFO_SSACF_TOPBLOCK (arg_info))
          = AppendVardec (BLOCK_VARDEC (INFO_SSACF_TOPBLOCK (arg_info)), new_vardec);

        /* rename this identifier */
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);
        IDS_VARDEC (arg_ids) = new_vardec;
#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_NAME (new_vardec));
#endif
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        INFO_SSACF_RESULTS (arg_info) = EXPRS_NEXT (INFO_SSACF_RESULTS (arg_info));
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSACFids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFNgen(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses parameter of generator to substitute constant arrays
 *   with their array representation to allow constant folding on known
 *   shape information.
 *
 ******************************************************************************/
node *
SSACFNgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFNgen");

    INFO_SSACF_INSCONST (arg_info) = SUBST_SCALAR_AND_ARRAY;
    DBUG_PRINT ("SSACF", ("substitute constant generator parameters"));

    if (NGEN_BOUND1 (arg_node) != NULL) {
        NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    }
    if (NGEN_BOUND2 (arg_node) != NULL) {
        NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    }
    if (NGEN_STEP (arg_node) != NULL) {
        NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node) != NULL) {
        NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSAConstantFolding(node* fundef)
 *
 * description:
 *   starts the DeadCodeRemoval for the given fundef. This fundef must not be
 *   a special fundef (these will be traversed in their order of application).
 *
 *
 ******************************************************************************/
node *
SSAConstantFolding (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSAConstantFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSAConstantFolding called for non-fundef node");

    DBUG_PRINT ("SSACF",
                ("starting constant folding in function %s", FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        INFO_SSACF_DEPTH (arg_info) = SSACF_TOPLEVEL; /* start on toplevel */

        old_tab = act_tab;
        act_tab = ssacf_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
