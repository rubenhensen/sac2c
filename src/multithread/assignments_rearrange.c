/*
 * $Log$
 * Revision 1.4  2004/08/05 10:52:22  skt
 * initialization of border & new_borderelem added
 *
 * Revision 1.3  2004/07/29 00:41:50  skt
 * build compilable intermediate version
 * work in progress
 *
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

/* todo - loops & conditionals */

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

    /* Initialisation of the Dataflowgraph for this N_fundef */
    INFO_ASMRA_EXECUTIONMODE (arg_info) = ASMRA_ANY;
    INFO_ASMRA_DATAFLOWGRAPH (arg_info)
      = CreateDataflowgraphASMRA (FUNDEF_NAME (arg_node), arg_info);

    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from body"));

    /* now the dataflowgraph is ready to be used for rearranging the assignment*/

    PrintDataflowgraphASMRA (INFO_ASMRA_DATAFLOWGRAPH (arg_info), FUNDEF_NAME (arg_node));

    arg_node = Rearrange (arg_node, INFO_ASMRA_DATAFLOWGRAPH (arg_info));
    /* TODO */

    DeleteDataflowgraphASMRA (INFO_ASMRA_DATAFLOWGRAPH (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("ASMRA", ("trav into fundef-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from fundef-next"));
    }

    DBUG_RETURN (arg_node);
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
        DBUG_PRINT ("ASMRA", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from instruction"));
    } else {
        /* but you have to update the returnassignment in the dataflowgraph*/
        INFO_ASMRA_DATAFLOWGRAPH (arg_info)
          = UpdateReturnASMRA (INFO_ASMRA_DATAFLOWGRAPH (arg_info), arg_node);
    }

    /* continue traversal */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("ASMRA", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("ASMRA", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

node *
ASMRAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAlet");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "node is not a N_let");

    if (INFO_ASMRA_WITHDEEP (arg_node) == 0) {
        /* create dataflownode out of the let-ids */
        DBUG_PRINT ("ASMRA", ("before MakeDataflowNode"));
        INFO_ASMRA_ACTNODE (arg_info)
          = MakeDataflowNodeASMRA (IDS_NAME (LET_IDS (arg_node)),
                                   AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (arg_node))),
                                   arg_info);
        DBUG_PRINT ("ASMRA", ("after MakeDataflowNode"));

        /* insert the node into the dataflowgraph */
        INFO_ASMRA_DATAFLOWGRAPH (arg_info)
          = AddDataflowNodeASMRA (INFO_ASMRA_DATAFLOWGRAPH (arg_info),
                                  INFO_ASMRA_ACTNODE (arg_info));
    }
    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into expr"));
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from expr"));

    DBUG_RETURN (arg_node);
}

node *
ASMRAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ASMRAid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

    /*fprintf(stdout,"act. id = %s\n",ID_NAME(arg_node));*/
    INFO_ASMRA_DATAFLOWGRAPH (arg_info)
      = UpdateDependenciesASMRA (INFO_ASMRA_DATAFLOWGRAPH (arg_info), ID_AVIS (arg_node),
                                 INFO_ASMRA_ACTNODE (arg_info));

    DBUG_RETURN (arg_node);
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

node *
Rearrange (node *arg_node, node *dataflowgraph)
{
    int act_executionmode;
    int border_increases;
    int subgraph_counter;
    nodelist *border;
    nodelist *current_subgraph;
    nodelist *all_subgraphs;
    DBUG_ENTER ("rearrange");

    /* some initialisations */
    act_executionmode = DATA_ASMRA_EXECUTIONMODE (dataflowgraph);
    subgraph_counter = 1;
    current_subgraph = NULL;
    all_subgraphs = NULL;
    border_increases = 1;
    border = NULL;

    /* the spring of the dataflowgraph is first element of the border */
    border = NodeListAppend (border, dataflowgraph, NULL);

    /* status -1 means: already used for border (or subgraph) */
    DATA_ASMRA_STATUS (NODELIST_NODE (border)) = -1;

    /* creation of individual execution blocks */
    while (border != NULL) {
        while (border_increases) {
            current_subgraph = AddBorderElements (current_subgraph, border,
                                                  act_executionmode, subgraph_counter);
            border_increases = IncreaseBorder (border, dataflowgraph);
        }
        all_subgraphs = ConcatNodelist (all_subgraphs, current_subgraph);
        subgraph_counter++;
        current_subgraph = NULL;
    }

    /* arranging the assignments in this blocks */

    DBUG_RETURN (arg_node);
}

