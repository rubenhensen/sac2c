/*
 * $Log$
 * Revision 1.1  2004/04/27 09:59:04  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup asmra Assignments Rearrange
 * @ingroup muth
 *
 * @brief Rearranges the assignments of each function to avoid superflous synchronisation
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file assignments_rearrange.c
 *
 * prefix: ASMRA
 *
 * description:
 *   rearranges the assignment of each funcion to avoid superflous
 *   synchronisations
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "assignments_rearrange.h"
/*#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "my_debug.h"
#include "multithread_lib.h"

#include "internal_lib.h"*/

/******************************************************************************
 *
 * function:
 *   node *AssignmentsRearrange(node *arg_node, node *arg_info)
 *
 * description:
 *   Inits the traversal for this phase
 *
 ******************************************************************************/
node *
AssignmentsRearrange (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("AssignmentsRearrange");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "AssignmentsRearrange expects a N_fundef as arg_node");

    /* push info ... */
    old_tab = act_tab;
    act_tab = asmra_tab;

    FUNDEF_COMPANION (arg_node) = NULL;

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* pop info ... */
    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *ASMRAfundef(node *arg_node, node *arg_info)
 *
 * @brief Rearrange the assignments of the function
 *
 ******************************************************************************/
node *
ASMRAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAfundef");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "node is not a N_fundef");

    INFO_ASMRA_WITHDEEP (arg_info) = 0;

    /* Initialisation of the Dataflowgraph for this N_fundef */
    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;
    INFO_ASMRA_DATAFLOWGRAPH (arg_info)
      = CreateDataflowgraph (FUNDEF_ARGS (arg_node), INFO_ASMRA_EXECUTIONMODE (arg_info));

    /* continue traversal */
    arg_node = Trav (arg_node, arg_info);

    /* now the dataflowgraph is ready to be used for rearranging the assignment*/

    /* TODO */

    DeleteDataflowgraph (INFO_ASMRA_DATAFLOWGRAPH (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateDataflowgraph(node *args, int mode)
 *
 *   @brief creates a new dataflowgraph
 *
 *   @param args arguments of a function, used to build the first node
 *   @param mode the execution mode of the function
 *   @return returns a new dataflowgraph with one initial node
 *
 *****************************************************************************/
node *
CreateDataflowgraph (node *args, int mode)
{
    node *graph;

    DBUG_ENTER ("CreateDataflowgraph");

    DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "node is not a N_arg");

    /* build initial datanode */
    graph = MakeInfo ();
    DATA_ASMRA_INNERASSIGN (graph) = NULL;
    DATA_ASMRA_OUTERASSIGN (graph) = NULL;
    DATA_ASMRA_EXECUTIONMODE (graph) = mode;
    DATA_ASMRA_DEPENDENT (graph) = NULL;

    /* return datanode as new graph */
    DBUG_RETURN (graph);
}

int
DeleteDataflowgraph (node *graph)
{
    int status;
    /*nodelist *list_iterator;*/
    DBUG_ENTER ("DeleteDataflowgraph");
    /* iterate over all nodes and deletes their nodelists, then delete the "main"
       nodelist */

    status = 0;

    DBUG_RETURN (status);
}

node *
ASMRAassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "node is not a N_assign");

    /* there's a problem concerning finding the correct assign belonging to a
       N_let (the new nodes of the dataflowgraph are build in N_let-nodes):
       the current structure is
        N_block
         |
        N_assign(a)
         |
        N_assign(b1) -> N_mt -> N_Block -> N_assign(b2) -> instr
         |
        N_assign(c1) -> N_st -> N_Block -> N_assign(c2) -> instr
        ...
       the corresponding N_avis-nodes point to (a),(b2) and (c2), but if you want
       to rearrange the assignment-chain, you need pointer to (a),(b1) and (c1);
       so you habe to keep the assign in your mind if the current executionmode
       is ASMRA_ANY */

    if (INFO_ASMRA_EXECUTIONMODE (arg_info) == ASMRA_ANY) {
        INFO_ASMRA_OUTERASSIGN (arg_info) = arg_node;
    }

    /* if it's the return-instruction, one has to create the dataflownode BEFORE
       continuing the traversal! */
    if (NODE_TYPE (ASSIGN_INSTR (arg_info)) == N_return) {
        INFO_ASMRA_ACTNODE (arg_info) = MakeReturnnode (arg_node, arg_info);
    }

    /* continue traversal */
    arg_node = Trav (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ASMRAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAlet");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "node is not a N_let");

    if (INFO_ASMRA_WITHDEEP (arg_node) == 0) {
        /* create dataflownode out of the let-ids */
        INFO_ASMRA_ACTNODE (arg_info) = MakeNode (LET_IDS (arg_node), arg_info);

        /* insert the node into the dataflowgraph */
        INFO_ASMRA_DATAFLOWGRAPH (arg_info)
          = AddNode (INFO_ASMRA_DATAFLOWGRAPH (arg_info), INFO_ASMRA_ACTNODE (arg_info));
    }
    /* continue traversal */
    arg_node = Trav (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
MakeNode (ids *identifier, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("MakeNode");

    tmp = MakeInfo ();
    /* the inner assignment means, that its instruction is the N_let of the
       identifier */
    DATA_ASMRA_INNERASSIGN (tmp) = AVIS_SSAASSIGN (IDS_AVIS (identifier));
    /* the outer assignment equals the inner one, if mode = ASMRA_ANY, otherwise
       it is different - see ASMRAassign for more details */
    DATA_ASMRA_OUTERASSIGN (tmp) = INFO_ASMRA_OUTERASSIGN (arg_info);
    DATA_ASMRA_EXECUTIONMODE (tmp) = INFO_ASMRA_EXECUTIONMODE (arg_info);
    DATA_ASMRA_DEPENDENT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

node *
MakeReturnnode (node *assign_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("MakeReturnnode");

    tmp = MakeInfo ();
    DATA_ASMRA_INNERASSIGN (tmp) = assign_node;
    /* the outer assignment equals the inner one, if mode = ASMRA_ANY, otherwise
       it is different - see ASMRAassign for more details */
    DATA_ASMRA_OUTERASSIGN (tmp) = INFO_ASMRA_OUTERASSIGN (arg_info);
    DATA_ASMRA_EXECUTIONMODE (tmp) = INFO_ASMRA_EXECUTIONMODE (arg_info);
    DATA_ASMRA_DEPENDENT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

node *
AddNode (node *graph, node *newnode)
{

    DBUG_ENTER ("AddNode");

    DATA_ASMRA_DEPENDENT (graph)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (graph), newnode, NULL);

    DBUG_RETURN (graph);
}

node *
ASMRAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

    INFO_ASMRA_DATAFLOWGRAPH (arg_info)
      = UpdateDependencies (INFO_ASMRA_DATAFLOWGRAPH (arg_info), ID_AVIS (arg_node),
                            INFO_ASMRA_ACTNODE (arg_info));

    DBUG_RETURN (arg_node);
}

node *
UpdateDependencies (node *graph, node *avisnode, node *actnode)
{
    nodelist *list_iterator;
    node *tmp;
    DBUG_ENTER ("UpdateDependencies");

    DBUG_ASSERT ((NODE_TYPE (avisnode) == N_avis), "node is not a N_avis");

    /* is it a with_id?*/
    if (AVIS_WITHID (avisnode) != NULL) {
        /* no - let us have a closer look on it */

        /* Does the varible depend on an assignment ? */
        if (AVIS_SSAASSIGN (avisnode) == NULL) {
            /* time to search for the corresponding node */
            list_iterator = DATA_ASMRA_DEPENDENT (graph);
            DBUG_ASSERT ((list_iterator != NULL), "unexpected empty list");

            while (DATA_ASMRA_INNERASSIGN (NODELIST_NODE (list_iterator))
                   != AVIS_SSAASSIGN (avisnode)) {
                list_iterator = NODELIST_NEXT (list_iterator);
                DBUG_ASSERT ((list_iterator != NULL), "unexpected end of list");
            }

            /* tmp points on the dataflownode, that has to be a father of actnode */
            tmp = NODELIST_NODE (list_iterator);

            /* let us insert actnode into tmp's dependent_nodes, if this has not be
               done yet */
            if (NodeListFind (DATA_ASMRA_DEPENDENT (tmp), actnode) != NULL) {
                DATA_ASMRA_DEPENDENT (tmp)
                  = NodeListAppend (DATA_ASMRA_DEPENDENT (tmp), actnode, NULL);
            }
        } else { /* AVIS_SSAASSIGN */
                 /* nothing to do - by default it depends on the spring of the graph*/
        }
    } /* AVIS_WITHID */
    else {
        /* Yes - we can ignore it for the dataflowgraph */
    }
    DBUG_RETURN (graph);
}

/*node *ASMRAreturn(node *arg_node, node *arg_info)
{
  DBUG_ENTER("ASMRAreturn");

  DBUG_ASSERT((NODE_TYPE(arg_node) == N_return), "node is not a N_return");



  DBUG_RETURN(arg_node);
  }*/

node *
ASMRAst (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAst");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_st), "node is not a N_st");

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ST;

    ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;

    DBUG_RETURN (arg_node);
}

node *
ASMRAmt (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAmt");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_mt), "node is not a N_mt");

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_MT;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;

    DBUG_RETURN (arg_node);
}

node *
ASMRANwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRANwith2");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith2), "node is not a N_Nwith2");

    /* increase the "deepness of the withloop"-counter */
    INFO_ASMRA_WITHDEEP (arg_info)++;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);

    /* restore actual deepness */
    INFO_ASMRA_WITHDEEP (arg_info)--;

    DBUG_RETURN (arg_node);
}

/**
 * @}
 **/
