/*
 *
 * $Log$
 * Revision 1.18  1995/07/06 17:28:34  cg
 * MakeIds and MakeTypes adjusted for new statustype
 *
 * Revision 1.17  1995/06/01  15:08:38  cg
 * Bug in MakeNode fixed.
 *
 * Revision 1.16  1995/06/01  10:10:54  cg
 * status in MakeTypes initialized.
 *
 * Revision 1.15  1995/04/24  15:13:46  asi
 * added AppendIdsChain
 *
 * Revision 1.14  1995/04/11  11:34:45  asi
 * added 'flag' to struct 'node'
 *
 * Revision 1.13  1995/03/15  16:59:01  asi
 * Bug fixed : initialization of mask in MakeNode
 *
 * Revision 1.12  1995/03/14  14:11:24  asi
 * added initialization of 'bblock' in MakeNode
 *
 * Revision 1.11  1995/03/13  16:04:19  asi
 * changed MakeIds
 *
 * Revision 1.10  1995/03/13  15:47:32  hw
 * MakeIds inserted
 *
 * Revision 1.9  1995/03/13  15:13:36  asi
 * added initialization of 'varno' in MakeNode
 *
 * Revision 1.8  1995/03/08  10:39:40  hw
 * - added initialization of refcnt in MakeNode
 *
 * Revision 1.7  1995/02/22  10:48:08  hw
 * bug fixed in MakeTypes (shpseg is set to NULL)
 *
 * Revision 1.6  1995/01/18  17:39:17  asi
 * MAX_MASK inserted
 *
 * Revision 1.5  1995/01/16  11:10:25  asi
 * masks initial set to NULL in MakeNode
 *
 * Revision 1.4  1994/12/31  14:09:37  sbs
 * DBUG_PRINT in MakeTypes inserted
 *
 * Revision 1.3  1994/12/30  16:57:48  sbs
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
#include "optimize.h"

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
    tmp->shpseg = NULL;
    tmp->name = NULL;
    tmp->name_mod = NULL;
    tmp->dim = 0;
    tmp->next = NULL;
    tmp->id = NULL;
    tmp->id_mod = NULL;
    tmp->attrib = ST_regular;
    tmp->status = ST_regular;

    DBUG_PRINT ("MAKETYPES", (P_FORMAT, tmp));

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
 *  macros        : DBUG..., GEN_NODE, MAX_MASK
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
    for (i = 0; i < MAX_SONS; i++)
        tmp->node[i] = NULL;
    tmp->nnode = 0;
    tmp->info.id = NULL;
    tmp->bblock = 0;
    tmp->flag = 0;
    tmp->varno = 0;
    tmp->lineno = linenum;
    tmp->refcnt = 0;
    for (i = 0; i < MAX_MASK; i++)
        tmp->mask[i] = NULL;

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

/*
 *
 *  functionname  : AppendIdsChain
 *  arguments     : node *first: first ids chain
 *                  node *second: ids chain to be appended
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

ids *
AppendIdsChain (ids *first, ids *second)

{
    ids *tmp;

    DBUG_ENTER ("AppendIdsChain");

    if (first == NULL)
        first = second;
    else {
        tmp = first;
        while (tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = second;
    }

    DBUG_RETURN (first);
}

/*
 *
 *  functionname  : MakeIds
 *  arguments     : 1) identifier
 *  description   : generates and initialises a new 'ids' struct;
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GEN_NODE
 *
 *  remarks       :
 *
 */

ids *
MakeIds (char *id)
{
    ids *tmp;

    DBUG_ENTER ("MakeIds");

    tmp = GEN_NODE (ids);
    DBUG_ASSERT ((NULL != tmp), "out of memory");
    tmp->id = id;
    tmp->refcnt = 0;
    tmp->node = NULL;
    tmp->nchain = NULL;
    tmp->attrib = ST_regular;
    tmp->status = ST_regular;
    tmp->next = NULL;

    DBUG_RETURN (tmp);
}
