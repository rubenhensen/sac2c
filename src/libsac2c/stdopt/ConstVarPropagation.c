/*
 * $Id$
 */
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "constants.h"
#include "new_types.h"
#include "globals.h"

#include "ConstVarPropagation.h"

/*
 * optimization phase: Constant and Variable Propagation (CVP)
 *
 * CVP is used to eliminate the usage of assignment arguments (RHS N_id nodes),
 * whose definition has only a single value (N_id, N_num, ...) as a right hand
 * side. So we can avoid the usage of unnecessary copy assignments.
 *
 * Example:
 *    a = 7;                a = 7;
 *    b = a;          =>    b = a;
 *    c = fun(b);           c = fun(a);
 *
 * Obsolete definitions of assignments are removed by the DeadCodeRemoval.
 *
 * Implementation:
 *   This optimization phase is rather simple.
 *   For every function (fundef-node) we start a top-down traversal of the AST.
 *   We traverse in every assignment until we reach an id node. For every id
 *   node we either try do replace it with a constant scalar / constant array
 *   if it is allowed in the current context, or with another id.
 */

/*
 * NOTE: Similar optimizations are implemented in SSACSE.[ch] and
 *       SSAConstantFolding.[ch].
 *       CVP should separate the constant and variable propagation out of these
 *       optimizations, because the scope between them is different.
 *       But because the implementation of SSACSE and SSAConstantFolding
 *       remains untouched, they still provide these functionality.
 *       So its enough to change the implemenation of this file if you want to
 *       modify the constant and variable propagation. Only if the
 *       implementation of SSACSE or SSAConstantFolding interfere with your
 *       modifications, you have to take a look at these files...
 */

const int PROP_nothing = 0;
const int PROP_variable = 1;
const int PROP_scalarconst = 2;
const int PROP_arrayconst = 4;
const int PROP_array = 8;

/*
 * INFO structure
 */
struct INFO {
    int propmode;
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_PROPMODE(n) (n->propmode)
#define INFO_ONEFUNDEF(n) (n->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PROPMODE (result) = PROP_nothing;
    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
IsScalarConst (node *arg_node)
{
    bool res;

    DBUG_ENTER ("IsScalarConst");

    switch (NODE_TYPE (arg_node)) {
    case N_float:
    case N_double:
    case N_bool:
    case N_num:
    case N_char:
        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}
/******************************************************************************
 *
 * function:
 *   node* CVParray( node *arg_node, info *arg_info)
 *
 * description:
 *   propagate scalars and variables into ARRAY_AELEMS
 *
 *****************************************************************************/
node *
CVParray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVParray");

    INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CVPavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CVPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPavis");

    if (AVIS_DIM (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
        AVIS_DIM (arg_node) = TRAVdo (AVIS_DIM (arg_node), arg_info);
    }

    if (AVIS_SHAPE (arg_node) != NULL) {
        INFO_PROPMODE (arg_info)
          = PROP_variable | PROP_scalarconst | PROP_arrayconst | PROP_array;
        AVIS_SHAPE (arg_node) = TRAVdo (AVIS_SHAPE (arg_node), arg_info);
    }

    INFO_PROPMODE (arg_info) = PROP_nothing;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CVPreturn(node *arg_node, info *arg_info)
 *
 * description:
 *   only propagate variables into RETURN_EXPRS
 *
 *****************************************************************************/
node *
CVPreturn (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPreturn");

    INFO_PROPMODE (arg_info) = PROP_variable;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CVPfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *   only propagate variables into sons of N_funcond
 *
 *****************************************************************************/
node *
CVPfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPfuncond");

    INFO_PROPMODE (arg_info) = PROP_variable;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPid( node *arg_node, info *arg_info)
 *
 * description:
 *   Depending on the propagation mode, try to replace this N_id node by
 *    - a constant scalar ( PROP_scalarconst is set)
 *    - an array constant ( PROP_arrayconst is set)
 *    - another identifier ( PROP_variable is set)
 *
 *****************************************************************************/
node *
CVPid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CVPid");

    avis = ID_AVIS (arg_node);
    if (TYisAKV (AVIS_TYPE (avis))
        && (((INFO_PROPMODE (arg_info) & PROP_arrayconst)
             && (TYgetDim (AVIS_TYPE (avis)) != 0))
            || ((INFO_PROPMODE (arg_info) & PROP_scalarconst)
                && (TYgetDim (AVIS_TYPE (avis)) == 0)))) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = COconstant2AST (TYgetValue (AVIS_TYPE (avis)));
        global.optcounters.cvp_expr += 1;
    } else {
        if ((AVIS_SSAASSIGN (avis) != NULL)
            && (((INFO_PROPMODE (arg_info) & PROP_variable)
                 && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis))) == N_id))
                || ((INFO_PROPMODE (arg_info) & PROP_array)
                    && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis))) == N_array))
                || ((INFO_PROPMODE (arg_info) & PROP_scalarconst)
                    && (IsScalarConst (ASSIGN_RHS (AVIS_SSAASSIGN (avis))))))) {
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (ASSIGN_RHS (AVIS_SSAASSIGN (avis)));
            global.optcounters.cvp_expr += 1;
        }
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
    case F_type_conv:
    case F_type_error:
    case F_dispatch_error:
    case F_sel:
    case F_copy:
        /*
         * Only propagate variables here
         */
        INFO_PROPMODE (arg_info) = PROP_variable;
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    case F_saabind:
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_variable;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_variable;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        INFO_PROPMODE (arg_info) = PROP_variable;
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        break;

    case F_idx_shape_sel:
    case F_take_SxV:
    case F_drop_SxV:
        /*
         * Only the first argument may be a scalar constant
         */
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_variable;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_variable;
        EXPRS_EXPRS2 (PRF_ARGS (arg_node))
          = TRAVdo (EXPRS_EXPRS2 (PRF_ARGS (arg_node)), arg_info);
        break;

    case F_modarray:
        /*
         * The first two arguments of modarray must be variable
         * the other one can as well be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_variable;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);

        break;

    /**
     * Although the following operations do not exist during the
     * optimisations, CVP needs to be able to handle them correctly
     * as CVP is run after IVE, as well!
     */
    case F_idx_sel:
        DBUG_ASSERT (global.compiler_subphase >= PH_opt_ivesplit,
                     "F_idx_ operations are not allowed during the optimizer!");
        /*
         * The second argument of idx_sel must be variable
         * the others can as well be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_variable;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        break;

    case F_idx_modarray:
        DBUG_ASSERT (global.compiler_subphase >= PH_opt_ivesplit,
                     "F_idx_ operations are not allowed during the optimizer!");
        /*
         * The first argument of idx_modarray must be variable
         * the others can as well be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_variable;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        break;

    default:
        /*
         * In the default case, all arguments may be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_variable | PROP_scalarconst;
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
 *   only propagate variables into the application arguments
 *
 ********************************************************************/

