/* *
 * $Log$
 * Revision 1.11  2002/10/23 15:34:25  mwe
 * remove bug - store pointer to block node in function AssociativeLaw() and not in
 *ALblock()
 *
 * Revision 1.10  2002/10/16 11:47:43  mwe
 * reduce optimization expenditure
 * prevent useless optimization of unused nodes
 *
 * Revision 1.9  2002/10/14 16:15:43  mwe
 * F_add_AxA changed to F_add_SxS
 *
 * Revision 1.8  2002/10/14 12:10:30  mwe
 * al_expr-counter activated
 *
 * Revision 1.7  2002/10/10 09:25:06  mwe
 * debugging all functions
 * new functions added
 * seems to be the first correct working version
 *
 *
 *
 *----------------------------
 *revision 1.6
 *date: 2002/09/11 23:16:17;  author: dkr;  state: Exp;  lines: +2 -2
 *prf_node_info.mac modified
 *----------------------------
 *revision 1.5
 *date: 2002/08/25 14:23:12;  author: mwe;  state: Exp;  lines: +503 -69
 *elemination of warnings
 *changings in many functions
 *----------------------------
 *revision 1.4
 *date: 2002/07/21 21:46:40;  author: mwe;  state: Exp;  lines: +32 -16
 *editing ALassign
 *----------------------------
 *revision 1.3
 *date: 2002/07/21 15:56:44;  author: mwe;  state: Exp;  lines: +338 -28
 *editing CreateNAssignNodes and CommitNAssignNodes
 *----------------------------
 *revision 1.2
 *date: 2002/07/20 14:23:53;  author: mwe;  state: Exp;  lines: +427 -2
 *adding first functions
 *----------------------------
 *revision 1.1
 *date: 2002/06/07 17:38:35;  author: mwe;  state: Exp;
 *Initial revision
 *=============================================================================
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

        /* store pointer on actual N_block-node for append of new N_vardec nodes */
        if (ASSIGN_INSTR (arg_node) != NULL) {
            INFO_AL_BLOCKNODE (arg_info) = ASSIGN_INSTR (arg_node);
            if (BLOCK_VARDEC (ASSIGN_INSTR (arg_node)) != NULL) {
                (INFO_AL_TYPE (arg_info))
                  = VARDEC_TYPE (BLOCK_VARDEC (ASSIGN_INSTR (arg_node)));
            }
        }
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

        INFO_AL_OPTCONSTANTLIST (arg_info) = NULL;
        INFO_AL_OPTVARIABLELIST (arg_info) = NULL;

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

        ASSIGN_STATUS (arg_node) = 1;

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (ContainOptInformation (arg_info) == 1) {

            nodelist *nodelist1;
            node *old_succ, *akt_nassign;

            /* include new N_assign nodes from arg_info behind actual N_assign node
             * modify old successor
             * delete arg_info nodes
             */

            al_expr++;

            /* store next N_assign node */
            old_succ = ASSIGN_NEXT (arg_node);

            /* pointer on the last included N_assign node */
            akt_nassign = arg_node;

            /* include N_assign-nodes created from constant nodes */
            nodelist1 = INFO_AL_OPTCONSTANTLIST (arg_info);

            if (nodelist1 != NULL) {
                while (NODELIST_NEXT (nodelist1) != NULL) {
                    ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                    akt_nassign = ASSIGN_NEXT (akt_nassign);
                    nodelist1 = NODELIST_NEXT (nodelist1);
                }
                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
            }

            /* include N_assign-nodes created from 'variable' nodes */
            nodelist1 = INFO_AL_OPTVARIABLELIST (arg_info);

            while (NODELIST_NEXT (nodelist1) != NULL) {
                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
                nodelist1 = NODELIST_NEXT (nodelist1);
            }

            ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
            akt_nassign = ASSIGN_NEXT (akt_nassign);

            /* connect last included element with stored N_assign node */
            ASSIGN_NEXT (akt_nassign) = old_succ;

            /* clear nodelists in arg_info  */

            (INFO_AL_OPTVARIABLELIST (arg_info))
              = RemoveNodelistNodes (INFO_AL_OPTVARIABLELIST (arg_info));
            (INFO_AL_OPTCONSTANTLIST (arg_info))
              = RemoveNodelistNodes (INFO_AL_OPTCONSTANTLIST (arg_info));

            FreeNodelist (INFO_AL_OPTVARIABLELIST (arg_info));
            FreeNodelist (INFO_AL_OPTCONSTANTLIST (arg_info));
            (INFO_AL_CONSTANTLIST (arg_info)) = NULL;
            (INFO_AL_VARIABLELIST (arg_info)) = NULL;
            INFO_AL_OPTCONSTANTLIST (arg_info) = NULL;
            (INFO_AL_OPTVARIABLELIST (arg_info)) = NULL;
            INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
            INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
        }
    }

    /* traverse in N_let-node */

    if ((ASSIGN_INSTR (arg_node) != NULL) && (ASSIGN_STATUS (arg_node) == 1)) {

        INFO_AL_CURRENTASSIGN (arg_info) = arg_node;

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

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
    DBUG_ENTER ("ALlet");
    if (LET_EXPR (arg_node) != NULL) {
        INFO_AL_LETNODE (arg_info) = arg_node;
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
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
 *   start all subroutines
 *
 ****************************************************************************/

node *
ALprf (node *arg_node, node *arg_info)
{
    int anz_const;
    int anz_all;

    DBUG_ENTER ("AssociativeLawOptimize");

    if (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs) {
        if (IsAssociativeAndCommutative (arg_node) == 1) {

            INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
            INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            INFO_AL_CONSTANTLIST (arg_info) = NULL;
            INFO_AL_VARIABLELIST (arg_info) = NULL;
            INFO_AL_CURRENTPRF (arg_info) = PRF_PRF (arg_node);

            arg_info = TravElems (PRF_ARGS (arg_node), arg_info);
            arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

            anz_const = CountConst (arg_info);
            anz_all = CountVar (arg_info) + anz_const;

            if ((anz_const > 1) && (anz_all > 2)) {

                /* start optimize */
                CreateNAssignNodes (arg_info);

                anz_const = CountConst (arg_info);

                if (anz_const > 0) {
                    CommitNAssignNodes (arg_info);
                } else {
                    INFO_AL_NUMBEROFVARIABLES (arg_info) = 2;
                    INFO_AL_NUMBEROFCONSTANTS (arg_info) = 2;
                }

            } else {
                /* nothing to optimize */
                FreeNodelist (INFO_AL_CONSTANTLIST (arg_info));
                FreeNodelist (INFO_AL_VARIABLELIST (arg_info));
                INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
                INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            }
        }
    }
    DBUG_RETURN (arg_node);
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
    case F_add_SxS:
    case F_mul_SxS:
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

        if (OtherPrfOp (arg_node, arg_info) || ReachedArgument (EXPRS_EXPR (arg_node))
            || ReachedDefinition (EXPRS_EXPR (arg_node))) {

            arg_info = AddNode (arg_node, arg_info, 0);

        } else {

            ASSIGN_STATUS (INFO_AL_CURRENTASSIGN (arg_info)) = 0;
            INFO_AL_CURRENTASSIGN (arg_info)
              = AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)));

            arg_info = TravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                    AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                  arg_info);
            arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                    AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                  arg_info);
        }
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   bool ReachedArgument(node *arg_node)
 *
 * description:
 *   returns true if the arg_node is an argument of a function
 *
 ****************************************************************************/

