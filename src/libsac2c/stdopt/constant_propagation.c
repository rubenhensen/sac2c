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

#include "constant_propagation.h"

/*
 * optimization phase: Constant Propagation (CP)
 *
 * CP is used to eliminate the usage of assignment arguments (RHS N_id nodes),
 * whose definition has only a single value (N_id, N_num, ...) as a right hand
 * side. So we can avoid the usage of unnecessary copy assignments.
 * This phase is only used post-optimization. During optimization,
 * we never want to drive simple scalars (N_num, N_char...) into
 * RHS nodes.
 *
 * Example:
 *    a = 7;                a = 7;
 *    b = a;          =>    b = a;
 *    c = fun(b);           c = fun(7);
 *
 * Obsolete definitions of assignments are removed by DeadCodeRemoval.
 *
 * Implementation:
 *   This optimization phase is rather simple.
 *   For every function (fundef-node) we start a top-down traversal of the AST.
 *   We traverse in every assignment until we reach an id node. For every id
 *   node we try to replace it with a constant scalar / constant array
 *   if it is allowed in the current context.
 *
 *   Some of the PROP_nothing traversals can likely be deleted now.
 */

/*
 * NOTE: Similar optimizations are implemented in SSACSE.[ch],
 *       SSAConstantFolding.[ch], and variable_propagation.[ch].
 *       CP performs constant propagation only
 *       because the scope between them is different.
 *       But because the implementation of SSACSE and SSAConstantFolding
 *       remains untouched, they still provide these functionality.
 *       So its enough to change the implemenation of this file if you want to
 *       modify the constant and variable propagation. If the
 *       implementation of SSACSE or SSAConstantFolding interfere with your
 *       modifications, you should take a look at these files...
 *
 */

const int PROP_nothing = 0;
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
 *   node* CParray( node *arg_node, info *arg_info)
 *
 * description:
 *   propagate scalars and variables into ARRAY_AELEMS
 *
 *****************************************************************************/
node *
CParray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CParray");

    INFO_PROPMODE (arg_info) = PROP_scalarconst;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CPavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPavis");

    INFO_PROPMODE (arg_info) = PROP_nothing;

    if (AVIS_DIM (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        AVIS_DIM (arg_node) = TRAVdo (AVIS_DIM (arg_node), arg_info);
    }

    if (AVIS_SHAPE (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_arrayconst | PROP_array;
        AVIS_SHAPE (arg_node) = TRAVdo (AVIS_SHAPE (arg_node), arg_info);
    }

    if ((AVIS_MINVAL (arg_node) != NULL) && (arg_node != AVIS_MINVAL (arg_node))) {
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_arrayconst | PROP_array;
        AVIS_MINVAL (arg_node) = TRAVdo (AVIS_MINVAL (arg_node), arg_info);
    }

    if ((AVIS_MAXVAL (arg_node) != NULL) && (arg_node != AVIS_MAXVAL (arg_node))) {
        INFO_PROPMODE (arg_info) = PROP_scalarconst | PROP_arrayconst | PROP_array;
        AVIS_MAXVAL (arg_node) = TRAVdo (AVIS_MAXVAL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CPreturn(node *arg_node, info *arg_info)
 *
 * description:
 *   (only propagate variables into RETURN_EXPRS)
 *
 *****************************************************************************/
node *
CPreturn (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPreturn");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CPfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *   (only propagate variables into sons of N_funcond)
 *
 *****************************************************************************/
node *
CPfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPfuncond");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPid( node *arg_node, info *arg_info)
 *
 * description:
 *   Depending on the propagation mode, try to replace this N_id node by
 *    - a constant scalar ( PROP_scalarconst is set)
 *    - an array constant ( PROP_arrayconst is set)
 *
 *****************************************************************************/
node *
CPid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CPid");

    avis = ID_AVIS (arg_node);
    if (TYisAKV (AVIS_TYPE (avis))
        && (((INFO_PROPMODE (arg_info) & PROP_arrayconst)
             && (TYgetDim (AVIS_TYPE (avis)) != 0))
            || ((INFO_PROPMODE (arg_info) & PROP_scalarconst)
                && (TYgetDim (AVIS_TYPE (avis)) == 0)))) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = COconstant2AST (TYgetValue (AVIS_TYPE (avis)));
        global.optcounters.cp_expr += 1;
    } else {
        if ((AVIS_SSAASSIGN (avis) != NULL)
            && (((INFO_PROPMODE (arg_info) & PROP_array)
                 && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis))) == N_array))
                || ((INFO_PROPMODE (arg_info) & PROP_scalarconst)
                    && (IsScalarConst (ASSIGN_RHS (AVIS_SSAASSIGN (avis))))))) {
            arg_node = FREEdoFreeNode (arg_node);
            DBUG_PRINT ("CP", ("CPid replacing %s", AVIS_NAME (avis)));
            if (N_id == NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis)))) {
                DBUG_PRINT ("CP",
                            ("by %s",
                             AVIS_NAME (ID_AVIS (ASSIGN_RHS (AVIS_SSAASSIGN (avis))))));
            }
            arg_node = DUPdoDupNode (ASSIGN_RHS (AVIS_SSAASSIGN (avis)));
            global.optcounters.cp_expr += 1;
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node* CPprf(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the arguments of the prf node
 *   Some prf arguments must not become constants.
 *   E.g. The prf implementation of F_dim_A cannot handle constants at all!
 *
 *****************************************************************************/

node *
CPprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPprf");

    /*
     * Depending on the primitive function, different arguments
     * are allowed to become constant
     */
    switch (PRF_PRF (arg_node)) {
    case F_dim_A:
    case F_shape_A:
    case F_accu:
    case F_type_conv:
    case F_type_error:
    case F_dispatch_error:
    case F_sel_VxA:
    case F_copy:
    case F_guard:
    case F_afterguard:
    case F_type_constraint:
    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
    case F_modarray_AxVxA:
    case F_idx_modarray_AxSxS:
    case F_idx_modarray_AxSxA:
    case F_attachextrema:
    case F_attachintersect:
        /*
         * Only propagate variables here
         *  This can probably be deleted...
         */
        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;

    case F_saabind:
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        break;

    case F_idx_shape_sel:
    case F_take_SxV:
    case F_drop_SxV:
        /*
         * Only the first argument may be a scalar constant
         */
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_nothing;
        EXPRS_EXPRS2 (PRF_ARGS (arg_node))
          = TRAVdo (EXPRS_EXPRS2 (PRF_ARGS (arg_node)), arg_info);
        break;

    case F_modarray_AxVxS:
        /*
         * The first two arguments of modarray must be variable
         * the other one can as well be a constant scalar
         */
        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);

        break;

    /**
     * Although the following operations do not exist during the
     * optimisations, CP needs to be able to handle them correctly
     * as CP is run after IVE, as well!
     */
    case F_idxs2offset:
    case F_vect2offset:
        DBUG_ASSERT (global.compiler_subphase >= PH_opt_ivesplit,
                     "F_idx2offset/vect2offset operations are not allowed during the "
                     "optimizer!");

        /*
         * The first argument (the shape) may be an array constant. All others
         * must be identifiers.
         */

        INFO_PROPMODE (arg_info) = PROP_arrayconst | PROP_array;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_EXPRS2 (arg_node) = TRAVopt (PRF_EXPRS2 (arg_node), arg_info);
        break;

    case F_idx_sel:
        DBUG_ASSERT (global.compiler_subphase >= PH_opt_ivesplit,
                     "F_idx_ operations are not allowed during the optimizer!");
        /*
         * The second argument of idx_sel must be variable
         * the others can as well be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        break;

#ifdef BUG437
    case F_idx_modarray_AxSxS:
    case F_idx_modarray_AxSxA:
        DBUG_ASSERT (global.compiler_subphase >= PH_opt_ivesplit,
                     "F_idx_ operations are not allowed during the optimizer!");
        /*
         * The first argument of idx_modarray must be variable
         * the others can as well be constant scalars
         */
        INFO_PROPMODE (arg_info) = PROP_nothing;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);

        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        break;
