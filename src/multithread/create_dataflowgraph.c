/*
 * $Log$
 * Revision 1.3  2004/08/06 17:24:38  skt
 * some adaptions made
 *
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
    node *dataflownode;
    node *dataflowgraph_cond;
    node *dataflownode_cond;
    int withdeep;
    bool condflag;
};

/*
 * INFO macros
 *   node*      DATAFLOWGRAPH  (the dataflowgraph of the currunt function
 *                              as far as it has been constructed)
 *   node*      CURRENTDFNODE  (the the dataflownode belonging to the current
 *                              assignment)
 *   int        WITHDEEP       (counts the deepness of a withloop)
 *
 *    There's a problem concerning N_conds:
 *      N_block
 *       |
 *      N_assign - N_cond - N_block (then) HERE
 *       |                \ N_block (else) HERE
 *      ...
 *    => you have to handle two different dataflowgraphs, when you are HERE
 *       that's the reason for
 *   bool       WITHINCOND,
 *   node*      DATAFLOWGRAPH_COND and
 *   node*      CURRENTDFNODE_COND.
 */
#define INFO_CDFG_DATAFLOWGRAPH(n) (n->dataflowgraph)
#define INFO_CDFG_CURRENTDFNODE(n) (n->dataflownode)
#define INFO_CDFG_WITHDEEP(n) (n->withdeep)
#define INFO_CDFG_WITHINCOND(n) (n->condflag)
#define INFO_CDFG_DATAFLOWGRAPH_COND(n) (n->dataflowgraph_cond)
#define INFO_CDFG_CURRENTDFNODE_COND(n) (n->dataflownode_cond)

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
    INFO_CDFG_CURRENTDFNODE (result) = NULL;
    INFO_CDFG_WITHDEEP (result) = 0;
    INFO_CDFG_WITHINCOND (result) = FALSE;
    INFO_CDFG_DATAFLOWGRAPH_COND (result) = NULL;
    INFO_CDFG_CURRENTDFNODE_COND (result) = NULL;

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
 * @fn node *CDFGblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return the block-node with its belonging dataflowgraph
 *
 *****************************************************************************/