int
ReachedArgument (node *arg_node)
{
    DBUG_ENTER ("ReachedArgument");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            return (1);
        else
            return (0);
    } else
        return (0);
}

/*****************************************************************************
 *
 * function:
 *   bool ReachedDefinition(node *arg_node)
 *
 * description:
 *   returns true if after the arg_node is a definition of a variable
 *
 ****************************************************************************/

int
ReachedDefinition (node *arg_node)
{
    DBUG_ENTER ("ReachedDefinition");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            return (0);
        else if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                 != N_prf)
            return (1);
        else
            return (0);
    } else
        return (0);
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
    nodelist *newnodelistnode = MakeNodelistNode (EXPRS_EXPR (arg_node), NULL);

    DBUG_ENTER ("AddNode");

    if (constant == 1) {
        INFO_AL_NUMBEROFCONSTANTS (arg_info) = INFO_AL_NUMBEROFCONSTANTS (arg_info) + 1;
    }

    else {
        INFO_AL_NUMBEROFVARIABLES (arg_info) = INFO_AL_NUMBEROFVARIABLES (arg_info) + 1;
    }

    if (constant == 1) {
        NODELIST_NEXT (newnodelistnode) = (INFO_AL_CONSTANTLIST (arg_info));
        INFO_AL_CONSTANTLIST (arg_info) = newnodelistnode;
    } else {
        NODELIST_NEXT (newnodelistnode) = ((nodelist *)(INFO_AL_VARIABLELIST (arg_info)));
        INFO_AL_VARIABLELIST (arg_info) = newnodelistnode;
    }

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
    if (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))) == NULL)
            otherOp = 0;
        else {
            otherPrf = PRF_PRF (
              LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))));

            if (INFO_AL_CURRENTPRF (arg_info) == otherPrf)
                otherOp = 0;
            else
                otherOp = 1;
        }
    } else
        otherOp = 0;

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

    count = INFO_AL_NUMBEROFCONSTANTS (arg_info);

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

    count = INFO_AL_NUMBEROFVARIABLES (arg_info);

    DBUG_RETURN (count);
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
    int count;
    node *listElem, *newnode;
    nodelist *aktElem, *secElem;

    DBUG_ENTER ("CreateNAssignNodes");

    aktElem = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first list element*/
    secElem = NULL;

    aktElem = CreateExprsNodes (aktElem, CountConst (arg_info));

    aktElem = INFO_AL_CONSTANTLIST (arg_info); /*pointer on first list element*/
    for (count = 0; count < (div (CountConst (arg_info), 2).quot); count++) {
        aktElem = CreateAssignNodesFromExprsNodes (aktElem, arg_info);
        aktElem = NODELIST_NEXT (aktElem);
    }

    if (div (CountConst (arg_info), 2).rem == 1) { /*concatenate last nodelist-element*/

        aktElem = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first list element*/
        while (NODELIST_NEXT (aktElem) != NULL) {
            secElem = aktElem;
            aktElem = NODELIST_NEXT (aktElem);
        }

        newnode = CreateAssignNodeFromAssignAndExprsNode (aktElem, secElem, arg_info);

        (INFO_AL_OPTCONSTANTLIST (arg_info))
          = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
        NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))) = NULL;

        FreeNodelist (aktElem);

        NODELIST_NODE (secElem) = newnode; /* store new node */
        NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
    }

    /*
     * constant nodes ready
     */

    aktElem = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/
    aktElem = CreateExprsNodes (aktElem, CountVar (arg_info));

    aktElem = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/

    if ((CountVar (arg_info) == 1) && (CountConst (arg_info) <= 3)) {

        secElem
          = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first constant list element*/
        while (NODELIST_NEXT (secElem) != NULL)
            secElem = NODELIST_NEXT (secElem);

        newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, aktElem);

        listElem = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        /* free listElem */

        INFO_AL_OPTVARIABLELIST (arg_info)
          = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
        NODELIST_NEXT ((INFO_AL_OPTVARIABLELIST (arg_info))) = NULL;

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;
        /* prohibits functioncall of 'CommitNassignNodes()' */
        INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
        INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
    } else {

        if (CountVar (arg_info) == 1) {

            secElem = INFO_AL_CONSTANTLIST (
              arg_info); /*Pointer on first constant list element*/
            while (NODELIST_NEXT (secElem) != NULL)
                secElem = NODELIST_NEXT (secElem);

            newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, aktElem);
            newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

            if (INFO_AL_OPTCONSTANTLIST (arg_info) != NULL) {
                NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info)))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                NODELIST_NEXT (NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))))
                  = NULL;
            } else {
                (INFO_AL_OPTCONSTANTLIST (arg_info))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                ;
                NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))) = NULL;
            }
            NODELIST_NODE (secElem) = NULL; /* store new node */ /* vorher aktElem */
            /*NODELIST_NEXT(secElem) = NULL;*/ /* remove old N_expr-node */
            NODELIST_NODE (aktElem) = newnode;
        } else {

            for (count = 0; count < (div (CountVar (arg_info), 2).quot); count++) {
                aktElem = CreateAssignNodesFromExprsNodes (aktElem, arg_info);
                aktElem = NODELIST_NEXT (aktElem);
            }

            if (div (CountVar (arg_info), 2).rem
                == 1) { /*concatenate last nodelist-element*/

                aktElem
                  = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/
                while (NODELIST_NEXT (aktElem) != NULL) {
                    secElem = aktElem;
                    aktElem = NODELIST_NEXT (aktElem);
                }

                newnode
                  = CreateAssignNodeFromAssignAndExprsNode (aktElem, secElem, arg_info);

                (INFO_AL_OPTVARIABLELIST (arg_info))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                NODELIST_NEXT ((INFO_AL_OPTVARIABLELIST (arg_info))) = NULL;

                FreeNodelist (aktElem);

                NODELIST_NODE (secElem) = newnode; /* store new node */
                NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
            }
        }
    }

    /*
     *  remove NULL-pointer-nodes
     */

    INFO_AL_CONSTANTLIST (arg_info)
      = RemoveNullpointerNodes ((INFO_AL_CONSTANTLIST (arg_info)));

    INFO_AL_VARIABLELIST (arg_info)
      = RemoveNullpointerNodes ((INFO_AL_VARIABLELIST (arg_info)));

    DBUG_RETURN (arg_info);
}

