/*
 * $Log$
 * Revision 1.2  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.1  2004/07/29 08:39:12  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup cdfg Create Dataflowgraph
 * @ingroup muth
 *
 * @brief Create dataflowgraphs for the code
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file create_dataflowgraph.c
 *
 * prefix: CDFG
 *
 * description:
 *   creates a dataflowgraph for each function-definition, to use ist later on
 *   in other subphases of mt-mode 3 like rearranging the code or inserting
 *   a communication-structure
 *   Attention: Tghis phase expects only one assignment per ex-/st-/mt-cell!
 *              This assignment must be tagged with the correct executionmode!
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "create_dataflowgraph.h"
#include "multithread.h"
#include "print.h"

/*
 * INFO structure
 */
struct INFO {
    node *dataflowgraph;
    node *outerassignment;
    node *actualnode;
    int executionmode;
    int withdeep;
};

/*
 * INFO macros
 *   node*      DATAFLOWGRAPH  (the dataflowgraph of the currunt function
 *                              as far as it has been constructed)
 *   int        EXECUTIONMODE  (the current execution mode)
 */
#define INFO_CDFG_DATAFLOWGRAPH(n) (n->dataflowgraph)
#define INFO_CDFG_OUTERASSIGN(n) (n->outerassignment)
#define INFO_CDFG_ACTNODE(n) (n->actualnode)
#define INFO_CDFG_EXECUTIONMODE(n) (n->executionmode)
#define INFO_CDFG_WITHDEEP(n) (n->withdeep)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CDFG_DATAFLOWGRAPH (result) = NULL;
    INFO_CDFG_OUTERASSIGN (result) = NULL;
    INFO_CDFG_ACTNODE (result) = NULL;
    INFO_CDFG_EXECUTIONMODE (result) = MUTH_ANY;
    INFO_CDFG_WITHDEEP (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CreateDataflowgraph(node *arg_node)
 *
 * description:
 *   Inits the traversal for this phase
 *
 ******************************************************************************/
node *
CreateDataflowgraph (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CreateDataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CreateDataflowgraph expects a N_modul as arg_node");

    arg_info = MakeInfo ();

    /* push info ... */
    old_tab = act_tab;
    act_tab = cdfg_tab;

    DBUG_PRINT ("CDFG", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 ******************************************************************************/
node *
CDFGfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGfundef");

    DBUG_PRINT ("CDFG", ("Welcome to ASMRAfundef"));
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "node is not a N_fundef");

    INFO_CDFG_WITHDEEP (arg_info) = 0;

    /* Initialisation of the Dataflowgraph for this N_fundef */
    INFO_CDFG_EXECUTIONMODE (arg_info) = MUTH_ANY;
    INFO_CDFG_DATAFLOWGRAPH (arg_info)
      = InitiateDataflowgraph (FUNDEF_NAME (arg_node), arg_info);

    /* continue traversal */
    DBUG_PRINT ("CDFG", ("trav into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from body"));

    /* now the dataflowgraph is ready to be used for rearranging the assignment*/
#if CDFG_DEBUG
    PrintDataflowgraph (INFO_CDFG_DATAFLOWGRAPH (arg_info), FUNDEF_NAME (arg_node));
#endif

    FUNDEF_DATAFLOWGRAPH (arg_node) = INFO_CDFG_DATAFLOWGRAPH (arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CDFG", ("trav into fundef-next"));
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from fundef-next"));
    }

    DBUG_RETURN (arg_node);
}

node *
CDFGassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "node is not a N_assign");

    /* there's a problem concerning finding the correct assign belonging to a
       N_let (the new nodes of the dataflowgraph are build in N_let-nodes):
       the current structure is
        N_block
         |
        N_assign(a)
         |
        N_assign(b1) -> N_mt -> N_block -> N_assign(b2) -> instr
         |
        N_assign(c1) -> N_st -> N_block -> N_assign(c2) -> instr
        ...
       the corresponding N_avis-nodes point to (a),(b2) and (c2), but if you want
       to rearrange the assignment-chain, you need pointer to (a),(b1) and (c1);
       so you habe to keep the assign in your mind if the current executionmode
       is MUTH_ANY (if the current executionmode != MUTH_ANY, you are already in
       an ex-, st- or mt-cell) */

    INFO_CDFG_EXECUTIONMODE (arg_info) = ASSIGN_EXECMODE (arg_node);

    if (ASSIGN_EXECMODE (arg_node) == MUTH_ANY) {
        INFO_CDFG_OUTERASSIGN (arg_info) = arg_node;
    }

    /* if it's the return-instruction, one need not to traverse into the
       instruction, because the dataflownode for return already exists
    */
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return) {
        DBUG_PRINT ("CDFG", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from instruction"));
    } else {
        /* but you have to update the returnassignment in the dataflowgraph*/
        INFO_CDFG_DATAFLOWGRAPH (arg_info)
          = UpdateReturn (INFO_CDFG_DATAFLOWGRAPH (arg_info), arg_node);
    }

    /* continue traversal */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CDFG", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from next"));
    }

    DBUG_RETURN (arg_node);
}