node *
CDFGblock (node *arg_node, info *arg_info)
{
    node *old_dataflowgraph;
    node *old_dataflowgraph_cond;
    DBUG_ENTER ("CDFGblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

    /* push info... */
    old_dataflowgraph = INFO_CDFG_DATAFLOWGRAPH (arg_info);
    old_dataflowgraph_cond = INFO_CDFG_DATAFLOWGRAPH_COND (arg_info);

    /* avoid to build dataflowgraphs within with-loops */
    if (INFO_CDFG_WITHDEEP (arg_info) == 0) {
        /* Initialisation of the dataflowgraph for this N_block */
        BLOCK_DATAFLOWGRAPH (arg_node) = MakeDataflowgraph ();

        /* last question - is it a cond-block (then/else) or not */
        if (INFO_CDFG_WITHINCOND (arg_info) == FALSE) {
            DBUG_ASSERT ((INFO_CDFG_DATAFLOWGRAPH (arg_info) == NULL),
                         "is impossible to have a dataflowgraph yet");
            DBUG_ASSERT ((INFO_CDFG_CURRENTDFNODE (arg_info) == NULL),
                         "is impossible to have a dataflownode yet");
            INFO_CDFG_DATAFLOWGRAPH (arg_info) = BLOCK_DATAFLOWGRAPH (arg_node);
        } else {
            DBUG_ASSERT ((INFO_CDFG_DATAFLOWGRAPH_COND (arg_info) == NULL),
                         "is impossible to have a cond-dataflowgraph yet");
            DBUG_ASSERT ((INFO_CDFG_CURRENTDFNODE_COND (arg_info) == NULL),
                         "is impossible to have a cond-dataflownode yet");
            INFO_CDFG_DATAFLOWGRAPH_COND (arg_info) = BLOCK_DATAFLOWGRAPH (arg_node);
        }
    }

    /* continue traversal */
    DBUG_PRINT ("CDFG", ("trav into instruction(s)"));
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from instruction(s)"));

    if (INFO_CDFG_WITHINCOND (arg_info) == FALSE) {
        PrintDataflowgraph (INFO_CDFG_DATAFLOWGRAPH (arg_info));
    } else {
        PrintDataflowgraph (INFO_CDFG_DATAFLOWGRAPH_COND (arg_info));
    }

    /* pop info... */
    INFO_CDFG_DATAFLOWGRAPH (arg_info) = old_dataflowgraph;
    INFO_CDFG_DATAFLOWGRAPH_COND (arg_info) = old_dataflowgraph_cond;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGassign (node *arg_node, info *arg_info)
{
    node *old_dataflownode;
    node *old_dataflownode_cond;

    DBUG_ENTER ("CDFGassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "node is not a N_assign");

    /* push info... */
    old_dataflownode = INFO_CDFG_CURRENTDFNODE (arg_info);
    old_dataflownode_cond = INFO_CDFG_CURRENTDFNODE_COND (arg_info);

    /* only assignments on the top-level will be represented in the dataflowgraph
     */
    if (INFO_CDFG_WITHDEEP (arg_info) == 0
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)) {
        /* are we within a conditional? No - handle the "outer" dataflowgraph
         *                              Yes - handle the "inner" cond-dataflowgraph
         */
        if (INFO_CDFG_WITHINCOND (arg_info) == FALSE) {
            INFO_CDFG_CURRENTDFNODE (arg_info)
              = MakeDataflownode (arg_node, StringCopy ("DF__test"));
            INFO_CDFG_DATAFLOWGRAPH (arg_info)
              = AddDataflownode (INFO_CDFG_DATAFLOWGRAPH (arg_info),
                                 INFO_CDFG_CURRENTDFNODE (arg_info));
        } else {
            INFO_CDFG_CURRENTDFNODE_COND (arg_info)
              = MakeDataflownode (arg_node, StringCopy ("DF__test"));
            INFO_CDFG_DATAFLOWGRAPH_COND (arg_info)
              = AddDataflownode (INFO_CDFG_DATAFLOWGRAPH_COND (arg_info),
                                 INFO_CDFG_CURRENTDFNODE_COND (arg_info));
        }
    }
    /* if it's the return-instruction, one need not to traverse into the
       instruction, because the dataflownode for return already exists
       -> the sink
    */
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return) {
        DBUG_PRINT ("CDFG", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from instruction"));

        /* continue traversal */
        if (ASSIGN_NEXT (arg_node) != NULL) {
            DBUG_PRINT ("CDFG", ("trav into next"));
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
            DBUG_PRINT ("CDFG", ("trav from next"));
        }
    } else {
        /* but you have to update the sink in the dataflowgraph */
        DATAFLOWNODE_ASSIGN (DATAFLOWGRAPH_SINK (INFO_CDFG_DATAFLOWGRAPH (arg_info)))
          = arg_node;
        DBUG_ASSERT ((ASSIGN_NEXT (arg_node) == NULL),
                     "it's forbitten to have an assignment after the N_return");
    }

    /* pop info ... */
    INFO_CDFG_CURRENTDFNODE (arg_info) = old_dataflownode;
    INFO_CDFG_CURRENTDFNODE_COND (arg_info) = old_dataflownode_cond;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGcond(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGCond");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_cond), "CDFGcond expects a N_cond");
    DBUG_ASSERT ((INFO_CDFG_WITHINCOND (arg_info) == FALSE),
                 "nesting conditionals is forbitten");

    DBUG_PRINT ("CDFG", ("trav into cond-condition"));
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from cond-condition"));

    INFO_CDFG_WITHINCOND (arg_info) = TRUE;

    if (COND_THEN (arg_node) != NULL) {
        DBUG_PRINT ("CDFG", ("trav into then-branch"));
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from then-branch"));
    }
    if (COND_ELSE (arg_node) != NULL) {
        DBUG_PRINT ("CDFG", ("trav into else-branch"));
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from else-branch"));
    }

    INFO_CDFG_WITHINCOND (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

#if CDFG_DEBUG
    fprintf (stdout, "act. id = %s\n", ID_NAME (arg_node));
#endif

    INFO_CDFG_DATAFLOWGRAPH (arg_info)
      = UpdateDependencies (INFO_CDFG_DATAFLOWGRAPH (arg_info), ID_AVIS (arg_node),
                            INFO_CDFG_CURRENTDFNODE (arg_info));
    if (INFO_CDFG_WITHINCOND (arg_info) == TRUE) {
        UpdateDependencies (INFO_CDFG_DATAFLOWGRAPH_COND (arg_info), ID_AVIS (arg_node),
                            INFO_CDFG_CURRENTDFNODE_COND (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGwith2(node *arg_node, info *arg_info)
 *
 * @brief increases & drecreases the withdeep counter to mark wheter the
 *        traversal is within a withloop or not
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
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

void
PrintDataflowgraph (node *dataflowgraph)
{
    nodelist *member_iterator;
    DBUG_ENTER ("PrintDataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (dataflowgraph) == N_dataflowgraph),
                 "PrintDataflowgraph expects a N_dataflowgraph");

    member_iterator = DATAFLOWGRAPH_MEMBERS (dataflowgraph);
    while (member_iterator != NULL) {
        PrintDataflownode (NODELIST_NODE (member_iterator));
        member_iterator = NODELIST_NEXT (member_iterator);
    }
    fprintf (stdout, "\n");
    DBUG_VOID_RETURN;
}

void
PrintDataflownode (node *datanode)
{
    nodelist *dependent_iterator;
    DBUG_ENTER ("PrintDataflownode");

    DBUG_ASSERT ((NODE_TYPE (datanode) == N_dataflownode),
                 "PrintDataflownode expects a N_dataflownode");
    if (DATAFLOWNODE_ASSIGN (datanode) != NULL) {
        PrintNode (DATAFLOWNODE_ASSIGN (datanode));
    } else {
        fprintf (stdout, "NULL found!\n");
    }

    fprintf (stdout, "DFN_Name: %s, mode: %s\n", DATAFLOWNODE_NAME (datanode),
             MUTHDecodeExecmode (DATAFLOWNODE_EXECMODE (datanode)));

    dependent_iterator = DATAFLOWNODE_DEPENDENT (datanode);

    if (dependent_iterator != NULL) {
        fprintf (stdout, "  ->");

        while (dependent_iterator != NULL) {
            fprintf (stdout, " %s,",
                     DATAFLOWNODE_NAME (NODELIST_NODE (dependent_iterator)));
            dependent_iterator = NODELIST_NEXT (dependent_iterator);
        }
        fprintf (stdout, "\n");
    } else {
        fprintf (stdout, "  -> No dependent nodes\n");
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************->>
 *
 * @fn node *AddDataflownode(node *graph, node *newnode)
 *
 * @brief adds the dataflownode newnode to the dataflowgraph graph and updates
 *        the dependencies source->newnode and newnode->sink
 *        Attention! all other dependencies are made during the normal
 *                   traversal mechanism
 *
 * @param graph
 * @param newnode
 * @return graph, including newnode and all dependencies
 *
 *****************************************************************************/
node *
AddDataflownode (node *graph, node *newnode)
{
    DBUG_ENTER ("AddDataflownode");
    DBUG_ASSERT ((NODE_TYPE (graph) == N_dataflowgraph),
                 "AddDataflownode expects a N_dataflowgraph as 1st parameter");
    DBUG_ASSERT ((NODE_TYPE (newnode) == N_dataflownode),
                 "AddDataflownode expects a N_dataflownode as 2nd parameter");

    /* add node to the graph members*/
    DATAFLOWGRAPH_MEMBERS (graph)
      = NodeListAppend (DATAFLOWGRAPH_MEMBERS (graph), newnode, NULL);

    /* node added to the graph -> made dependent from the spring */
    DATAFLOWNODE_DEPENDENT (DATAFLOWGRAPH_SOURCE (graph))
      = NodeListAppend (DATAFLOWNODE_DEPENDENT (DATAFLOWGRAPH_SOURCE (graph)), newnode,
                        NULL);
    DATAFLOWNODE_REFCOUNT (newnode) = 1;

    /* well, the return-node must depend on the new-member*/
    DATAFLOWNODE_DEPENDENT (newnode) = NodeListAppend (DATAFLOWNODE_DEPENDENT (newnode),
                                                       DATAFLOWGRAPH_SINK (graph), NULL);
    DATAFLOWNODE_REFCOUNT (DATAFLOWGRAPH_SINK (graph))++;

    DBUG_RETURN (graph);
}

node *
UpdateDependencies (node *graph, node *avisnode, node *actnode)
{
    bool found;
    nodelist *member_iterator;
    node *father;
    DBUG_ENTER ("UpdateDependencies");
    DBUG_ASSERT ((NODE_TYPE (avisnode) == N_avis), "node is not a N_avis");

    found = FALSE;

    /* Does the variable depend on an assignment ? */
    if (AVIS_SSAASSIGN (avisnode) != NULL) {
        member_iterator = DATAFLOWGRAPH_MEMBERS (graph);

        /* time to search for the corresponding node */
        while (found == FALSE && member_iterator != NULL) {
            if ((DATAFLOWNODE_ASSIGN (NODELIST_NODE (member_iterator)))
                == AVIS_SSAASSIGN (avisnode)) {
                found = TRUE;
            }
            member_iterator = NODELIST_NEXT (member_iterator);
        }

        if (found == TRUE) {
            /* father points on the dataflownode, which is a father of actnode */
            father = NODELIST_NODE (member_iterator);

            /* let us insert actnode into father's dependent_nodes,
             * if this has not be done yet */
            if (NodeListFind (DATAFLOWNODE_DEPENDENT (father), actnode) == NULL) {
                DATAFLOWNODE_DEPENDENT (father)
                  = NodeListAppend (DATAFLOWNODE_DEPENDENT (father), actnode, NULL);
                DATAFLOWNODE_REFCOUNT (actnode)++;
            } else {
                /* actnode already depends on the father -> nothing to do */
            }
        } else {
            /* we can ignore it, cause it's assignment is not part of the
             * dataflowgraph */
        }
    } else { /* (AVIS_SSAASSIGN(avisnode) != NULL) */
             /* nothing to do - by default it depends on the spring of the graph*/
    }
    DBUG_RETURN (graph);
}

char *
SetName (node *assign)
{
    node *instr;
    char *return_value;
    DBUG_ENTER ("SetName");
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "SetName expects a N_assign");

    instr = ASSIGN_INSTR (assign);
    return_value = NULL;
    if (NODE_TYPE (instr) == N_let) {
        return_value
          = StringCopy ("DF__condi tion"); /*IDS_NAME(LET_IDS(ASSIGN_INSTR(assign))));*/
    } else if (NODE_TYPE (instr) == N_cond) {
        return_value = StringCopy ("DF__condition");
    } else if (NODE_TYPE (instr) == N_Nwith2) {
        return_value = StringCopy ("DF__with-loop");
    } else {
        PrintNode (assign);
        DBUG_ASSERT (0, "SetName was called with an invalid assignment");
    }
    DBUG_RETURN (return_value);
}

/**
 * @}
 **/
