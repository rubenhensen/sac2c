/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:39:42  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"

#include "traverse.h"

#define VAR "__tmp"       /* name of new variable */
#define VAR_LENGTH 10     /* dimension for array of char */
#define P_FORMAT "(%06x)" /* formatstring for pointer address */

extern node *MakeNode (nodetype); /* defined in sac.y or y.tab.c respectively */
extern char *mdb_nodetype[];      /* defined in my_debug.h */

static int var_counter = 0;

/*
 *
 *  functionname  : GenTmpVar
 *  arguments     : 1) counter number
 *  description   : allocate string for temporary variable
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
GenTmpVar (int count)
{
    char *string;

    DBUG_ENTER ("GenTmpVar");

    string = (char *)malloc (sizeof (char) * VAR_LENGTH);
    sprintf (string, VAR "%d", count);

    DBUG_PRINT ("TMP", ("new variable: %s", string));

    DBUG_RETURN (string);
}

/*
 *
 *  functionname  : Flatten
 *  arguments     : 1) syntax tree
 *  description   : eliminates nested function applications
 *  global vars   : syntax_tree, act_tab, flat_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
Flatten (node *arg_node)
{
    node *info_node;

    DBUG_ENTER ("Flatten");

    var_counter = 0;
    act_tab = flat_tab;
    info_node = MakeNode (N_info);
    info_node->nnode = 1;
    info_node->node[0] = NULL;
    arg_node = Trav (arg_node, info_node);
    free (info_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnAssign
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : Insert reference to actual assign node into info node
 *                  call recursively and return adress from the info node
 *                  obtained back.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
FltnAssign (node *arg_node, node *arg_info)
{
    node *return_node;

    DBUG_ENTER ("FltnAssign");

    arg_info->node[0] = arg_node;
    DBUG_PRINT ("FLATTEN", ("arg_info->node[0] set to %08x!", arg_node));
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    return_node = arg_info->node[0];
    DBUG_PRINT ("FLATTEN", ("node %08x inserted before %08x", return_node, arg_node));
    if (arg_node->nnode == 2) /* there are more assigns that follow */
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : FltnPrf
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   : var_counter
 *  internal funs : GenTmpVar
 *  external funs : MakeNode
 *  macros        : GEN_NODE
 *
 *  remarks       :
 *
 */

node *
FltnPrf (node *arg_node, node *arg_info)
{
    int i;
    node *tmp_node1, *id_node, *let_node, *assign_node;

    DBUG_ENTER ("FltnPrf");

    for (i = 0; i < 2; i++)
        if ((arg_node->node[i]->nodetype == N_ap)
            || (arg_node->node[i]->nodetype == N_prf)) {

            /* This argument is a function application and thus has to be abstracted
            ** out. Therefore a new N_assign, a new N_let, and a new temporary
            ** variable are generated and inserted.
            */
            tmp_node1 = arg_node->node[i];

            id_node = MakeNode (N_id);
            id_node->info.id = GenTmpVar (var_counter);
            arg_node->node[i] = id_node;

            let_node = MakeNode (N_let);
            let_node->info.ids = GEN_NODE (ids);
            let_node->info.ids->id = GenTmpVar (var_counter++);
            let_node->info.ids->next = NULL;

            assign_node = MakeNode (N_assign);
            assign_node->node[0] = let_node;
            assign_node->node[1]
              = arg_info->node[0]; /* here the new node is put in front! */
            if (NULL == assign_node->node[1])
                assign_node->nnode = 1;
            else
                assign_node->nnode = 2;

            arg_info->node[0] = assign_node;
            DBUG_PRINT ("FLATTEN", ("new assign-node %08x inserted before node %08x",
                                    assign_node, assign_node->node[1]));

            /* Now, we have to flatten the child "tmp_node1" recursively! */

            let_node->nnode = 1;
            let_node->node[0] = Trav (tmp_node1, arg_info);
        }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnExprs
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   : var_counter
 *  internal funs : GenTmpVar
 *  external funs : MakeNode
 *  macros        : GEN_NODE
 *
 *  remarks       :
 *
 */

node *
FltnExprs (node *arg_node, node *arg_info)
{
    int i;
    node *tmp_node1, *id_node, *let_node, *assign_node;

    DBUG_ENTER ("FltnExprs");

    if ((arg_node->node[0]->nodetype == N_ap) || (arg_node->node[0]->nodetype == N_prf)) {

        /* This argument is a function application and thus has to be abstracted
        ** out. Therefore a new N_assign, a new N_let, and a new temporary
        ** variable are generated and inserted.
        */
        tmp_node1 = arg_node->node[0];

        id_node = MakeNode (N_id);
        id_node->info.id = GenTmpVar (var_counter);
        arg_node->node[0] = id_node;

        let_node = MakeNode (N_let);
        let_node->info.ids = GEN_NODE (ids);
        let_node->info.ids->id = GenTmpVar (var_counter++);
        let_node->info.ids->next = NULL;

        assign_node = MakeNode (N_assign);
        assign_node->node[0] = let_node;
        assign_node->node[1] = arg_info->node[0]; /* here the new node is put in front! */
        if (NULL == assign_node->node[1])
            assign_node->nnode = 1;
        else
            assign_node->nnode = 2;

        arg_info->node[0] = assign_node;

        /* Now, we have to flatten the child "tmp_node1" recursively! */

        let_node->nnode = 1;
        let_node->node[0] = Trav (tmp_node1, arg_info);
    }

    /* Last, but not least remaining exprs have to be done */
    if (arg_node->nnode == 2)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}