node *
CDFGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGlet");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), "node is not a N_let");

    if (INFO_CDFG_WITHDEEP (arg_info) == 0) {
        /* create dataflownode out of the let-ids */
        DBUG_PRINT ("CDFG", ("before MakeDataflowNode"));
        INFO_CDFG_ACTNODE (arg_info)
          = MakeDataflowNode (IDS_NAME (LET_IDS (arg_node)),
                              AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (arg_node))), arg_info);
        DBUG_PRINT ("CDFG", ("after MakeDataflowNode"));

        /* insert the node into the dataflowgraph */
        INFO_CDFG_DATAFLOWGRAPH (arg_info)
          = AddDataflowNode (INFO_CDFG_DATAFLOWGRAPH (arg_info),
                             INFO_CDFG_ACTNODE (arg_info));
    }
    /* continue traversal */
    DBUG_PRINT ("CDFG", ("trav into expr"));
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from expr"));

    DBUG_RETURN (arg_node);
}

node *
CDFGid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

    /*fprintf(stdout,"act. id = %s\n",ID_NAME(arg_node));*/
    INFO_CDFG_DATAFLOWGRAPH (arg_info)
      = UpdateDependencies (INFO_CDFG_DATAFLOWGRAPH (arg_info), ID_AVIS (arg_node),
                            INFO_CDFG_ACTNODE (arg_info));

    DBUG_RETURN (arg_node);
}

node *
CDFGwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGNwith2");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith2), "node is not a N_Nwith2");

    /* increase the "deepness of the withloop"-counter */
    INFO_CDFG_WITHDEEP (arg_info)++;

    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /* restore actual deepness */
    INFO_CDFG_WITHDEEP (arg_info)--;

    DBUG_RETURN (arg_node);
}

node *
InitiateDataflowgraph (char *name, info *arg_info)
{
    node *graph;
    DBUG_ENTER ("CreateDataflowgraph");

    /* build initial datanode */
    graph = MakeDataflowNode (name, NULL, arg_info);

    /* add the sink to the dataflowgraph */
    INFO_MUTH_DFGDEPENDENT (graph)
      = NodeListAppend (INFO_MUTH_DFGDEPENDENT (graph),
                        MakeDataflowNode (NULL, NULL, arg_info), NULL);

    /* return datanode as new graph */
    DBUG_RETURN (graph);
}

