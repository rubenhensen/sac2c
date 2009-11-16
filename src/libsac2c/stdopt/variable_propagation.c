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

#include "variable_propagation.h"

/*
 * optimization phase: Variable Propagation (VP)
 *
 * VP is used to eliminate the usage of assignment arguments (RHS N_id nodes),
 * whose definition has only a single value (N_id) as a right hand
 * side. So we can avoid the usage of unnecessary copy assignments.
 *
 * Example:
 *    a = 7;                a = 7;
 *    b = a;          =>    b = a;
 *    c = fun(b);           c = fun(a);
 *
 * Obsolete definitions of assignments are removed by DeadCodeRemoval.
 *
 * Implementation:
 *   This optimization phase is rather simple.
 *   For every function (fundef-node) we start a top-down traversal of the AST.
 *   We traverse in every assignment until we reach an id node. For every id
 *   node we either try do replace it with another id.
 *
 *   This code is lifted from ConstVarPropagation.c. The intent here
 *   is that we never propagate scalar constants into RHS nodes
 *   until after optimization is complete. The simplest way to do
 *   this is to break CVP into two independent phases, VP and CP.
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

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
};

/*
 * INFO macros
 */
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

/** <!--********************************************************************-->
 *
 * @fn node *VPavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
VPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("VPavis");

    AVIS_DIM (arg_node) = TRAVopt (AVIS_DIM (arg_node), arg_info);

    AVIS_SHAPE (arg_node) = TRAVopt (AVIS_SHAPE (arg_node), arg_info);

    if ((AVIS_MINVAL (arg_node) != NULL) && (arg_node != AVIS_MINVAL (arg_node))) {
        AVIS_MINVAL (arg_node) = TRAVdo (AVIS_MINVAL (arg_node), arg_info);
    }

    if ((AVIS_MAXVAL (arg_node) != NULL) && (arg_node != AVIS_MAXVAL (arg_node))) {
        AVIS_MAXVAL (arg_node) = TRAVdo (AVIS_MAXVAL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VPid( node *arg_node, info *arg_info)
 *
 * description:
 *   Try to replace this N_id node by another identifier
 *   This function does all the real work for this phase.
 *
 *****************************************************************************/
node *
VPid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("VPid");

    avis = ID_AVIS (arg_node);
    if ((AVIS_SSAASSIGN (avis) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis))) == N_id)) {
        arg_node = FREEdoFreeNode (arg_node);
        DBUG_PRINT ("VP", ("CVPid replacing %s", AVIS_NAME (avis)));
        if (N_id == NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis)))) {
            DBUG_PRINT ("VP", ("by %s",
                               AVIS_NAME (ID_AVIS (ASSIGN_RHS (AVIS_SSAASSIGN (avis))))));
        }
        arg_node = DUPdoDupNode (ASSIGN_RHS (AVIS_SSAASSIGN (avis)));
        global.optcounters.vp_expr += 1;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VPgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *   the default element, if present, must be a variable
 *
 *****************************************************************************/
node *
VPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("VPgenarray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VPmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   only variables are allowed in MODARRAY_ARRAY
 *
 *****************************************************************************/
node *
VPmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("VPmodarray");

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VPfold(node *arg_node, info *arg_info)
 *
 * description:
 *  only variables are allowed as neutral elements
 *
 *****************************************************************************/
node *
VPfold (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("VPfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VPrange(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse body and expression for each range node
 *
 *
 *****************************************************************************/
node *
VPrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("VPrange");

    /* traverse body of range */
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    /* traverse expression to do variable substitution */
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);

    /* traverse to next node */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* VPlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the expr of the let node
 *
 *****************************************************************************/

node *
VPlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("VPlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* VPassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse in the instr of the assign node
 *
 *****************************************************************************/

node *
VPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("VPassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* VPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse the body of the fundef node
 *
 *****************************************************************************/

node *
VPfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("VPfundef");

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
 *   node* VPdoVarPropagation(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
VPdoVarPropagation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("VPdoVarPropagation");

    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_vp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/********************************************************************
 *
 * function:
 *   node* VPdoVarPropagationOneFundef(node *arg_node)
 *
 * description:
 *   This function is called to start this optimization.
 *   Starting point of the traversal through the AST.
 *
 ********************************************************************/

node *
VPdoVarPropagationOneFundef (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("VPdoVarPropagation");

    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_vp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
