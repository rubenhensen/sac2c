/*
 * $Log$
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
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "SSAConstantFolding.h"
#include "constants.h"
#include "optimize.h"

/*
 * constant identifiers should be substituted by its constant value
 * also in the arg-chain of N_ap and N_prf nodes.
 */
#define SUBST_ID_WITH_CONSTANT_IN_AP_ARGS TRUE

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

/* functions for internal use only */

/* traversal functions */

/******************************************************************************
 *
 * function:
 *   node* SSACFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block in this order
 *
 *
 ******************************************************************************/
node *
SSACFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfundef");

    INFO_SSACF_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
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

    /* ##nmw## to be implemented */

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

    INFO_SSACF_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    remove_assignment = INFO_SSACF_REMASSIGN (arg_info);

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remove_assignment) {
        /* skip this assignment and free it */
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
 *   checks for constant conditional - removes corresponding part or the whole
 *   conditional.
 *   traverses conditional, then-part, else-part
 *
 ******************************************************************************/
node *
SSACFcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "missing condition in conditional");

    /* traverse condition to analyse for constant expression */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* check for constant condition ##nmw## to be implemented */

    /* traverse then-part */
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    /* traverse else-part */
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFdo(node *arg_node, node *arg_info)
 *
 * description:
 *   this function should never be called in ssa-form.
 *
 *
 ******************************************************************************/
node *
SSACFdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFdo");

    DBUG_ASSERT ((FALSE), "there must not be any do-nodes in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   this function should never be called in ssa-form.
 *
 *
 ******************************************************************************/
node *
SSACFwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFwhile");

    DBUG_ASSERT ((FALSE), "there must not be any while-nodes in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   propagate constant return vales from special functions back in calling
 *   context.
 *
 *
 ******************************************************************************/
node *
SSACFreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFreturn");

    /* do NOT substitue constant identifiers with their value */
    INFO_SSACF_INSCONST (arg_info) = FALSE;
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
 *
 *
 ******************************************************************************/
node *
SSACFlet (node *arg_node, node *arg_info)
{
    constant *new_co;
    DBUG_ENTER ("SSACFlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    if ((LET_IDS (arg_node) != NULL)
        && (AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) == NULL)) {
        /* left side is not maked as constant -> compute expression */

        /* traverse expression to calculate constants */
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            /* handling for constant propagation in special fundefs and back */
            /* to be implemented */

        } else {
            /* set AVIS_SSACONST attributes */
            DBUG_ASSERT ((IDS_NEXT (LET_IDS (arg_node)) == NULL),
                         "only one result allowed for non N_ap nodes");

            new_co = COAST2Constant (LET_EXPR (arg_node));
            if (new_co != NULL) {
                AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) = new_co;
                DBUG_PRINT ("SSACF", ("identifier %s marked as constant",
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        IDS_AVIS (LET_IDS (arg_node))))));
            } else {
                /* expression is not constant */
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
 *   traverse in special function to propagate constants in both directions.
 *
 *
 ******************************************************************************/
node *
SSACFap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSACFap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /* substitute scalar constants in arguments */
    INFO_SSACF_INSCONST (arg_info) = SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    /* traverse special fundef without recursion */
    if (((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun))
        && (AP_FUNDEF (arg_node) != INFO_SSACSE_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSACF", ("traverse in special fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSACF_DEPTH (new_arg_info) = INFO_SSACF_DEPTH (arg_info) + 1;

        /* propagate constant args to called special function */
        /* to be implemented ##nmw## */

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        /* check for constant return values an assign them to result variables */
        /* to be implemented ##nmw## */

        DBUG_PRINT ("SSACF", ("traversal of special fundef %s finished\n",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSACF", ("do not traverse in normal fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFid(node *arg_node, node *arg_info)
 *
 * description:
 *   substitute scalar identifers with their computed constant
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
    if ((TYPES_DIM (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (arg_node)))) == SCALAR)
        && (AVIS_SSACONST (ID_AVIS (arg_node)) != NULL)
        && (INFO_SSACF_INSCONST (arg_info))) {
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
 *   node* SSACFnum(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFnum");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFfloat(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFfloat (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfloat");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFdouble(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFdouble (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFdouble");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFchar(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFchar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFchar");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFbool(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFbool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFbool");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFstr(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFstr");

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
        break;

    case F_shape:
        break;

    case F_reshape:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COReshape (arg1, arg2);
        }
        break;

    case F_psi:
    case F_idx_psi:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COPsi (arg1, arg2);
        }
        break;

    case F_modarray:
    case F_idx_modarray:
        break;

    case F_take:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COTake (arg1, arg2);
        }
        break;

    case F_drop:
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
 *
 ******************************************************************************/
static ids *
SSACFids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSACFids");

    /* do something useful */

    if (IDS_NEXT (arg_ids) != NULL) {
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
