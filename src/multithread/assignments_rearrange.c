/*
 * $Log$
 * Revision 1.2  2004/04/30 14:10:05  skt
 * some debugging
 *
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
#include "print.h"

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

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "AssignmentsRearrange expects a N_modul as arg_node");

    /* push info ... */
    old_tab = act_tab;
    act_tab = asmra_tab;

    DBUG_PRINT ("ASMRA", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from modul-funs"));

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

    DBUG_PRINT ("ASMRA", ("Welcome to ASMRAfundef"));
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "node is not a N_fundef");

    INFO_ASMRA_WITHDEEP (arg_info) = 0;

    PrintNode (arg_node);

    /* Initialisation of the Dataflowgraph for this N_fundef */
    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;
    INFO_ASMRA_DATAFLOWGRAPH (arg_info)
      = CreateDataflowgraph (FUNDEF_NAME (arg_node), arg_info);

    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from body"));

    /* now the dataflowgraph is ready to be used for rearranging the assignment*/

    /* TODO */
    PrintDataflowgraph (INFO_ASMRA_DATAFLOWGRAPH (arg_info), FUNDEF_NAME (arg_node));

    DeleteDataflowgraph (INFO_ASMRA_DATAFLOWGRAPH (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("ASMRA", ("trav into fundef-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from fundef-next"));
    }

    DBUG_RETURN (arg_node);
}

node *
CreateDataflowgraph (char *name, node *arg_info)
{
    node *graph;
    DBUG_ENTER ("CreateDataflowgraph");

    /* build initial datanode */
    graph = MakeNode (name, NULL, arg_info);

    /* add the sink to the dataflowgraph */
    DATA_ASMRA_DEPENDENT (graph) = NodeListAppend (DATA_ASMRA_DEPENDENT (graph),
                                                   MakeNode (NULL, NULL, arg_info), NULL);

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
    /* TODO */

    status = 0;

    DBUG_RETURN (status);
}

int
PrintDataflowgraph (node *dataflowgraph, char *name)
{
    int status;
    nodelist *list_iterator;
    DBUG_ENTER ("PrintDataflowgraph");

    fprintf (stdout, "The Dataflowgraph for %s:\n", name);

    list_iterator = DATA_ASMRA_DEPENDENT (dataflowgraph);
    while (list_iterator != NULL) {
        PrintDataflownode (NODELIST_NODE (list_iterator));
        list_iterator = NODELIST_NEXT (list_iterator);
    }
    fprintf (stdout, "\n");
    DBUG_RETURN (status);
}