nodelist *
AddBorderElements (nodelist *subgraph, nodelist *border, int ex_mode, int counter)
{
    nodelist *list_iterator;
    nodelist *tmp;
    DBUG_ENTER ("AddBorderElements");

    list_iterator = border;

    while (list_iterator != NULL) {
        fprintf (stdout, "current executionmode: %i\n",
                 DATA_ASMRA_EXECUTIONMODE (NODELIST_NODE (list_iterator)));
        if (DATA_ASMRA_EXECUTIONMODE (NODELIST_NODE (list_iterator)) == ex_mode) {
            DATA_ASMRA_STATUS (NODELIST_NODE (list_iterator)) = counter;
            tmp = subgraph;
            subgraph = list_iterator;

            if (border == list_iterator) {
                /* uups - we've got the first element of the border, so let's chance
                   its pointer, too */
                border = NODELIST_NEXT (border);
            }

            list_iterator = NODELIST_NEXT (list_iterator);
            NODELIST_NEXT (subgraph) = tmp;
        } else {
            list_iterator = NODELIST_NEXT (list_iterator);
        }
    }

    DBUG_RETURN (subgraph);
}

int
IncreaseBorder (nodelist *border, node *dataflowgraph)
{
    nodelist *list_iterator;
    nodelist *new_borderelems;
    int border_increases;
    DBUG_ENTER ("IncreaseBorder");

    new_borderelems = NULL;
    list_iterator = DATA_ASMRA_DEPENDENT (dataflowgraph);
    /* walk through the dataflowgraph and add all nodes to the border, which
       refcouter equals 0
    */
    while (list_iterator != NULL) {
        if (DATA_ASMRA_NODEREFCOUNT (NODELIST_NODE (list_iterator)) == 0) {
            new_borderelems
              = NodeListAppend (new_borderelems, NODELIST_NODE (list_iterator), NULL);
        }
        list_iterator = NODELIST_NEXT (list_iterator);
    }

    if (new_borderelems == NULL) {
        border_increases = 0;
    } else {
        border_increases = 1;
    }

    /* decrease the referencecounter of the nodes, which depends on the new
       border elements and add the new border elements to the current border */
    while (new_borderelems != NULL) {
        list_iterator = DATA_ASMRA_DEPENDENT (NODELIST_NODE (new_borderelems));
        while (list_iterator != NULL) {
            DATA_ASMRA_NODEREFCOUNT (NODELIST_NODE (list_iterator))--;
            list_iterator = NODELIST_NEXT (list_iterator);
        }
        border = NodeListAppend (border, NODELIST_NODE (new_borderelems), NULL);
        new_borderelems = NODELIST_NEXT (new_borderelems);
    }

    DBUG_RETURN (border_increases);
}

node *
CreateDataflowgraphASMRA (char *name, node *arg_info)
{
    node *graph;
    DBUG_ENTER ("CreateDataflowgraphASMRA");

    /* build initial datanode */
    graph = MakeDataflowNodeASMRA (name, NULL, arg_info);

    /* add the sink to the dataflowgraph */
    DATA_ASMRA_DEPENDENT (graph)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (graph),
                        MakeDataflowNodeASMRA (NULL, NULL, arg_info), NULL);

    /* return datanode as new graph */
    DBUG_RETURN (graph);
}