#endif // BUG437

    default:
        /*
         * In the default case, NO prf arguments may be constant scalars
         * during saacyc optimization and thereabouts.
         */
        INFO_PROPMODE (arg_info) = PROP_scalarconst;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CPap(node *arg_node, info *arg_info)
 *
 * description:
 *   only propagate variables into the application arguments
 *
 ********************************************************************/

node *
CPap (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPap");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPcond(node *arg_node, info *arg_info)
 *
 * description:
 *   only propagate variables into COND_COND
 *
 *****************************************************************************/

node *
CPcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPcond");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CPgenerator( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CPgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPgenerator");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);

    INFO_PROPMODE (arg_info) = PROP_nothing;
    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_nothing;
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_nothing;
        GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
    }

    if (GENERATOR_GENWIDTH (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_nothing;
        GENERATOR_GENWIDTH (arg_node) = TRAVdo (GENERATOR_GENWIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *   GENARRAY_SHAPE may be an array constant
 *   the default element must be a variable if present
 *
 *****************************************************************************/
node *
CPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPgenarray");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        INFO_PROPMODE (arg_info) = PROP_nothing;
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
 *   node *CPmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   only variables are allowed in MODARRAY_ARRAY
 *
 *****************************************************************************/
node *
CPmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPmodarray");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPfold(node *arg_node, info *arg_info)
 *
 * description:
 *  only variables are allowed as neutral elements
 *
 *****************************************************************************/
node *
CPfold (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPfold");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPcode(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse codeblock and expression for each Ncode node
 *
 *
 *****************************************************************************/
node *
CPcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPcode");

    /* traverse codeblock */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /* traverse expression to do variable substitution */
    INFO_PROPMODE (arg_info) = PROP_nothing;
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /* traverse to next node */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CPrange(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse body and expression for each range node
 *
 *
 *****************************************************************************/
node *
CPrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPrange");

    /* traverse body of range */
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    /* traverse expression to do variable substitution */
    INFO_PROPMODE (arg_info) = PROP_nothing;
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);

    /* traverse to next node */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CPlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the expr of the let node
 *
 *****************************************************************************/

node *
CPlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CPlet");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CPassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the instr of the assign node
 *
 *****************************************************************************/

node *
CPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CPassign");

    INFO_PROPMODE (arg_info) = PROP_nothing;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the body of the fundef node
 *
 *****************************************************************************/

node *
CPfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("CPfundef");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CPdoConstantPropagation(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
CPdoConstantPropagation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("CPdoConstantPropagation");

    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_cp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* CPdoConstantPropagationOneFundef(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
CPdoConstantPropagationOneFundef (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("CPdoConstantPropagationOneFundef");

    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_cp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
