/*
 *
 * $Log$
 * Revision 1.22  2005/07/26 15:44:42  sah
 * CVP no more propagates constants in
 * default expression position of a wl
 *
 * Revision 1.21  2005/06/16 08:02:53  sbs
 * F_dispatch_error supported.
 *
 * Revision 1.20  2005/06/02 13:42:48  mwe
 * set SSAASSIGN if moving constant args in vardec chain
 *
 * Revision 1.19  2005/05/31 13:59:34  mwe
 * bug in CVPids fixed
 *
 * Revision 1.18  2005/04/21 06:32:47  ktr
 * Eliminated application of SSATnewVardec
 *
 * Revision 1.17  2005/02/14 16:10:10  mwe
 * propagate only constant scalar return values
 *
 * Revision 1.16  2005/02/14 15:51:48  mwe
 * CVPids added (moved from CFids), propagate constant return values
 *
 * Revision 1.14  2004/12/09 18:15:09  ktr
 * IsConstantArray fixed.
 *
 * Revision 1.13  2004/12/08 18:00:42  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.12  2004/11/26 13:14:48  ktr
 * Comment of CVPprf enhanced.
 *
 * Revision 1.11  2004/11/26 12:44:05  mwe
 * changes according to changes in ast.xml
 *
 * Revision 1.10  2004/10/22 15:40:08  ktr
 * CVP now propagates variables into branches of funcond.
 *
 * Revision 1.9  2004/10/15 11:40:08  ktr
 * Constant scalars and constant arrays are now propagated over special function
 * boundaries.
 *
 * Revision 1.8  2004/09/24 17:08:35  ktr
 * Bug #60: Constant arguments of PRFs are not propagted for all prfs any
 * longer.
 *
 * Revision 1.7  2004/08/11 13:15:10  ktr
 * QUICKFIX: CVP does not propagate into branches of funcond in order to prevent
 * problems with UndoSSATransform. (branches with final dec_rc statement)
 * Probably, UndoSSATransform should be adjusted.
 *
 * Revision 1.6  2004/07/23 13:58:18  ktr
 * Added CVPNwithop to prevent CVP to propagate the neutral element.
 *
 * Revision 1.5  2004/07/22 14:17:20  ktr
 * - Special functions are now traversed when they are used
 * - Constants are no longer propagated into funaps as this can introduce
 *   inefficiencies (precompile will lift constant args anyway).
 *
 * Revision 1.4  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.3  2004/03/06 21:17:49  mwe
 * CON_cond added
 *
 * Revision 1.2  2004/03/06 20:06:40  mwe
 * CVPfuncond added
 *
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
#include "node_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "constants.h"

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
 * node specialfun:      is set when traversing in a argumentlist of a special function
 *                    (do- or while loop, condition)
 *   withloop:        is set when traversing in a nwith node
 *   withloop_cexprs: is set when traversing in the cexprs chain of a withloop
 *   cond:            is set when traversing in the conditional of a condition
 *   neutral:         is set when traversing in the neutral element of fold-wl
 *   default:         is set when traversing in the default element of wl
 *   sel:             constant N_num arrays and N_ids arrays are allowed
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
    CON_cond,
    CON_funcond,
    CON_neutral,
    CON_default,
    CON_undef
} context_t;

/*
 * INFO structure
 */
struct INFO {
    context_t context;
    bool attrib;
    node *fundef;
    node *assign;
    node *postassign;
    bool specfun;
    bool singleids;
};

/*
 * INFO macros
 */