node *
CVPap (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPap");

    INFO_PROPMODE (arg_info) = PROP_variable;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPcond(node *arg_node, info *arg_info)
 *
 * description:
 *   only propagate variables into COND_COND
 *
 *****************************************************************************/

node *
CVPcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPcond");

    INFO_PROPMODE (arg_info) = PROP_variable;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CVPgenerator( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CVPgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPgenerator");

    INFO_PROPMODE (arg_info) = PROP_variable;
    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);

    INFO_PROPMODE (arg_info) = PROP_variable;
    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_variable;
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_variable;
        GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
    }

    if (GENERATOR_GENWIDTH (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_variable;
        GENERATOR_GENWIDTH (arg_node) = TRAVdo (GENERATOR_GENWIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *   GENARRAY_SHAPE may be an array constant
 *   the default element must be a variable if present
 *
 *****************************************************************************/
node *
CVPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPgenarray");

    INFO_PROPMODE (arg_info) = PROP_variable;
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_variable;
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   only variables are allowed in MODARRAY_ARRAY
 *
 *****************************************************************************/
node *
CVPmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPmodarray");

    INFO_PROPMODE (arg_info) = PROP_variable;
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CVPfold(node *arg_node, info *arg_info)
 *
 * description:
 *  only variables are allowed as neutral elements
 *
 *****************************************************************************/
node *
CVPfold (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPfold");

    INFO_PROPMODE (arg_info) = PROP_variable;
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

    /* traverse expression to do variable substitution */
    INFO_PROPMODE (arg_info) = PROP_variable;
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /* traverse to next node */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CVPlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the expr of the let node
 *
 *****************************************************************************/

node *
CVPlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CVPlet");

    INFO_PROPMODE (arg_info) = PROP_variable;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CVPassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the instr of the assign node
 *
 *****************************************************************************/

node *
CVPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPassign");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CVPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the body of the fundef node
 *
 *****************************************************************************/

node *
CVPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CVPfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
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

    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_cvp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* ConstVarPropagationOneFundef(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
CVPdoConstVarPropagationOneFundef (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("ConstVarPropagation");

    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_cvp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}