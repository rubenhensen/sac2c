

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

        /* store pointer on actual N_block-node for append of new N_vardec nodes */
        (*arg_info).node[0] = arg_node;
        ((types *)(arg_info->dfmask[0])) = VARDEC_TYPE (BLOCK_VARDEC (arg_node));

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

            /* clear nodelists in arg_info  */
            FreeNodelist ((nodelist *)(arg_info->info2));
            FreeNodelist ((nodelist *)(arg_info->info3));
            (arg_info->info2) = NULL;
            (arg_info->info3) = NULL;
            (*arg_info).varno = 0;
            (*arg_info).counter = 0;
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
        (*arg_info).node[1] = arg_node;
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
    case F_add_AxA:
    case F_mul_AxA:
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
        /*    nl = ((nodelist *) (arg_info->info2));*/
        (*arg_info).counter = (*arg_info).counter + 1;
    }

    else {
        /*    nl = ((nodelist *) (arg_info->info3));*/
        (*arg_info).varno = (*arg_info).varno + 1;
    }
    /*  if (nl == NULL){
        nl=newnodelistnode;
      }
      else{
        while (NODELIST_NEXT(nl) != NULL){
          nl=NODELIST_NEXT(nl);
        }
      }
    */
    if (constant == 1) {
        NODELIST_NEXT (newnodelistnode) = ((nodelist *)(arg_info->info2));
        (nodelist *)(arg_info->info2) = newnodelistnode;
    } else {
        NODELIST_NEXT (newnodelistnode) = ((nodelist *)(arg_info->info3));
        (nodelist *)(arg_info->info3) = newnodelistnode;
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
    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))) == NULL)
            otherOp = 0;
        else {
            otherPrf = PRF_PRF (
              LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))));

            if ((*arg_info).info.prf == otherPrf)
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
    int count;
    node *listElem, *listElem2, *newvardec, *newnode;
    char *newname, *mod;
    nodelist *aktElem, *secElem;
    statustype status;
    types *type;

    DBUG_ENTER ("CreateNAssignNodes");

    aktElem = (nodelist *)(arg_info->info2); /*Pointer on first list element*/
    secElem = NULL;
    for (count = 0; count < CountConst (arg_info); count++) {
        listElem = NODELIST_NODE (aktElem); /*pointer on actual node stored in nodelist*/
        listElem = DupTree (listElem);      /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (aktElem) = listElem;
        aktElem = NODELIST_NEXT (aktElem);
    }
    /* all elements of the nodelist are N_expr nodes*/

    aktElem = (nodelist *)(arg_info->info2); /*pointer on first list element*/
    for (count = 0; count < (div (CountConst (arg_info), 2).quot); count++) {
        secElem = NODELIST_NEXT (aktElem); /*pointer on next list element*/
        listElem = NODELIST_NODE (aktElem);
        listElem2 = NODELIST_NODE (secElem);

        EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
        listElem = MakePrf ((*arg_info).info.prf, listElem);

        /* to do: create new vardec node
         *        create new ids
         *        create new name    */

        /*    strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( (  (types*)(arg_info->dfmask[0])  ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname = TmpVar (); /* create new name */

        newvardec = MakeVardec (newname, type,
                                (
                                  BLOCK_VARDEC (
                                    arg_info->node[0]))); /* create new N_vardec node,
                                                             next pointer on first
                                                             n_vardec of actual N_block */

        BLOCK_VARDEC (arg_info->node[0])
          = newvardec; /* newvardec as first n_vardec node */

        /* create new N_let, new ids and new N_assign - node */
        listElem = MakeAssignLet (newname, newvardec, listElem);

        VARDEC_OBJDEF (newvardec) = listElem;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = listElem;
        /* ??? */ AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = listElem;
        NODELIST_NODE (aktElem) = listElem;
        NODELIST_NEXT (aktElem) = NODELIST_NEXT (secElem); /* remove second node*/
        secElem = aktElem; /* store pointer on last element */
        aktElem = NODELIST_NEXT (aktElem);
    }

    if (div (CountConst (arg_info), 2).rem == 1) { /*concatenate last nodelist-element*/

        listElem = NODELIST_NODE (secElem);  /* pointer on last N_assign-node */
        listElem2 = NODELIST_NODE (aktElem); /* pointer on last node in list */
                                             /* to do: create new name, mod, statustype */

        newname = IDS_NAME (LET_IDS (ASSIGN_INSTR (listElem)));
        if (newname != NULL)
            strcpy (newname, newname);
        mod = IDS_MOD (LET_IDS (ASSIGN_INSTR (listElem)));
        if (mod != NULL)
            strcpy (mod, mod);
        status = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (listElem)));

        newnode = MakeId (newname, mod, status);
        ID_AVIS (newnode) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (listElem)));
        ID_VARDEC (newnode) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (listElem)));

        /* to do: connect new N_id-node with correct N_AVIS */
        newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */
        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new vardec node
         *        create new ids
         *        create new name       */

        /* strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname = TmpVar (); /* create new name */

        newvardec = MakeVardec (newname, type, /*( (arg_info->info).types ),*/
                                (
                                  BLOCK_VARDEC (
                                    arg_info->node[0]))); /* create new N_vardec node,
                                                             next pointer on first
                                                             n_vardec of actual N_block */

        BLOCK_VARDEC (arg_info->node[0])
          = newvardec; /* newvardec as first n_vardec node */

        /* create new N_let, new ids and new N_assign - node */
        newnode = MakeAssignLet (newname, newvardec, newnode);

        VARDEC_OBJDEF (newvardec) = newnode;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
        AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

        NODELIST_NODE (secElem) = newnode; /* store new node */
        NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
    }

    /* nodelist for constant nodes ready */

    aktElem = (nodelist *)(arg_info->info3); /*Pointer on first list element*/

    for (count = 0; count < (CountVar (arg_info)); count++) {
        listElem = NODELIST_NODE (aktElem); /*pointer on actual node stored in nodelist*/
        listElem = DupTree (listElem);      /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (aktElem) = listElem;
        aktElem = NODELIST_NEXT (aktElem);
    }
    /* all elements of the nodelist are N_expr nodes*/

    aktElem = (nodelist *)(arg_info->info3); /*pointer on first list element*/
    for (count = 0; count < (div (CountVar (arg_info), 2).quot); count++) {
        secElem = NODELIST_NEXT (aktElem); /*pointer on next list element*/
        listElem = NODELIST_NODE (aktElem);
        listElem2 = NODELIST_NODE (secElem);

        EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
        listElem = MakePrf ((*arg_info).info.prf, listElem);

        /* to do: create new vardec node
         *        create new ids
         *        create new name    */

        /* strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname = TmpVar (); /* create new name */

        newvardec = MakeVardec (newname, type,
                                (
                                  BLOCK_VARDEC (
                                    arg_info->node[0]))); /* create new N_vardec node,
                                                             next pointer on first
                                                             n_vardec of actual N_block */

        BLOCK_VARDEC (arg_info->node[0])
          = newvardec; /* newvardec as first n_vardec node */

        /* create new N_let, new ids and new N_assign - node */
        listElem = MakeAssignLet (newname, newvardec, listElem);

        VARDEC_OBJDEF (newvardec) = listElem;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = listElem;
        AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = listElem;
        NODELIST_NODE (aktElem) = listElem;
        NODELIST_NEXT (aktElem) = NODELIST_NEXT (secElem); /* remove second node*/
        secElem = aktElem; /* store pointer on last element */
        aktElem = NODELIST_NEXT (aktElem);
    }

    if (div (CountVar (arg_info), 2).rem == 1) { /*concatenate last nodelist-element*/

        listElem = NODELIST_NODE (secElem);  /* pointer on last N_assign-node */
        listElem2 = NODELIST_NODE (aktElem); /* pointer on last node in list */
                                             /* to do: create new name, mod, statustype */

        newname = IDS_NAME (LET_IDS (ASSIGN_INSTR (listElem)));
        if (newname != NULL)
            strcpy (newname, newname);
        mod = IDS_MOD (LET_IDS (ASSIGN_INSTR (listElem)));
        if (mod != NULL)
            strcpy (mod, mod);
        status = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (listElem)));

        newnode = MakeId (newname, mod, status);

        /* to do: connect new N_id-node with correct N_AVIS */
        newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */
        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new vardec node
         *        create new ids
         *        create new name       */

        /* strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname = TmpVar (); /* create new name */

        newvardec = MakeVardec (newname, type,
                                (
                                  BLOCK_VARDEC (
                                    arg_info->node[0]))); /* create new N_vardec node,
                                                             next pointer on first
                                                             n_vardec of actual N_block */

        BLOCK_VARDEC (arg_info->node[0])
          = newvardec; /* newvardec as first n_vardec node */

        /* create new N_let, new ids and new N_assign - node */
        newnode = MakeAssignLet (newname, newvardec, newnode);

        VARDEC_OBJDEF (newvardec) = newnode;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
        AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

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
    node *newnode, *newvardec, *elem1, *elem2;
    char *newname1, *newname2, *newmod1, *newmod2;
    nodelist *lastListElem, *aktListElem;
    statustype status1, status2;
    types *type;

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

        /**/

        newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newname1 != NULL)
            strcpy (newname1, newname1);
        newmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newmod1 != NULL)
            strcpy (newmod1, newmod1);
        status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

        newname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newname2 != NULL)
            strcpy (newname2, newname2);
        newmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newmod2 != NULL)
            strcpy (newmod2, newmod2);
        status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

        /**/

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeExprs (MakeId (newname2, newmod2, status2), NULL));

        ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

        ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new name
         *        create new ids node
         *        create new vardec node  */

        /* strcpy( newname ,  TYPES_NAME( (  (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname1 = TmpVar ();

        newvardec = MakeVardec (newname1, type, (BLOCK_VARDEC (arg_info->node[0])));

        BLOCK_VARDEC (arg_info->node[0]) = newvardec;

        newnode = MakeAssignLet (newname1, newvardec, newnode);

        VARDEC_OBJDEF (newvardec) = newnode;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
        AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        aktListElem = NODELIST_NEXT (aktListElem);
    }

    /* constant nodes ready  */

    /* conect last constant node with first variable node */

    elem1 = NODELIST_NODE (lastListElem);
    aktListElem = (nodelist *)(arg_info->info3);
    lastListElem = aktListElem;
    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    if (aktListElem != lastListElem) {

        elem2 = NODELIST_NODE (aktListElem);
        aktListElem = NODELIST_NEXT (aktListElem);

        /* to do: create new name, mod , statustype */

        /**/

        newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newname1 != NULL)
            strcpy (newname1, newname1);
        newmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newmod1 != NULL)
            strcpy (newmod1, newmod1);
        status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

        newname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newname2 != NULL)
            strcpy (newname2, newname2);
        newmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newmod2 != NULL)
            strcpy (newmod2, newmod2);
        status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

        /**/

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeExprs (MakeId (newname2, newmod2, status2), NULL));

        ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

        ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

        newnode = MakePrf ((*arg_info).info.prf, newnode);

        /* to do: create new name
         *        create new ids node
         *        create new vardec node  */

        /* strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
            strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

        type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                          TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL, NULL);

        newname1 = TmpVar ();

        newvardec = MakeVardec (newname1, type, (BLOCK_VARDEC (arg_info->node[0])));

        BLOCK_VARDEC (arg_info->node[0]) = newvardec;

        newnode = MakeAssignLet (newname1, newvardec, newnode);

        VARDEC_OBJDEF (newvardec) = newnode;
        AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
        AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        while (NODELIST_NEXT (aktListElem) != lastListElem) {
            elem1 = NODELIST_NODE (aktListElem);
            aktListElem = NODELIST_NEXT (aktListElem);
            elem2 = NODELIST_NODE (aktListElem);

            /* to do: create new name, mod , statustype */

            /**/

            newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
            if (newname1 != NULL)
                strcpy (newname1, newname1);
            newmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
            if (newmod1 != NULL)
                strcpy (newmod1, newmod1);
            status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

            newname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
            if (newname2 != NULL)
                strcpy (newname2, newname2);
            newmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
            if (newmod2 != NULL)
                strcpy (newmod2, newmod2);
            status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

            /**/

            newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                                 MakeExprs (MakeId (newname2, newmod2, status2), NULL));

            ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
            ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
              = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

            ID_VARDEC (EXPRS_EXPR (newnode))
              = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
            ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
              = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));
            newnode = MakePrf ((*arg_info).info.prf, newnode);

            /* to do: create new name
             *        create new ids node
             *        create new vardec node  */

            /* strcpy( newname ,  TYPES_NAME( ( (types*)(arg_info->dfmask[0]) ) ) );
                strcpy( mod , TYPES_MOD( ( (types*)(arg_info->dfmask[0]) ) ) );*/

            type = MakeTypes (TYPES_BASETYPE (((types *)(arg_info->dfmask[0]))),
                              TYPES_DIM (((types *)(arg_info->dfmask[0]))), NULL, NULL,
                              NULL);

            newname1 = TmpVar ();

            newvardec = MakeVardec (newname1, type, (BLOCK_VARDEC (arg_info->node[0])));

            BLOCK_VARDEC (arg_info->node[0]) = newvardec;

            newnode = MakeAssignLet (newname1, newvardec, newnode);

            VARDEC_OBJDEF (newvardec) = newnode;
            AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
            AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

            NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
            lastListElem = NODELIST_NEXT (lastListElem);

            aktListElem = NODELIST_NEXT (aktListElem);
        }

        /* variable nodes ready  */

        /* connect last two N_assign nodes of variable list */

        elem1 = NODELIST_NODE (aktListElem);
        elem2 = NODELIST_NODE (lastListElem);

        /* to do create new name, mod, statustype */

        /**/

        newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newname1 != NULL)
            strcpy (newname1, newname1);
        newmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newmod1 != NULL)
            strcpy (newmod1, newmod1);
        status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

        newname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newname2 != NULL)
            strcpy (newname2, newname2);
        newmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newmod2 != NULL)
            strcpy (newmod2, newmod2);
        status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

        /**/

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeExprs (MakeId (newname2, newmod2, status2), NULL));

        ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

        ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

        elem1 = PRF_ARGS (arg_info->node[1]);

        /* free elem1 */

        PRF_ARGS (arg_info->node[1]) = newnode;

        /* besser: Pointer auf aktuell zubearbeitenden N_assign-node (N_prf !!!) merken
         *         und dortige N_exprs-nodes durch die neu erzeugeten ersetzen
         *         => diesen Knoten nicht mehr in nodelist aufnehmen!
         */

    } else {

        elem2 = NODELIST_NODE (lastListElem);

        /* to do create new name, mod, statustype */

        /**/

        newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newname1 != NULL)
            strcpy (newname1, newname1);
        newmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
        if (newmod1 != NULL)
            strcpy (newmod1, newmod1);
        status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

        newname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newname2 != NULL)
            strcpy (newname2, newname2);
        newmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
        if (newmod2 != NULL)
            strcpy (newmod2, newmod2);
        status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

        /**/

        newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                             MakeExprs (MakeId (newname2, newmod2, status2), NULL));

        ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

        ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
        ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
          = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

        elem1 = PRF_ARGS (arg_info->node[1]);

        /* free elem1 */

        PRF_ARGS (arg_info->node[1]) = newnode;
    }
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