int
PrintDataflownode (node *datanode)
{
    int status;
    nodelist *list_iterator;
    DBUG_ENTER ("PrintDataflownode");

    if (DATA_ASMRA_NAME (datanode) != NULL) {
        fprintf (stdout, "- Name: %s, mode: %i\n", DATA_ASMRA_NAME (datanode),
                 DATA_ASMRA_EXECUTIONMODE (datanode));
    } else {
        fprintf (stdout, "- Return, mode: %i\n", DATA_ASMRA_EXECUTIONMODE (datanode));
    }

    list_iterator = DATA_ASMRA_DEPENDENT (datanode);

    if (list_iterator != NULL) {
        fprintf (stdout, "  ->");

        while (list_iterator != NULL) {
            if (NODE_TYPE (
                  ASSIGN_INSTR (DATA_ASMRA_INNERASSIGN (NODELIST_NODE (list_iterator))))
                != N_return) {
                fprintf (stdout, " %s,", DATA_ASMRA_NAME (NODELIST_NODE (list_iterator)));
            } else {
                fprintf (stdout, " Return");
            }
            list_iterator = NODELIST_NEXT (list_iterator);
        }
        fprintf (stdout, "\n");
    } else {
        fprintf (stdout, "  -> No dependent nodes\n");
    }

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

    /* if it's the return-instruction, one need not to traverse into the
       instruction, because the dataflownode for return already exists
    */
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return) {
        /* continue traversal */
        DBUG_PRINT ("ASMRA", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from instruction"));

        DBUG_PRINT ("ASMRA", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from next"));
    } else {
        /* but you have to update the returnassignment in the dataflowgraph*/
        INFO_ASMRA_DATAFLOWGRAPH (arg_info)
          = UpdateReturn (INFO_ASMRA_DATAFLOWGRAPH (arg_info), arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
GetReturnNode (node *graph)
{
    nodelist *list_iterator;

    DBUG_ENTER ("GetReturnNode");

    list_iterator = DATA_ASMRA_DEPENDENT (graph);

    while ((list_iterator != NULL)
           && (DATA_ASMRA_INNERASSIGN (NODELIST_NODE (list_iterator)) != NULL)) {
        list_iterator = NODELIST_NEXT (list_iterator);
    }
    DBUG_ASSERT (list_iterator != NULL, "the dataflowgraph has no returnnode!");

    DBUG_RETURN (NODELIST_NODE (list_iterator));
}

node *
UpdateReturn (node *graph, node *arg_node)
{
    node *returnnode;

    DBUG_ENTER ("UpdateReturn");

    returnnode = GetReturnNode (graph);

    DATA_ASMRA_INNERASSIGN (returnnode) = arg_node;
    DATA_ASMRA_OUTERASSIGN (returnnode) = arg_node;

    DBUG_RETURN (graph);
}

node *
ASMRAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAlet");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "node is not a N_let");

    if (INFO_ASMRA_WITHDEEP (arg_node) == 0) {
        /* create dataflownode out of the let-ids */
        DBUG_PRINT ("ASMRA", ("before MakeNode"));
        INFO_ASMRA_ACTNODE (arg_info)
          = MakeNode (IDS_NAME (LET_IDS (arg_node)),
                      AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (arg_node))), arg_info);
        DBUG_PRINT ("ASMRA", ("after MakeNode"));

        /* insert the node into the dataflowgraph */
        INFO_ASMRA_DATAFLOWGRAPH (arg_info)
          = AddNode (INFO_ASMRA_DATAFLOWGRAPH (arg_info), INFO_ASMRA_ACTNODE (arg_info));
    }
    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into expr"));
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from expr"));

    DBUG_RETURN (arg_node);
}

node *
MakeNode (char *name, node *inner_assign, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("MakeNode");

    tmp = MakeInfo ();

    DATA_ASMRA_NAME (tmp) = name;
    /* the inner assignment means, that its instruction is the N_let of the
       identifier */
    DATA_ASMRA_INNERASSIGN (tmp) = inner_assign;

    /* it is impossible to have a outer assign without an inner assign */
    if (inner_assign == NULL) {
        DATA_ASMRA_OUTERASSIGN (tmp) = NULL;
    } else {
        DATA_ASMRA_OUTERASSIGN (tmp) = INFO_ASMRA_OUTERASSIGN (arg_info);
    }

    DATA_ASMRA_EXECUTIONMODE (tmp) = INFO_ASMRA_EXECUTIONMODE (arg_info);
    DATA_ASMRA_DEPENDENT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

node *
AddNode (node *graph, node *newnode)
{
    DBUG_ENTER ("AddNode");

    /* node added to the graph -> made dependent from the spring */
    DATA_ASMRA_DEPENDENT (graph)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (graph), newnode, NULL);

    DATA_ASMRA_DEPENDENT (newnode)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (newnode), GetReturnNode (graph), NULL);
    DBUG_RETURN (graph);
}

node *
ASMRAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

    fprintf (stdout, "act. id = %s\n", ID_NAME (arg_node));

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

    fprintf (stdout, "UpdateDependencies for %s\n", DATA_ASMRA_NAME (actnode));

    /* is it a with_id?*/
    if (AVIS_WITHID (avisnode) == NULL) {
        /* no - let us have a closer look on it */

        /* Does the variable depend on an assignment ? */
        if (AVIS_SSAASSIGN (avisnode) != NULL) {
            list_iterator = DATA_ASMRA_DEPENDENT (graph);

            /* time to search for the corresponding node */
            while (DATA_ASMRA_INNERASSIGN (NODELIST_NODE (list_iterator))
                   != AVIS_SSAASSIGN (avisnode)) {
                list_iterator = NODELIST_NEXT (list_iterator);
                DBUG_ASSERT ((list_iterator != NULL),
                             "Variable without any correspondent lefthandside");
            }

            /* tmp points on the dataflownode, that has to be a father of actnode */
            tmp = NODELIST_NODE (list_iterator);

            fprintf (stdout, "%s ist the father of %s\n", DATA_ASMRA_NAME (tmp),
                     DATA_ASMRA_NAME (actnode));

            /* let us insert actnode into tmp's dependent_nodes, if this has not be
               done yet */
            if (NodeListFind (DATA_ASMRA_DEPENDENT (tmp), actnode) == NULL) {
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

node *
ASMRAst (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAst");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_st), "node is not a N_st");

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ST;

    DBUG_PRINT ("ASMRA", ("trav into st-region"));
    ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from st-region"));

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;

    DBUG_RETURN (arg_node);
}

node *
ASMRAmt (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAmt");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_mt), "node is not a N_mt");

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_MT;

    DBUG_PRINT ("ASMRA", ("trav into mt-region"));
    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from mt-region"));

    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;

    DBUG_RETURN (arg_node);
}

node *
ASMRAwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRANwith2");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith2), "node is not a N_Nwith2");

    fprintf (stdout, "Hello again!\n");

    /* increase the "deepness of the withloop"-counter */
    INFO_ASMRA_WITHDEEP (arg_info)++;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /* restore actual deepness */
    INFO_ASMRA_WITHDEEP (arg_info)--;

    DBUG_RETURN (arg_node);
}

/**
 * @}
 **/