#define INFO_CVP_CONTEXT(n) (n->context)
#define INFO_CVP_ATTRIB(n) (n->attrib)
#define INFO_CVP_FUNDEF(n) (n->fundef)
#define INFO_CVP_ASSIGN(n) (n->assign)
#define INFO_CVP_POSTASSIGN(n) (n->postassign)
#define INFO_CVP_SPECFUN(n) (n->specfun)
#define INFO_CVP_SINGLEIDS(n) (n->singleids)
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CVP_FUNDEF (result) = NULL;
    INFO_CVP_CONTEXT (result) = CON_undef;
    INFO_CVP_ATTRIB (result) = FALSE;
    INFO_CVP_ASSIGN (result) = NULL;
    INFO_CVP_POSTASSIGN (result) = NULL;
    INFO_CVP_SPECFUN (result) = TRUE;
    INFO_CVP_SINGLEIDS (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

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
 *   bool IsConstantArray(node *arg_node)
 *
 * description:
 *   returns TRUE if arg_node is of type N_array and all elements are
 *   constants
 *   otherwise returns FALSE
 *
 *****************************************************************************/
static bool
IsConstantArray (node *arg_node)
{
    bool ret = TRUE;

    DBUG_ENTER ("IsConstantArray");

    if (NODE_TYPE (arg_node) == N_array) {
        node *elems = ARRAY_AELEMS (arg_node);

        while (ret && (elems != NULL)) {
            if (!IsConstant (EXPRS_EXPR (elems))) {
                ret = FALSE;
                break;
            }
            elems = EXPRS_NEXT (elems);
        }
    } else {
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
 *   bool AskPropagationOracle(node *let, info *arg_info)
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
AskPropagationOracle (node *let, info *arg_info)
{

    bool answer;

    DBUG_ENTER ("AskPropagationOracle");

    switch (INFO_CVP_CONTEXT (arg_info)) {

    case CON_let:
        answer = ((IsConstant (LET_EXPR (let))) || (IsConstantArray (LET_EXPR (let)))
                  || (IsVariable (LET_EXPR (let))));
        break;

    case CON_array:
    case CON_primfun:
    case CON_withloop:
    case CON_cond:
        /*
         * TRUE iff behind let node is constant value or an id node
         */
        answer = ((IsConstant (LET_EXPR (let))) || (IsVariable (LET_EXPR (let))));
        break;

    case CON_return:
    case CON_withloop_cexprs:
    case CON_specialfun:
    case CON_neutral:
    case CON_default:
    case CON_ap:
    case CON_funcond:
        /* TRUE iff behind let node is an id node */
        answer = IsVariable (LET_EXPR (let));
        break;

    case CON_undef:
        /* for compiler */
        answer = FALSE;
        DBUG_ASSERT (FALSE, "found let node in an undefined context");
        break;

    default:
        /* for compiler */
        answer = FALSE;
        DBUG_ASSERT (FALSE, "found invalid or not treated context");
    }

    /* prevent propagation of unique variables */
    answer = ((INFO_CVP_ATTRIB (arg_info) != TRUE) && (answer));

    DBUG_RETURN (answer);
}

/********************************************************************
 *
 * function:
 *   node *PropagateIntoCondArgs
 *
 * description:
 *   propagates constants into arguments of COND-FUNS
 *
 ********************************************************************/
static node *
PropagateIntoCondArgs (node *arg_node)
{
    node *condargs;
    node *apargs;

    DBUG_ENTER ("PropagateIntoCondArgs");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "arg_node must be N_ap");
    DBUG_ASSERT (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)),
                 "AP_FUNDEF( arg_node) must be a CONDFUN");

    condargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    apargs = AP_ARGS (arg_node);

    while (condargs != NULL) {
        node *avis = ID_AVIS (EXPRS_EXPR (apargs));
        if (AVIS_SSAASSIGN (avis) != NULL) {
            node *rhs = ASSIGN_RHS (AVIS_SSAASSIGN (avis));

            if ((IsConstant (rhs)) || (IsConstantArray (rhs))) {
                AVIS_SSAASSIGN (ARG_AVIS (condargs)) = AVIS_SSAASSIGN (avis);
            }
        }

        condargs = ARG_NEXT (condargs);
        apargs = EXPRS_NEXT (apargs);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node *RemovePropagationFromCondArgs
 *
 * description:
 *   removes propagated AVIS_SSAASSIGNs from arguments of COND-FUNS
 *
 ********************************************************************/
static node *
RemovePropagationFromCondArgs (node *arg_node)
{
    node *condargs;

    DBUG_ENTER ("RemovePropagationFromCondArgs");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_ap, "arg_node must be N_ap");
    DBUG_ASSERT (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)),
                 "AP_FUNDEF( arg_node) must be a CONDFUN");

    condargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    while (condargs != NULL) {
        AVIS_SSAASSIGN (ARG_AVIS (condargs)) = NULL;

        condargs = ARG_NEXT (condargs);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVParray( node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the elements of an array node
 *
 ********************************************************************/

node *
CVParray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVParray");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_array;
        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPreturn(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the exprs of the return node
 *
 ********************************************************************/

node *
CVPreturn (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_return;
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the exprs of the condfun node
 *
 ********************************************************************/

node *
CVPfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPfuncond");

    if (FUNCOND_IF (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_let;
        FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    }
    if (FUNCOND_THEN (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_funcond;
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    }
    if (FUNCOND_ELSE (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_funcond;
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPid( node *arg_node, info *arg_info)
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
CVPid (node *arg_node, info *arg_info)
{
    node *avis, *let;

    DBUG_ENTER ("CVPid");

    avis = ID_AVIS (arg_node);
    if ((avis != NULL) && (AVIS_SSAASSIGN (avis) != NULL)) {

        INFO_CVP_ATTRIB (arg_info) = AVIS_ISUNIQUE (avis);
        let = ASSIGN_INSTR (AVIS_SSAASSIGN (avis));

        if (AskPropagationOracle (let, arg_info)) {

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (LET_EXPR (let));

            cvp_expr++;
        }
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPexprs(node *arg_node, info *arg_info)
 *
 * description:
 *   first traverse in the expr-node and then
 *   traverse in the next node in the exprs-chain
 *
 ********************************************************************/

node *
CVPexprs (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPexprs");

    if (EXPRS_EXPR (arg_node) != NULL) {
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node* CVPprf(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the arguments of the prf node
 *   Some prf arguments must not become constants.
 *   E.g. The prf implementation of F_dim cannot handle constants at all!
 *
 *****************************************************************************/

node *
CVPprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPprf");

    /*
     * Depending on the primitive function, different arguments
     * are allowed to become constant
     */
    switch (PRF_PRF (arg_node)) {
    case F_dim:
    case F_shape:
    case F_accu:
    case F_type_error:
    case F_dispatch_error:
    case F_sel:
    case F_shape_sel:
        /*
         * Only propagate variables here
         */
        INFO_CVP_CONTEXT (arg_info) = CON_ap;
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    case F_idx_sel:
    case F_idx_shape_sel:
    case F_take_SxV:
    case F_drop_SxV:
        /*
         * Only the first argument may be constant
         */
        INFO_CVP_CONTEXT (arg_info) = CON_primfun;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_CVP_CONTEXT (arg_info) = CON_ap;
        EXPRS_EXPRS2 (PRF_ARGS (arg_node))
          = TRAVdo (EXPRS_EXPRS2 (PRF_ARGS (arg_node)), arg_info);
        break;

    case F_idx_modarray:
    case F_modarray:
        /*
         * The first argument of modarray/idx_modarray must be variable
         * the others can as well be constant scalars
         */
        INFO_CVP_CONTEXT (arg_info) = CON_ap;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_CVP_CONTEXT (arg_info) = CON_primfun;
        EXPRS_EXPRS2 (PRF_ARGS (arg_node))
          = TRAVdo (EXPRS_EXPRS2 (PRF_ARGS (arg_node)), arg_info);
        break;

    default:
        /*
         * In the default case, all arguments may become constant
         */
        INFO_CVP_CONTEXT (arg_info) = CON_primfun;

        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPap(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the arguments of the prf node
 *
 ********************************************************************/

node *
CVPap (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPap");

    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))) {
        /*
         * special functions must be traversed when they are used
         */
        if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
            arg_node = PropagateIntoCondArgs (arg_node);
        }
        if (AP_FUNDEF (arg_node) != INFO_CVP_FUNDEF (arg_info)) {
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        }
        if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
            arg_node = RemovePropagationFromCondArgs (arg_node);
        }
        INFO_CVP_CONTEXT (arg_info) = CON_specialfun;
    } else {
        INFO_CVP_CONTEXT (arg_info) = CON_ap;
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    if ((!FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (!FUNDEF_ISPROVIDED (AP_FUNDEF (arg_node)))
        && (!FUNDEF_ISEXPORTED (AP_FUNDEF (arg_node)))) {
        INFO_CVP_SPECFUN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPcond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   traverses condition, then- and else-part (in this order).
 *
 *****************************************************************************/

node *
CVPcond (node *arg_node, info *arg_info)
{

    context_t old;
    DBUG_ENTER ("CVPcond");

    old = INFO_CVP_CONTEXT (arg_info);
    INFO_CVP_CONTEXT (arg_info) = CON_cond;
    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    INFO_CVP_CONTEXT (arg_info) = old;

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPwith(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse NPart, Nwithop and NCode in this order
 *
 *
 *****************************************************************************/
node *
CVPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPwith");

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;

    /* traverse and do variable substitution in partitions */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    /* traverse and do variable substitution in withops */
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    /* traverse and do cse in code blocks */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPNwithop(node *arg_node, info *arg_info)
 *
 * description:
 *   don't propagate constants into neutral element
 *
 *
 *****************************************************************************/
node *
CVPgenarray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPgenarray");

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        INFO_CVP_CONTEXT (arg_info) = CON_default;
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        INFO_CVP_CONTEXT (arg_info) = CON_withloop;
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CVPmodarray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPmodarray");

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    INFO_CVP_CONTEXT (arg_info) = CON_withloop;
    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CVPfold (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPfold");

    INFO_CVP_CONTEXT (arg_info) = CON_neutral;
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPcode(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse codeblock and expression for each Ncode node
 *
 *
 *****************************************************************************/
node *
CVPcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPNcode");

    /* traverse codeblock */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /*traverse expression to do variable substitution */
    INFO_CVP_CONTEXT (arg_info) = CON_withloop_cexprs;
    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    /* traverse to next node */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the expr of the let node
 *
 ********************************************************************/

node *
CVPlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPlet");

    INFO_CVP_SPECFUN (arg_info) = TRUE;

    if (LET_EXPR (arg_node) != NULL) {

        INFO_CVP_CONTEXT (arg_info) = CON_let;
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if ((LET_IDS (arg_node) != NULL) && (!INFO_CVP_SPECFUN (arg_info))) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the instr of the assign node
 *
 ********************************************************************/

node *
CVPassign (node *arg_node, info *arg_info)
{

    node *oldassign;

    DBUG_ENTER ("CVPassign");

    INFO_CVP_CONTEXT (arg_info) = CON_undef;

    INFO_CVP_POSTASSIGN (arg_info) = NULL;
    oldassign = INFO_CVP_ASSIGN (arg_info);
    INFO_CVP_ASSIGN (arg_info) = arg_node;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    INFO_CVP_ASSIGN (arg_info) = oldassign;

    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = TCappendAssign (INFO_CVP_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_CVP_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the instructions of the block node
 *
 ********************************************************************/

node *
CVPblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CVPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the body of the fundef node
 *
 ********************************************************************/

node *
CVPfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ("CVPfundef");

    DBUG_PRINT ("CVP", ("Traversion function %s", FUNDEF_NAME (arg_node)));

    oldfundef = INFO_CVP_FUNDEF (arg_info);
    INFO_CVP_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if ((FUNDEF_ARGS (arg_node) != NULL) && (!FUNDEF_ISEXPORTED (arg_node))
        && (!FUNDEF_ISPROVIDED (arg_node)) && (!FUNDEF_ISLACFUN (arg_node))) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_CVP_FUNDEF (arg_info) = oldfundef;

    DBUG_PRINT ("CVP", ("Completed traversal of function %s", FUNDEF_NAME (arg_node)));

    DBUG_RETURN (arg_node);
}

/**********************************************************************
 *
 * function:
 *   node *CVParg(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse into arguments
 *
 *********************************************************************/

node *
CVParg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVParg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    if ((TYisAKV (AVIS_TYPE (ARG_AVIS (arg_node))))
        && (0 == TYgetDim (AVIS_TYPE (ARG_AVIS (arg_node))))) {

        /*
         * argument is a known scalar value
         * change argument to a local identifier
         */
        BLOCK_VARDEC (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info)))
          = TBmakeVardec (ARG_AVIS (arg_node),
                          BLOCK_VARDEC (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info))));

        BLOCK_INSTR (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info)))
          = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (arg_node), NULL),
                                     COconstant2AST (
                                       TYgetValue (AVIS_TYPE (ARG_AVIS (arg_node))))),
                          BLOCK_INSTR (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info))));

        AVIS_SSAASSIGN (ARG_AVIS (arg_node))
          = BLOCK_INSTR (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info)));
        AVIS_DECL (ARG_AVIS (arg_node))
          = BLOCK_VARDEC (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info)));

        /*
         * create dummy argument
         */
        ARG_AVIS (arg_node)
          = TBmakeAvis (ILIBtmpVar (), TYcopyType (AVIS_TYPE (ARG_AVIS (arg_node))));
        AVIS_DECL (ARG_AVIS (arg_node)) = arg_node;

        cvp_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPids(info *arg_ids, node *arg_info)
 *
 * description:
 *   traverse ids chain and return exprs chain (stored in INFO_CF_RESULT)
 *   and look for constant results.
 *   each constant identifier will be set in an separate assignment (added to
 *   INFO_CVP_POSTASSIGN) and substituted in the function application with
 *   a new dummy identifier that can be removed by constant folding later.
 *
 *****************************************************************************/

node *
CVPids (node *arg_ids, info *arg_info)
{
    constant *new_co;
    node *assign_let;
    node *new_vardec;

    DBUG_ENTER ("CVPids");

    if ((IDS_NEXT (arg_ids) != NULL) || (!INFO_CVP_SINGLEIDS (arg_info))) {
        if ((TYisAKV (AVIS_TYPE (IDS_AVIS (arg_ids))))
            && (0 == TYgetDim (AVIS_TYPE (IDS_AVIS (arg_ids))))) {
            new_co = TYgetValue (AVIS_TYPE (IDS_AVIS (arg_ids)));

            DBUG_PRINT ("CVP", ("identifier %s marked as constant",
                                VARDEC_OR_ARG_NAME (AVIS_DECL (IDS_AVIS (arg_ids)))));

            /*
             * create one let assign for constant definition,
             * reuse old avis/vardec
             */
            assign_let = TCmakeAssignLet (IDS_AVIS (arg_ids), COconstant2AST (new_co));
            AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = assign_let;

            /*
             * append new copy assignment to then-part block
             */
            INFO_CVP_POSTASSIGN (arg_info)
              = TCappendAssign (INFO_CVP_POSTASSIGN (arg_info), assign_let);

            DBUG_PRINT ("CVP",
                        ("create constant assignment for %s", (IDS_NAME (arg_ids))));

            /*
             * Replace current IDS_AVIS with dummy identifier
             */
            new_vardec = TBmakeVardec (TBmakeAvis (ILIBtmpVarName (IDS_NAME (arg_ids)),
                                                   TYeliminateAKV (IDS_NTYPE (arg_ids))),
                                       NULL);
            BLOCK_VARDEC (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info)))
              = TCappendVardec (BLOCK_VARDEC (FUNDEF_BODY (INFO_CVP_FUNDEF (arg_info))),
                                new_vardec);

            AVIS_SSAASSIGN (VARDEC_AVIS (new_vardec)) = INFO_CVP_ASSIGN (arg_info);
            IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);
        }
        INFO_CVP_SINGLEIDS (arg_info) = FALSE;
        if (NULL != IDS_NEXT (arg_ids)) {
            IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
        }
    } else {

        if ((INFO_CVP_SINGLEIDS (arg_info)) && (TYisAKV (AVIS_TYPE (IDS_AVIS (arg_ids))))
            && (0 == TYgetDim (AVIS_TYPE (IDS_AVIS (arg_ids))))) {
            new_co = TYgetValue (AVIS_TYPE (IDS_AVIS (arg_ids)));

            LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (IDS_AVIS (arg_ids))))
              = COconstant2AST (new_co);
        }
    }

    INFO_CVP_SINGLEIDS (arg_info) = TRUE;

    DBUG_RETURN (arg_ids);
}

/********************************************************************
 *
 * function:
 *   node* ConstVarPropagation(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
CVPdoConstVarPropagation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("ConstVarPropagation");
    arg_info = MakeInfo ();

    DBUG_PRINT ("OPT", ("starting constant var propagation in function %s",
                        FUNDEF_NAME (arg_node)));

    DBUG_PRINT ("CVP", ("start with function %s", FUNDEF_NAME (arg_node)));

    TRAVpush (TR_cvp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
