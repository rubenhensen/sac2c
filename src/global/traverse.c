#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "print.h"
#include "traverse.h"

funptr *act_tab;

/*
**
**  global definitions of all funtabs needed for Trav
**
**  1) flat_tab
**
*/

#define NIF(n, s, x, y, z) x

funptr flat_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
**  2) print_tab
*/

#define NIF(n, s, x, y, z) y

funptr print_tab[] = {
#include "node_info.mac"
};

#undef NIF

/*
**
**  functionname  : Trav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : higher order traverse function; selects particular
**                  function from the globaly available funtab
**                  pointed to by "act_tab" with respect to the
**                  actual node-type.
**  global vars   : act_tab
**  internal funs : ---
**  external funs : all funs from flatten.c
**
**  remarks       : could be done as macro as well!!
**
*/

node *
Trav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("Trav");
    DBUG_ASSERT ((NULL != arg_node), "wrong argument:NULL pointer");
    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x number of nodes: %d",
                         mdb_nodetype[arg_node->nodetype], arg_node, arg_node->nnode));

    DBUG_RETURN ((*act_tab[arg_node->nodetype]) (arg_node, arg_info));
}

/*
**  dummy function for funtab entries not yet done
**  recursively invoces Trav on all subnodes
*/

node *
DummyFun (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("DummyFun");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    DBUG_RETURN (arg_node);
}