int
PrintDataflowgraph (node *dataflowgraph, char *name)
{
    int status;
    nodelist *list_iterator;
    DBUG_ENTER ("PrintDataflowgraph");
    status = 0;

    fprintf (stdout, "The Dataflowgraph for %s:\n", name);

    list_iterator = INFO_MUTH_DFGDEPENDENT (dataflowgraph);
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

    if (INFO_MUTH_DFGNAME (datanode) != NULL) {
        fprintf (stdout, "- Name: %s, mode: %i\n", INFO_MUTH_DFGNAME (datanode),
                 INFO_MUTH_DFGEXECUTIONMODE (datanode));
    } else {
        fprintf (stdout, "- Return, mode: %i\n", INFO_MUTH_DFGEXECUTIONMODE (datanode));
    }

    list_iterator = INFO_MUTH_DFGDEPENDENT (datanode);

    if (list_iterator != NULL) {
        fprintf (stdout, "  ->");

        while (list_iterator != NULL) {
            if (NODE_TYPE (
                  ASSIGN_INSTR (INFO_MUTH_DFGINNERASSIGN (NODELIST_NODE (list_iterator))))
                != N_return) {
                fprintf (stdout, " %s,",
                         INFO_MUTH_DFGNAME (NODELIST_NODE (list_iterator)));
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
GetReturnNode (node *graph)
{
    nodelist *list_iterator;

    DBUG_ENTER ("GetReturnNode");

    list_iterator = INFO_MUTH_DFGDEPENDENT (graph);

    while ((list_iterator != NULL)
           && (INFO_MUTH_DFGINNERASSIGN (NODELIST_NODE (list_iterator)) != NULL)) {
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

    INFO_MUTH_DFGINNERASSIGN (returnnode) = arg_node;
    INFO_MUTH_DFGOUTERASSIGN (returnnode) = arg_node;

    DBUG_RETURN (graph);
}

node *
MakeDataflowNode (char *name, node *inner_assign, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("MakeDataflowNode");

    /*tmp = MakeInfo();*/

    INFO_MUTH_DFGNAME (tmp) = name;
    /* the inner assignment means, that its instruction is the N_let of the
       identifier */
    INFO_MUTH_DFGINNERASSIGN (tmp) = inner_assign;

    /* it is impossible to have a outer assign without an inner assign */
    if (inner_assign == NULL) {
        INFO_MUTH_DFGOUTERASSIGN (tmp) = NULL;
    } else {
        INFO_MUTH_DFGOUTERASSIGN (tmp) = INFO_CDFG_OUTERASSIGN (arg_info);
    }

    INFO_MUTH_DFGEXECUTIONMODE (tmp) = INFO_CDFG_EXECUTIONMODE (arg_info);
    INFO_MUTH_DFGDEPENDENT (tmp) = NULL;

    /* initial value for the reference-counter is 0
       the reference of the spring is implicit*/
    INFO_MUTH_DFGNODEREFCOUNT (tmp) = 0;

    DBUG_RETURN (tmp);
}

node *
AddDataflowNode (node *graph, node *newnode)
{
    node *return_node;
    DBUG_ENTER ("AddDataflowNode");

    /* node added to the graph -> made dependent from the spring */
    INFO_MUTH_DFGDEPENDENT (graph)
      = NodeListAppend (INFO_MUTH_DFGDEPENDENT (graph), newnode, NULL);

    return_node = GetReturnNode (graph);

    INFO_MUTH_DFGDEPENDENT (newnode)
      = NodeListAppend (INFO_MUTH_DFGDEPENDENT (newnode), return_node, NULL);
    INFO_MUTH_DFGNODEREFCOUNT (return_node) += 1;

    DBUG_RETURN (graph);
}

node *
UpdateDependencies (node *graph, node *avisnode, node *actnode)
{
    nodelist *list_iterator;
    node *tmp;
    DBUG_ENTER ("UpdateDependencies");

    DBUG_ASSERT ((NODE_TYPE (avisnode) == N_avis), "node is not a N_avis");

    /*fprintf(stdout,"UpdateDependencies for %s\n",INFO_MUTH_DFGNAME(actnode));*/

    /* is it a with_id?*/
    if (AVIS_WITHID (avisnode) == NULL) {
        /* no - let us have a closer look on it */

        /* Does the variable depend on an assignment ? */
        if (AVIS_SSAASSIGN (avisnode) != NULL) {
            list_iterator = INFO_MUTH_DFGDEPENDENT (graph);

            /* time to search for the corresponding node */
            while (INFO_MUTH_DFGINNERASSIGN (NODELIST_NODE (list_iterator))
                   != AVIS_SSAASSIGN (avisnode)) {
                list_iterator = NODELIST_NEXT (list_iterator);
                DBUG_ASSERT ((list_iterator != NULL),
                             "Variable without any correspondent lefthandside");
            }

            /* tmp points on the dataflownode, that has to be a father of actnode */
            tmp = NODELIST_NODE (list_iterator);

            /*fprintf(stdout,"%s ist the father of %s\n",INFO_MUTH_DFGNAME(tmp),
              INFO_MUTH_DFGNAME(actnode));*/

            /* let us insert actnode into tmp's dependent_nodes, if this has not be
               done yet */
            if (NodeListFind (INFO_MUTH_DFGDEPENDENT (tmp), actnode) == NULL) {
                INFO_MUTH_DFGDEPENDENT (tmp)
                  = NodeListAppend (INFO_MUTH_DFGDEPENDENT (tmp), actnode, NULL);
                INFO_MUTH_DFGNODEREFCOUNT (tmp) += 1;
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