node *
CreateAssignNodeFromAssignAndExprsNode (nodelist *aktElem, nodelist *secElem,
                                        node *arg_info)
{

    /*  node *listElem, *listElem2, *newvardec, *newnode;
      char *newname1, *newname2, *mod;
      statustype status;
      types *type;*/
    node *newnode;

    DBUG_ENTER ("CreateAssignNodeFromAssignAndExprsNode");

    newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, aktElem);
    newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

    DBUG_RETURN (newnode);
}

nodelist *
CreateExprsNodes (nodelist *aktElem, int number_of_list_elements)
{
    int count;
    node *listElem;

    DBUG_ENTER ("CreateExprsNodes");

    for (count = 0; count < number_of_list_elements; count++) {
        listElem = NODELIST_NODE (aktElem); /*pointer on actual node stored in nodelist*/
        listElem = DupTree (listElem);      /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (aktElem) = listElem;
        aktElem = NODELIST_NEXT (aktElem);
    }
    /* all elements of the nodelist are N_expr nodes*/

    DBUG_RETURN (aktElem);
}

nodelist *
CreateAssignNodesFromExprsNodes (nodelist *aktElem, node *arg_info)
{

    nodelist *secElem;
    node *listElem, *listElem2;

    DBUG_ENTER ("CreateAssignNodesFromExprsNodes");

    secElem = NODELIST_NEXT (aktElem); /*pointer on next list element*/
    listElem = NODELIST_NODE (aktElem);
    listElem2 = NODELIST_NODE (secElem);

    EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
    listElem = MakeAssignNodeFromExprsNode (listElem, arg_info);

    NODELIST_NODE (aktElem) = listElem;
    NODELIST_NEXT (aktElem) = NODELIST_NEXT (secElem); /* remove second node*/
    secElem = aktElem;                                 /* store pointer on last element */

    DBUG_RETURN (aktElem);
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
    node *newnode, *elem1, *elem2;
    nodelist *lastListElem, *aktListElem, *constlist, *varlist;

    DBUG_ENTER ("CommitNAssignNodes");

    constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
    if (constlist != NULL) {
        while (NODELIST_NEXT (constlist) != NULL)
            constlist = NODELIST_NEXT (constlist);
    }

    varlist = INFO_AL_OPTVARIABLELIST (arg_info);
    if (varlist != NULL) {
        while (NODELIST_NEXT (varlist) != NULL)
            varlist = NODELIST_NEXT (varlist);
    }

    aktListElem = INFO_AL_CONSTANTLIST (arg_info);
    lastListElem = aktListElem;

    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    while (aktListElem != lastListElem) {
        elem1 = NODELIST_NODE (aktListElem);

        if (constlist != NULL) {
            NODELIST_NEXT (constlist) = aktListElem;
            constlist = NODELIST_NEXT (constlist);
        } else {
            INFO_AL_OPTCONSTANTLIST (arg_info) = aktListElem;
            constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
        }

        aktListElem = NODELIST_NEXT (aktListElem);
        elem2 = NODELIST_NODE (aktListElem);

        NODELIST_NEXT (constlist) = aktListElem;
        constlist = NODELIST_NEXT (constlist);

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
        newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);
        aktListElem = NODELIST_NEXT (aktListElem);
    }

    /* constant nodes ready */

    /* connect last constant node with first variable node */

    elem1 = NODELIST_NODE (lastListElem);

    if (constlist != NULL) {
        NODELIST_NEXT (constlist) = lastListElem;
        constlist = NODELIST_NEXT (constlist);
    } else {
        INFO_AL_OPTCONSTANTLIST (arg_info) = lastListElem;
        constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
    }
    NODELIST_NEXT (constlist) = NULL;

    aktListElem = INFO_AL_VARIABLELIST (arg_info);
    lastListElem = aktListElem;
    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    if (aktListElem != lastListElem) {

        elem2 = NODELIST_NODE (aktListElem);

        if (varlist != NULL) {
            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);
        } else {
            INFO_AL_OPTVARIABLELIST (arg_info) = aktListElem;
            varlist = INFO_AL_OPTVARIABLELIST (arg_info);
        }

        aktListElem = NODELIST_NEXT (aktListElem);

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
        newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        while (NODELIST_NEXT (aktListElem) != lastListElem) {
            elem1 = NODELIST_NODE (aktListElem);

            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);

            aktListElem = NODELIST_NEXT (aktListElem);
            elem2 = NODELIST_NODE (aktListElem);

            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);

            newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
            newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

            NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
            lastListElem = NODELIST_NEXT (lastListElem);

            aktListElem = NODELIST_NEXT (aktListElem);
        }

        /* variable nodes ready  */

        /* connect last two N_assign nodes of variable list */

        elem1 = NODELIST_NODE (aktListElem);
        elem2 = NODELIST_NODE (lastListElem);

        NODELIST_NEXT (varlist) = aktListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = lastListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = NULL;

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);

        elem1 = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        /* free elem1 */

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;

    } else {

        elem2 = NODELIST_NODE (lastListElem);

        if (varlist != NULL)
            NODELIST_NEXT (varlist) = lastListElem;
        else {
            INFO_AL_OPTVARIABLELIST (arg_info) = lastListElem;
            varlist = INFO_AL_OPTVARIABLELIST (arg_info);
        }

        if (NODELIST_NEXT (varlist) != NULL) {
            varlist = NODELIST_NEXT (varlist);
        }
        NODELIST_NEXT (varlist) = aktListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = NULL;

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);

        elem1 = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        /* free elem1 */

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;
    }
    NODELIST_NEXT (varlist) = NULL;

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

    if ((CountConst (arg_info) > 1) && (CountConst (arg_info) + CountVar (arg_info) > 3))
        DBUG_RETURN (1);
    else
        DBUG_RETURN (0);
}

