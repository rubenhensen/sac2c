

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

#include "AssociativeLaw.h"

/*****************************************************************************
 *
 * function:
 *   node *AssociativeLaw(node *arg_node)
 *
 * description:
 *   starting point of AssociativeLaw
 *
 *****************************************************************************/

node *
AssociativeLaw (node *arg_node, node *a)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("AssociativeLaw");

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = al_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_block-nodes
 *
 ****************************************************************************/

node *
ALblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALblock");

    if (BLOCK_INSTR (arg_node) != NULL) {

        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_assign-nodes
 *
 ****************************************************************************/

node *
ALassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALassign");

    if (ContainOptInformation (arg_info) == 1) {
        /* include new N_assign nodes from arg_info behind actual N_assign node
         * modify old successor
         * delete arg_info nodes
         */
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALinstr(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
ALinstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALinstr");
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALprf(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_prf-nodes
 *
 ****************************************************************************/

node *
ALprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALprf");
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   AssociativeLawOptimize(node *arg_node, node *arg_info)
 *
 * description:
 *   main-routine for optimization of one N_Assign-node
 *   start all subroutines
 *   arg_node is a N_prf-node
 *
 ****************************************************************************/

node *
AssociativeLawOptimize (node *arg_node, node *arg_info)
{
    int anz_const;
    int anz_all;
    DBUG_ENTER ("AssociativeLawOptimize");
    if (IsAssociativeAndCommutative (arg_node) == 1) {
        arg_info = TravElems (PRF_ARGS (arg_node), arg_info);

        arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

        anz_const = CountConst (arg_info);
        anz_all = CountAll (arg_info);

        if ((anz_const > 1) && (anz_all > 2)) {
            SortList (arg_info);
            CreateNAssignNodes (arg_info);
            CommitNAssignNodes (arg_info);
        }
    }
    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   IsAssociativeAndCommutative(node *arg_node)
 *
 * description:
 *   returns true if the primitive operation is associative and commutative
 *
 ****************************************************************************/

int
IsAssociativeAndCommutative (node *arg_node)
{
    DBUG_ENTER ("IsAssociativeAndCommutative");
    switch (PRF_PRF (arg_node)) {
    case F_add:
    case F_mul:
    case F_max:
    case F_min:
    case F_and:
    case F_or:
        DBUG_RETURN (1);

    default:
        DBUG_RETURN (0);
    }
}

/*****************************************************************************
 *
 * function:
 *   TravElems(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverse recursively throw the definitions of arg_node.
 *   The function terminates, when
 *     (a) another primitive operation is used
 *     (b) a non-variable expression reached
 *     (c) a variable is reached, which is defined outside the actual N_block
 *   All terminate-nodes are collected in arg_info.
 *   arg_node is a N_exprs-node.
 *
 ****************************************************************************/

node *
TravElems (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TravElems");

    if (IsConstant (EXPRS_EXPR (arg_node)))
        arg_info = AddNode (arg_node, arg_info, 1);
    else {
        if (OtherPrfOp (arg_node, arg_info)) {
            arg_info = AddNode (arg_node, arg_info, 0);
        } else {
            arg_info = TravElems (PRF_ARGS (LET_EXPR (ID_DEF (EXPRS_EXPR (arg_node)))),
                                  arg_info);
            arg_info = TravElems (EXPRS_NEXT (
                                    PRF_ARGS (LET_EXPR (ID_DEF (EXPRS_EXPR (arg_node))))),
                                  arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   bool IsConstant(node *arg_node)
 *
 * description:
 *   returns true if the arg_node is a constant value
 *
 ****************************************************************************/

int
IsConstant (node *arg_node)
{
    DBUG_ENTER ("IsConstant");

    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_double:
    case N_float:
    case N_bool:
    case N_char:
        DBUG_RETURN (1);

    default:
        DBUG_RETURN (0);
    }
}

/*****************************************************************************
 *
 * function:
 *   node *AddNode(node *arg_node, node *arg_info)
 *
 * description:
 *   add the arg_node to the node-list in arg_info
 *   if the bool-argument is true, the arg_node contains a constant value
 *   if the bool-argument is false, the arg_node contains no constant value
 *
 ****************************************************************************/

node *
AddNode (node *arg_node, node *arg_info, bool constant)
{
    nodelist *nl;
    nodelist *newnodelistnode = MakeNodelistNode (EXPRS_EXPR (arg_node), NULL);

    DBUG_ENTER ("AddNode");

    if (constant == 1) {
        nl = ((nodelist *)(arg_info->info2));
        (*arg_info).counter = (*arg_info).counter + 1;
    } else {
        nl = ((nodelist *)(arg_info->info3));
        (*arg_info).varno = (*arg_info).varno + 1;
    }

    while (NODELIST_NEXT (nl) != NULL) {
        nl = NODELIST_NEXT (nl);
    }

    NODELIST_NEXT (nl) = newnodelistnode;

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   bool OtherPrfOp(node *arg_node, node *arg_info)
 *
 * description:
 *   returns true if NODE_TYPE(EXPRS_EXPR(arg_node))==N_id and the used
 *   primitive operation is not the same as saved in arg_info.
 *
 ****************************************************************************/

int
OtherPrfOp (node *arg_node, node *arg_info)
{
    int otherOp;
    prf otherPrf;
    DBUG_ENTER ("OtherPrfOp");
    otherPrf = PRF_PRF (LET_EXPR (ID_DEF (EXPRS_EXPR (arg_node))));

    if ((*arg_info).info.prf == otherPrf)
        otherOp = 0;
    else
        otherOp = 1;

    DBUG_RETURN (otherOp);
}

/*****************************************************************************
 *
 * function:
 *   int CountConst(node *arg_info)
 *
 * description:
 *   returns the number of nodes with constant values
 *
 ****************************************************************************/

int
CountConst (node *arg_info)
{
    int count = 0;

    DBUG_ENTER ("CountConst");

    count = (*arg_info).counter;

    DBUG_RETURN (count);
}

/*****************************************************************************
 *
 * function:
 *   int CountAll(node *arg_info)
 *
 * description:
 *   returns the number of all nodes
 *
 ****************************************************************************/

int
CountAll (node *arg_info)
{
    int count = 0;
    DBUG_ENTER ("CountALL");

    count = (*arg_info).counter;
    count = count + (*arg_info).varno;

    DBUG_RETURN (count);
}

/*****************************************************************************
 *
 * function:
 *   SortList(node *arg_info)
 *
 * description:
 *   sort the N_Assign-nodes-list
 *   sort algorithm: in front: constant nodes
 *                     at end: variable nodes
 *
 ****************************************************************************/

node *
SortList (node *arg_info)
{
    DBUG_ENTER ("SortList");

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   CreateNAssignNodes(node *arg_info)
 *
 * description:
 *   create for all nodes in arg_info a N_Assign node
 *   create for all new N_Assign nodes an N_Assign node until it exit one
 *   'top' N_Assign node
 *
 ****************************************************************************/

node *
CreateNAssignNodes (node *arg_info)
{
    DBUG_ENTER ("CreateNAssignNodes");

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   CommitNAssignNodes(node *arg_info)
 *
 * description:
 *   commit all N_Assign nodes in the correct order
 *
 ****************************************************************************/

node *
CommitNAssignNodes (node *arg_info)
{
    DBUG_ENTER ("CommitNAssignNodes");

    DBUG_RETURN (arg_info);
}

int
ContainOptInformation (node *arg_info)
{
    DBUG_ENTER ("ContainOptInformation");

    DBUG_RETURN (1);
}
