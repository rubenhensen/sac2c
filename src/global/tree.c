/*
 *
 * $Log$
 * Revision 1.3  1994/12/30 16:57:48  sbs
 * added MakeTypes
 *
 * Revision 1.2  1994/12/20  17:42:51  hw
 * added includes stdlib.h & dbug.h
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>

#include "tree.h"
#include "dbug.h"
#include "my_debug.h"
#include "scnprs.h"

/*
 *
 *  functionname  : MakeTypes
 *  arguments     :
 *  description   : generates and initialises a new types;
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GEN_NODE
 *
 *  remarks       :
 *
 */

types *
MakeTypes (simpletype simple)
{
    types *tmp;

    DBUG_ENTER ("MakeTypes");

    tmp = GEN_NODE (types);
    tmp->simpletype = simple;
    tmp->name = NULL;
    tmp->name_mod = NULL;
    tmp->dim = 0;
    tmp->next = NULL;
    tmp->id = NULL;
    tmp->id_mod = NULL;

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : MakeNode
 *  arguments     : nodetype: type of node to be generated
 *  description   : generates and initialises a new node;
 *                  the only field not involved is node->info
 *  global vars   : linenum
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GEN_NODE
 *
 *  remarks       :
 *
 */

node *
MakeNode (nodetype nodetype)
{
    node *tmp;
    int i;

    DBUG_ENTER ("MakeNode");

    tmp = GEN_NODE (node);
    tmp->nodetype = nodetype;
    for (i = 0; i < 4; i++)
        tmp->node[i] = NULL;
    tmp->nnode = 0;
    tmp->info.id = NULL;
    tmp->lineno = linenum;

    DBUG_PRINT ("MAKENODE",
                ("%d nodetype: %s " P_FORMAT, tmp->lineno, mdb_nodetype[nodetype], tmp));

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : AppendNodeChain
 *  arguments     : int pos: node-index of chain
 *                  node *first: first node chain
 *                  node *second: node chain to be appended
 *  description   : follows first chain to it's end and
 *                  appends second.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
AppendNodeChain (int pos, node *first, node *second)

{
    node *tmp;

    DBUG_ENTER ("AppendNodeChain");

    if (first == NULL)
        first = second;
    else {
        tmp = first;
        while (tmp->node[pos] != NULL)
            tmp = tmp->node[pos];
        tmp->node[pos] = second;
        if (second != NULL)
            tmp->nnode++;
    }

    DBUG_RETURN (first);
}