nodelist *
RemoveNodelistNodes (nodelist *current_list)
{

    nodelist *temp = current_list;
    DBUG_ENTER ("RemoveNodelistNodes");

    if (temp != NULL) {
        while (NODELIST_NEXT (temp) != NULL) {
            NODELIST_NODE (temp) = NULL;
            temp = NODELIST_NEXT (temp);
        }
        NODELIST_NODE (temp) = NULL;
    }

    DBUG_RETURN (current_list);
}

nodelist *
RemoveNullpointerNodes (nodelist *source)
{

    nodelist *aktElem, *secElem;

    DBUG_ENTER ("RemoveNullpointerNodes");

    aktElem = source; /*Pointer on first list element*/
    ;
    secElem = NULL;
    while ((aktElem != NULL) && (NODELIST_NEXT (aktElem) != NULL)) {
        if (NODELIST_NODE (aktElem) == NULL) {
            if (secElem != NULL) {
                NODELIST_NEXT (secElem) = NODELIST_NEXT (aktElem);
                /*
                 * remove aktElem
                 */

                NODELIST_NEXT (aktElem) = NULL;
                FreeNodelist (aktElem);

                aktElem = secElem;
            } else {
                if (NODELIST_NEXT (aktElem) != NULL) {

                    secElem = aktElem;

                    source = NODELIST_NEXT (aktElem);

                    NODELIST_NEXT (secElem) = NULL;
                    FreeNodelist (secElem);
                    secElem = NULL;
                    aktElem = source;
                } else {
                    FreeNodelist (aktElem);
                    source = NULL;
                    break;
                }
            }
        }
        secElem = aktElem;
        aktElem = NODELIST_NEXT (aktElem);
    }

    if ((aktElem != NULL) && (NODELIST_NODE (aktElem) == NULL)) {
        FreeNodelist (aktElem);
        if (secElem != NULL)
            NODELIST_NEXT (secElem) = NULL;
    }

    DBUG_RETURN (source);
}