int
DeleteDataflowgraphASMRA (node *graph)
{
    int status;
    nodelist *list_iterator;

    DBUG_ENTER ("DeleteDataflowgraph");
    /* iterate over all nodes and deletes their nodelists, then delete the "main"
       nodelist */

    list_iterator = DATA_ASMRA_DEPENDENT (graph);

    while (list_iterator != NULL) {
        DATA_ASMRA_DEPENDENT (NODELIST_NODE (list_iterator))
          = NodeListFree (DATA_ASMRA_DEPENDENT (NODELIST_NODE (list_iterator)), TRUE);
        list_iterator = NODELIST_NEXT (list_iterator);
    }

    DATA_ASMRA_DEPENDENT (graph) = NodeListFree (DATA_ASMRA_DEPENDENT (graph), TRUE);

    graph = Free (graph);

    status = 0;

    DBUG_RETURN (status);
}

int
PrintDataflowgraphASMRA (node *dataflowgraph, char *name)
{
    int status;
    nodelist *list_iterator;
    DBUG_ENTER ("PrintDataflowgraph");
    status = 0;

    fprintf (stdout, "The Dataflowgraph for %s:\n", name);

    list_iterator = DATA_ASMRA_DEPENDENT (dataflowgraph);
    while (list_iterator != NULL) {
        PrintDataflownodeASMRA (NODELIST_NODE (list_iterator));
        list_iterator = NODELIST_NEXT (list_iterator);
    }
    fprintf (stdout, "\n");
    DBUG_RETURN (status);
}

int
PrintDataflownodeASMRA (node *datanode)
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
GetReturnNodeASMRA (node *graph)
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
UpdateReturnASMRA (node *graph, node *arg_node)
{
    node *returnnode;

    DBUG_ENTER ("UpdateReturn");

    returnnode = GetReturnNodeASMRA (graph);

    DATA_ASMRA_INNERASSIGN (returnnode) = arg_node;
    DATA_ASMRA_OUTERASSIGN (returnnode) = arg_node;

    DBUG_RETURN (graph);
}

node *
MakeDataflowNodeASMRA (char *name, node *inner_assign, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("MakeDataflowNode");

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

    /* set status 0 (default) */
    DATA_ASMRA_STATUS (tmp) = 0;

    /* initial value for the reference-counter is 0
       the reference of the spring is implicit*/
    DATA_ASMRA_NODEREFCOUNT (tmp) = 0;

    DBUG_RETURN (tmp);
}

node *
AddDataflowNodeASMRA (node *graph, node *newnode)
{
    node *return_node;
    DBUG_ENTER ("AddDataflowNode");

    /* node added to the graph -> made dependent from the spring */
    DATA_ASMRA_DEPENDENT (graph)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (graph), newnode, NULL);

    return_node = GetReturnNodeASMRA (graph);

    DATA_ASMRA_DEPENDENT (newnode)
      = NodeListAppend (DATA_ASMRA_DEPENDENT (newnode), return_node, NULL);
    DATA_ASMRA_NODEREFCOUNT (return_node) += 1;

    DBUG_RETURN (graph);
}

node *
UpdateDependenciesASMRA (node *graph, node *avisnode, node *actnode)
{
    nodelist *list_iterator;
    node *tmp;
    DBUG_ENTER ("UpdateDependencies");

    DBUG_ASSERT ((NODE_TYPE (avisnode) == N_avis), "node is not a N_avis");

    /*fprintf(stdout,"UpdateDependencies for %s\n",DATA_ASMRA_NAME(actnode));*/

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

            /*fprintf(stdout,"%s ist the father of %s\n",DATA_ASMRA_NAME(tmp),
              DATA_ASMRA_NAME(actnode));*/

            /* let us insert actnode into tmp's dependent_nodes, if this has not be
               done yet */
            if (NodeListFind (DATA_ASMRA_DEPENDENT (tmp), actnode) == NULL) {
                DATA_ASMRA_DEPENDENT (tmp)
                  = NodeListAppend (DATA_ASMRA_DEPENDENT (tmp), actnode, NULL);
                DATA_ASMRA_NODEREFCOUNT (tmp) += 1;
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

/**
 * @}
 **/
