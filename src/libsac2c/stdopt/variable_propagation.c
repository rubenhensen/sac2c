#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "VP"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "constants.h"
#include "new_types.h"
#include "globals.h"
#include "pattern_match.h"

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

/** <!--********************************************************************-->
 *
 * @fn node *VPavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
VPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at N_avis %s", AVIS_NAME (arg_node));

    arg_node = TRAVsons (arg_node, arg_info);

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

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    DBUG_PRINT ("Looking at N_id %s", AVIS_NAME (avis));
    if ((AVIS_SSAASSIGN (avis) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (avis))) == N_id)) {
        arg_node = FREEdoFreeNode (arg_node);
        DBUG_PRINT ("Replacing %s by %s", AVIS_NAME (avis),
                    AVIS_NAME (ID_AVIS (ASSIGN_RHS (AVIS_SSAASSIGN (avis)))));
        arg_node = DUPdoDupNode (ASSIGN_RHS (AVIS_SSAASSIGN (avis)));
        global.optcounters.vp_expr += 1;
    }

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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
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
    DBUG_ENTER ();

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_PRINT ("leaving body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    TRAVpush (TR_vp);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
