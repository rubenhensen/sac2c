

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

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (ContainOptInformation (arg_info) == 1) {

            nodelist *nodelist1;
            node *old_succ, *akt_nassign;

            /* include new N_assign nodes from arg_info behind actual N_assign node
             * modify old successor
             * delete arg_info nodes
             */

            /* store next N_assign node */
            old_succ = ASSIGN_NEXT (arg_node);

            /* pointer on the last included N_assign node */
            akt_nassign = arg_node;

            /* include N_assign-nodes created from constant nodes */
            nodelist1 = (nodelist *)(arg_info->info2);

            while (NODELIST_NEXT (nodelist1) != NULL) {

                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
                nodelist1 = NODELIST_NEXT (nodelist1);
            }
            ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
            akt_nassign = ASSIGN_NEXT (akt_nassign);

            /* include N_assign-nodes created from 'variable' nodes */
            nodelist1 = (nodelist *)(arg_info->info3);

            while (NODELIST_NEXT (nodelist1) != NULL) {

                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
                nodelist1 = NODELIST_NEXT (nodelist1);
            }
            ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
            akt_nassign = ASSIGN_NEXT (akt_nassign);

            /* connect last included element with stored N_assign node */
            ASSIGN_NEXT (akt_nassign) = old_succ;
        }
    }

    /* traverse in N_let-node */

    if (ASSIGN_INSTR (arg_node) != NULL) {

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
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

/*node *ALinstr(node *arg_node, node* arg_info)
 *{
 * DBUG_ENTER("ALinstr");
 * if (ASSIGN_INSTR(arg_node) != NULL)
 *   {
 *     ASSIGN_INSTR(arg_node)=Trav(ASSIGN_INSTR(arg_node),arg_info);
 *   }
 *
 * DBUG_RETURN(arg_info);
 *}
 */

/*****************************************************************************
 *
 * function:
 *   ALlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
ALlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALprf");
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
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

        (*arg_info).varno = 0;
        (*arg_info).counter = 0;
        (*arg_info).info.prf = PRF_PRF (arg_node);

        arg_info = TravElems (PRF_ARGS (arg_node), arg_info);

        arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

        anz_const = CountConst (arg_info);
        anz_all = CountVar (arg_info) + anz_const;

        if ((anz_const > 1) && (anz_all > 2)) {

            /* start optimize */
            SortList (arg_info);
            CreateNAssignNodes (arg_info);
            CommitNAssignNodes (arg_info);
        } else {

            /* nothing to optimize */
            FreeNodelist ((nodelist *)(arg_info->info2));
            FreeNodelist ((nodelist *)(arg_info->info3));
            (*arg_info).varno = 0;
            (*arg_info).counter = 0;
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

    if (IsConstant (EXPRS_EXPR (arg_node))) {

        arg_info = AddNode (arg_node, arg_info, 1);

    } else {

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
    }

    else {
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
CountVar (node *arg_info)
{
    int count = 0;
    DBUG_ENTER ("CountALL");

    count = (*arg_info).varno;

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
    int zaehl;
    node *listElem, *listElem2, *newvardec, *newnode;
    char *newname, mod;
    statustype status;
    ids *newids;
    nodelist *aktElem, *secElem;

    DBUG_ENTER ("CreateNAssignNodes");

    aktElem = (nodelist *)(arg_info->info2); /*Pointer on first list element*/

    for (zaehl = 0; zaehl < CountConst (arg_info); zaehl++) {
        listElem = NODELIST_NODE (aktElem); /*pointer on actual node stored in nodelist*/
        listElem = DupNode (listElem);      /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (aktElem) = listElem;
    }
    /* all elements of the nodelist are N_expr nodes*/

    aktElem = (nodelist *)(arg_info->info2); /*pointer on first list element*/
    for (zaehl = 0; zaehl < (div (CountConst (arg_info), 2).quot); zaehl++) {
        secElem = NODELIST_NEXT (aktElem); /*pointer on next list element*/
        listElem = NODELIST_NODE (aktElem);
        listElem2 = NODELIST_NODE (secElem);

        EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
        listElem = MakePrf ((*arg_info).info.prf, listElem);

        /* to do: create new vardec node
         *        create new ids
         *        create new name    */

        listElem = MakeLet (listElem, newids);
        listElem = MakeAssignLet (newname, newvardec, listElem);

        NODELIST_NEXT (aktElem) = NODELIST_NEXT (secElem); /* remove second node*/
        secElem = aktElem; /* store pointer on last element */
        aktElem = NODELIST_NEXT (aktElem);
    }

    if (div (CountConst (arg_info), 2).rem == 1) { /*concatenate last nodelist-element*/

        listElem = NODELIST_NODE (secElem);  /* pointer on last N_assign-node */
        listElem2 = NODELIST_NODE (aktElem); /* pointer on last node in list */
                                             /* to do: create new name, mod, statustype */

        newnode = MakeId (newname, mod, status);

        /* to do: connect new N_id-node with correct N_AVIS */
        newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */
        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new vardec node
         *        create new ids
         *        create new name       */

        newnode = MakeLet (newnode, newids);
        newnode = MakeAssignLet (newname, newvardec, newnode);

        NODELIST_NODE (secElem) = newnode; /* store new node */
        NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
    }

    /* nodelist for constant nodes ready */

    aktElem = (nodelist *)(arg_info->info3); /*Pointer on first list element*/

    for (zaehl = 0; zaehl < (CountVar (arg_info)); zaehl++) {
        listElem = NODELIST_NODE (aktElem); /*pointer on actual node stored in nodelist*/
        listElem = DupNode (listElem);      /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (aktElem) = listElem;
    }
    /* all elements of the nodelist are N_expr nodes*/

    aktElem = (nodelist *)(arg_info->info3); /*pointer on first list element*/
    for (zaehl = 0; zaehl < (div (CountVar (arg_info), 2).quot); zaehl++) {
        secElem = NODELIST_NEXT (aktElem); /*pointer on next list element*/
        listElem = NODELIST_NODE (aktElem);
        listElem2 = NODELIST_NODE (secElem);

        EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
        listElem = MakePrf ((*arg_info).info.prf, listElem);

        /* to do: create new vardec node
         *        create new ids
         *        create new name    */

        listElem = MakeLet (listElem, newids);
        listElem = MakeAssignLet (newname, newvardec, listElem);

        NODELIST_NEXT (aktElem) = NODELIST_NEXT (secElem); /* remove second node*/
        secElem = aktElem; /* store pointer on last element */
        aktElem = NODELIST_NEXT (aktElem);
    }

    if (div (CountVar (arg_info), 2).rem == 1) { /*concatenate last nodelist-element*/

        listElem = NODELIST_NODE (secElem);  /* pointer on last N_assign-node */
        listElem2 = NODELIST_NODE (aktElem); /* pointer on last node in list */
                                             /* to do: create new name, mod, statustype */

        newnode = MakeId (newname, mod, status);

        /* to do: connect new N_id-node with correct N_AVIS */
        newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */
        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new vardec node
         *        create new ids
         *        create new name       */

        newnode = MakeLet (newnode, newids);
        newnode = MakeAssignLet (newname, newvardec, newnode);

        NODELIST_NODE (secElem) = newnode; /* store new node */
        NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
    }

    /* nodelist for 'variable' nodes ready */

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

    ids *newids;
    node *newnode, *newvardec, *elem1, *elem2;
    char *newname1, *newname2, *newmod1, *newmod2;
    nodelist *lastListElem, *aktListElem;
    statustype status1, status2;

    DBUG_ENTER ("CommitNAssignNodes");

    aktListElem = (nodelist *)(arg_info->info2);
    lastListElem = aktListElem;

    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    while (aktListElem != lastListElem) {
        elem1 = NODELIST_NODE (aktListElem);
        aktListElem = NODELIST_NEXT (aktListElem);
        elem2 = NODELIST_NODE (aktListElem);

        /* to do: create new name, mod , statustype */

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeId (newname2, newmod2, status2));

        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new name
         *        create new ids node
         *        create new vardec node  */

        newnode = MakeLet (newnode, newids);

        newnode = MakeAssignLet (newname1, newvardec, newnode);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        aktListElem = NODELIST_NEXT (aktListElem);
    }

    /* constant nodes ready  */

    aktListElem = (nodelist *)(arg_info->info3);
    lastListElem = aktListElem;

    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    while (aktListElem != lastListElem) {
        elem1 = NODELIST_NODE (aktListElem);
        aktListElem = NODELIST_NEXT (aktListElem);
        elem2 = NODELIST_NODE (aktListElem);

        /* to do: create new name, mod , statustype */

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeId (newname2, newmod2, status2));

        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new name
         *        create new ids node
         *        create new vardec node  */

        newnode = MakeLet (newnode, newids);

        newnode = MakeAssignLet (newname1, newvardec, newnode);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        aktListElem = NODELIST_NEXT (aktListElem);
    }

    /* variable nodes ready  */

    /* connect last N_assign nodes of both lists */
    lastListElem = (nodelist *)(arg_info->info2);

    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    elem1 = NODELIST_NODE (aktListElem);
    elem2 = NODELIST_NODE (lastListElem);

    /* to do create new name, mod, statustype */

    newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                         MakeId (newname2, newmod2, status2));

    newnode = MakePrf ((*arg_info).info.prf, newnode);

    /* to do: create new name
     *        create new ids node
     *        create new vardec node   */

    newnode = MakeLet (newnode, newids);

    newnode = MakeAssignLet (newname1, newvardec, newnode);

    NODELIST_NEXT (aktListElem) = MakeNodelistNode (newnode, NULL);

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   ContainOptInformation(node *arg_info)
 *
 * description:
 *   returns 1 (true) if the arg_info-node contains N_assign-nodes to include
 *   in the tree
 *
 ****************************************************************************/

int
ContainOptInformation (node *arg_info)
{
    DBUG_ENTER ("ContainOptInformation");

    if ((CountConst (arg_info) > 1) && (CountVar (arg_info) > 1))
        DBUG_RETURN (1);
    else
        DBUG_RETURN (0);
}
