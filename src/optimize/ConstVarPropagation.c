/*
 *
 * $Log$
 * Revision 1.1  2004/03/02 16:58:34  mwe
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "SSATransform.h"

#include "ConstVarPropagation.h"

/*
 * optimization phase: Constant and Variable Propagation (CVP)
 *
 * CVP is used to eliminate the usage of assignment arguments (RHS N_id nodes),
 * whose definition has only a single value (N_id, N_num, ...) as a right hand side.
 * So we can avoid the usage of unnecessary copy assignments.
 *
 * Example:
 *    a = 7;                a = 7;
 *    b = a;          =>    b = 7;
 *    c = fun(b);           c = fun(7);
 *
 * Obsolet definitions of assignments are removed by the DeadCodeRemoval.
 *
 * Implementation:
 *   This optimization phase is rather simple.
 *   For every function (fundef-node) we start a top-down traversal of the AST.
 *   We traverse in every assignment until we reach an id node. For every id node we
 *   ask a 'PropagationOracle' (function: AskPropagationOracle) if we should
 *   propagate the right hand side of the definition of this id node.
 *   According to the answer we do a propagation or not (function: CVPid).
 *   The PropagationOracle uses the 'context' in which the id is used, to make a decision.
 *   (see: enumeration type: context_t)
 */

/*
 * NOTE: Similar optimizations are implemented in SSACSE.[ch] and SSAConstantFolding.[ch].
 *       CVP should separate the constant and variable propagation out of these
 *       optimizations, because the scope between them is different.
 *       But because the implementation of SSACSE and SSAConstantFolding remains
 * untouched, they still provide these functionality. So its enough to change the
 * implemenation of this file if you want to modify the constant and variable propagation.
 * Only if the implementation of SSACSE or SSAConstantFolding interfere with your
 * modifications, you have to take a look at these files...
 */

/*
 * enumeration type: context_t
 *
 * 'context' is used to specifiy the special context in which the
 * right hand of an assignment is used. It should describe where and how the right hand
 * of the assignment is used.
 * The context could be used by an oracle, which decides whether a propagation
 * of a value should take place or not. So it is possible to define differnt behaviours
 * for different contexts.
 * If you want to change the behaviour for a special context (for example: no constant
 * propagation into return nodes => (context == nreturn)), so you have to find the
 * corresponding entry in the function 'AskPropagationOracle' and change it according to
 * your needs. If no corresponding context exists, you have to expand 'context' with a new
 * entry (example: nreturn),  you have to set the context appropriate while the
 * AST-traversal (example: set INFO_CVP_CONTEXT to return when traversing into a N_return
 * node) and add a new 'case' statement to 'AskPropagationOracle'.
 *
 * Following contexts exits:
 *   undef:           means undefined context, is set while traversing into an N_assign
 * node, is used to check if for every right hand side a defined context exists. if the
 * oracle gets an undefined context an assertion is raised let:             is set when
 * traversing from an assign in a let node nreturn:         is set when traversing in a
 * return node ap:              is set when traversing in an ap node primfun:         is
 * set when traversing in a prf node array:           is set when traversing in a array
 * node specialfun:      is set when traversing in a special function (do- or while loop,
 * condition) withloop:        is set when traversing in a nwith node withloop_cexprs: is
 * set when traversing in the cexprs chain of a withloop
 *
 *
 */
typedef enum {
    CON_return,
    CON_let,
    CON_withloop,
    CON_withloop_cexprs,
    CON_array,
    CON_ap,
    CON_primfun,
    CON_specialfun,
    CON_undef
} context_t;

/****************************************************************************
 *
 * function:
 *   bool IsConstant(node *arg_node)
 *
 * description:
 *   returns TRUE if arg_node is of type num, double, float, bool or char
 *   (nodes with a constant value)
 *   otherwise returns FALSE
 *
 *****************************************************************************/
static bool
IsConstant (node *arg_node)
{
    bool ret;
    DBUG_ENTER ("IsConstant");

    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_double:
    case N_float:
    case N_bool:
    case N_char:
        ret = TRUE;
        break;

    default:
        ret = FALSE;
    }

    DBUG_RETURN (ret);
}

/****************************************************************************
 *
 * function:
 *   bool IsVariable(node *arg_node)
 *
 * description:
 *   returns TRUE if arg_node is of type id
 *   (nodes with a variable value)
 *   otherwise returns FALSE
 *
 *****************************************************************************/
static bool
IsVariable (node *arg_node)
{

    bool ret;
    DBUG_ENTER ("IsVariable");

    if (NODE_TYPE (arg_node) == N_id)
        ret = TRUE;
    else
        ret = FALSE;

    DBUG_RETURN (ret);
}

/****************************************************************************
 *
 * function:
 *   bool AskPropagationOracle(node *let, node* arg_info)
 *
 * description:
 *   This function decides if a propagation of values should take place.
 *   For the decision it uses the information given by the context
 *   and the let-node itsself.
 *   If the oracle finds a node used in the 'undef' context or an unknown context
 *   (default) then an assertion is raised. So take care that all possible
 *   traversals of the syntax tree lead to a defined context.
 *
 *****************************************************************************/