node *
MakeExprsNodeFromExprsAndAssignNode (nodelist *assignnode, nodelist *exprsnode)
{

    node *listElem, *listElem2, *newnode;
    char *newname1, *newname2, *mod;
    statustype status;

    DBUG_ENTER ("MakeExprsNodeFromExprsAndAssignNode");

    listElem = NODELIST_NODE (assignnode); /* pointer on last constant N_assign-node */
    listElem2 = NODELIST_NODE (exprsnode); /* pointer on node in list */

    newname2 = NULL;
    newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (listElem)));
    if (newname1 != NULL)
        newname2 = StringCopy (newname1);
    mod = IDS_MOD (LET_IDS (ASSIGN_INSTR (listElem)));
    if (mod != NULL)
        mod = StringCopy (mod);
    status = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (listElem)));

    newnode = MakeId (newname2, mod, status);
    newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */

    ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (listElem)));

    ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (listElem)));

    DBUG_RETURN (newnode);
}

node *
MakeAssignNodeFromExprsNode (node *newnode, node *arg_info)
{

    node *newvardec;
    types *type;
    char *newname1, *newname2;

    DBUG_ENTER ("MakeAssignNodeFromExprsNode");
    newnode = MakePrf (INFO_AL_CURRENTPRF (arg_info), newnode);

    type = MakeTypes (TYPES_BASETYPE ((INFO_AL_TYPE (arg_info))),
                      TYPES_DIM ((INFO_AL_TYPE (arg_info))), NULL, NULL, NULL);

    newname1 = TmpVar ();

    newvardec
      = MakeVardec (newname1, type, (BLOCK_VARDEC (INFO_AL_BLOCKNODE (arg_info))));

    BLOCK_VARDEC (INFO_AL_BLOCKNODE (arg_info)) = newvardec;

    newname2 = StringCopy (newname1);
    newnode = MakeAssignLet (newname2, newvardec, newnode);

    VARDEC_OBJDEF (newvardec) = newnode;
    AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
    AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

    DBUG_RETURN (newnode);
}

node *
MakeExprsNodeFromAssignNodes (node *elem1, node *elem2)
{

    node *newnode;
    statustype status1, status2;
    char *varname1, *newname1, *varmod1, *newmod1, *varname2, *newname2, *varmod2,
      *newmod2;

    DBUG_ENTER ("MakeExprsNodeFromAssignNodes");

    newname1 = NULL;
    newname2 = NULL;
    varname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
    if (varname1 != NULL)
        newname1 = StringCopy (varname1);
    varmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
    if (varmod1 != NULL)
        newmod1 = StringCopy (varmod1);
    else
        newmod1 = NULL;
    status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

    varname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
    if (varname2 != NULL)
        newname2 = StringCopy (varname2);
    varmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
    if (varmod2 != NULL)
        newmod2 = StringCopy (varmod2);
    else
        newmod2 = NULL;
    status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

    newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                         MakeExprs (MakeId (newname2, newmod2, status2), NULL));

    ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
    ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
      = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

    ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
    ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
      = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

    DBUG_RETURN (newnode);
}