static bool
AskPropagationOracle (node *let, node *arg_info)
{

    bool answer;

    DBUG_ENTER ("AskPropagationOracle");

    switch (INFO_CVP_CONTEXT (arg_info)) {
    case CON_array:
    case CON_ap:
    case CON_primfun:
    case CON_let:
    case CON_withloop:
        /* TRUE iff behind let node is constant value or an id node */
        answer = IsConstant (LET_EXPR (let)) || IsVariable (LET_EXPR (let));
        break;
    case CON_return:
    case CON_withloop_cexprs:
    case CON_specialfun:
        /* TRUE iff behind let node is an id node */
        answer = IsVariable (LET_EXPR (let));
        break;
    case CON_undef:
        /* for compiler */
        answer = FALSE;
        DBUG_ASSERT (FALSE, "found let node in an undefined context");
    default:
        /* for compiler */
        answer = FALSE;
        DBUG_ASSERT (FALSE, "found invalid or not treated context");
    }

    /* prevent propagation of unique variables */
    answer = !(INFO_CVP_ATTRIB (arg_info) == ST_unique) & answer;

    DBUG_RETURN (answer);
}

/********************************************************************
 *
 * function:
 *   node* CVParray(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the elements of an array node
 *
 ********************************************************************/

node *
CVParray (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVParray");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_array;
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPreturn(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the exprs of the return node
 *
 ********************************************************************/

node *
CVPreturn (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_return;
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEid( node *arg_node, node *arg_info)
 *
 * description:
 *   First we try to look inside the definition of the id node
 *   If we found the definition we ask the PropagationOracle if we
 *   should propagate a value or not.
 *   If the oracle says 'yes' (or TRUE), then we delete the current assign node
 *   and copy the node behind the let-node of the definition. This copy should
 *   be an id-node or one of the nodes with a constant value (N_num,...).
 *   This duplicated node is returned as the result of this function.
 *   If the oracle says 'no' (or FALSE), then we do nothing and return the
 *   current id node.
 *
 *****************************************************************************/
node *
CVPid (node *arg_node, node *arg_info)
{
    node *avis, *let;

    DBUG_ENTER ("CVPid");

    avis = IDS_AVIS (ID_IDS (arg_node));
    if ((avis != NULL) && (AVIS_SSAASSIGN (avis) != NULL)) {

        INFO_CVP_ATTRIB (arg_info) = VARDEC_OR_ARG_ATTRIB (AVIS_VARDECORARG (avis));
        let = ASSIGN_INSTR (AVIS_SSAASSIGN (avis));

        if (AskPropagationOracle (let, arg_info)) {

            arg_node = FreeNode (arg_node);
            arg_node = DupNode (LET_EXPR (let));

            cvp_expr++;
        }
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPexprs(node* arg_node, node* arg_info)
 *
 * description:
 *   first traverse in the expr-node and then
 *   traverse in the next node in the exprs-chain
 *
 ********************************************************************/

node *
CVPexprs (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPexprs");

    if (EXPRS_EXPR (arg_node) != NULL) {
        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPprf(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the arguments of the prf node
 *
 ********************************************************************/

node *
CVPprf (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPprf");

    INFO_CVP_CONTEXT (arg_info) = CON_primfun;

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPap(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the arguments of the prf node
 *
 ********************************************************************/

node *
CVPap (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPap");

    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))) {
        INFO_CVP_CONTEXT (arg_info) = CON_specialfun;
    } else {
        INFO_CVP_CONTEXT (arg_info) = CON_ap;
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   traverses condition, then- and else-part (in this order).
 *
 *****************************************************************************/

node *
CVPcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CVPcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse NPart, Nwithop and NCode in this order
 *
 *
 *****************************************************************************/
node *
CVPNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CVPNwith");

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;

    /* traverse and do variable substitution in partitions */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /* traverse and do variable substitution in withops */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse and do cse in code blocks */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse codeblock and expression for each Ncode node
 *
 *
 *****************************************************************************/
node *
CVPNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CVPNcode");

    /* traverse codeblock */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /*traverse expression to do variable substitution */
    INFO_CVP_CONTEXT (arg_info) = CON_withloop_cexprs;
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse to next node */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPlet(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the expr of the let node
 *
 ********************************************************************/

node *
CVPlet (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPlet");

    if (LET_EXPR (arg_node) != NULL) {

        INFO_CVP_CONTEXT (arg_info) = CON_let;
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPassign(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the instr of the assign node
 *
 ********************************************************************/

node *
CVPassign (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPassign");

    INFO_CVP_CONTEXT (arg_info) = CON_undef;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPblock(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the instructions of the block node
 *
 ********************************************************************/

node *
CVPblock (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPfundef(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse in the body of the fundef node
 *
 ********************************************************************/

node *
CVPfundef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CVPfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* ConstVarPropagation(node* arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
ConstVarPropagation (node *arg_node)
{

    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("ConstVarPropagation");
    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = cvp_tab;

    arg_node = Trav (arg_node, arg_info);

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    DBUG_RETURN (arg_node);
}
