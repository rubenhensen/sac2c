/*
 *
 * $Log$
 * Revision 2.3  1999/09/10 14:21:32  jhs
 * I deleted on simple and plain blank.
 *
 * Revision 2.2  1999/05/07 13:00:22  jhs
 * initialization of src_file in Make Node added.
 *
 * Revision 2.1  1999/02/23 12:39:49  sacbase
 * new release made
 *
 * Revision 1.31  1998/05/06 14:30:37  dkr
 * added support for DataFlowMasks
 *
 * Revision 1.30  1998/03/25 14:33:49  srs
 * removed NEWTREE
 *
 * Revision 1.29  1997/11/13 12:57:30  srs
 * initialized compound info2 of node-type in MakeNode()
 *
 * Revision 1.28  1997/10/30 12:22:10  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.27  1997/05/14 08:16:43  sbs
 * N_annotate added
 *
 * Revision 1.26  1996/01/02  15:50:34  cg
 * function MakeTypes now initializes new struct entries tdef and id_cmod
 * of struct types
 *
 * Revision 1.25  1995/12/20  09:09:48  cg
 * removed some functions which are replaced by new ones in tree_basic.c
 *
 * Revision 1.24  1995/11/01  16:24:16  cg
 * moved function AppendIdsChain to tree_compound.[ch]
 *
 * Revision 1.23  1995/09/27  15:16:54  cg
 * ATTENTION:
 * tree.c and tree.h are not part of the new virtual syntax tree.
 * They are kept for compatibility reasons with old code only !
 * All parts of their old versions which are to be used in the future are moved
 * to tree_basic and tree_compound.
 * DON'T use tree.c and tree.h when writing new code !!
 *
 * Revision 1.22  1995/09/07  09:49:48  sbs
 * first set of Make<N_...> functions/ access macros
 * inserted.
 *
 * Revision 1.21  1995/07/14  13:21:12  asi
 * DBUG_PRINT("APP",... added to AppendNodeChain
 *
 * Revision 1.20  1995/07/10  07:31:59  asi
 * removed bblock from structure node and added def to structure ids
 *
 * Revision 1.19  1995/07/07  16:21:19  hw
 * added 'char *prf_name_str[]'( moved from typecheck.c)
 *
 * Revision 1.18  1995/07/06  17:28:34  cg
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

/*--------------------------------------------------------------------------*/
/* The following functions are supported for compatibility reasons only     */
/*--------------------------------------------------------------------------*/

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
        while (tmp->node[pos])
            tmp = tmp->node[pos];
        tmp->node[pos] = second;
        DBUG_PRINT ("APP", ("Append node chain behind line %d", tmp->lineno));
    }

    DBUG_RETURN (first);
}

/*
 * The function AppendNodeChain should be replaced
 * by a new function in tree_compound.c
 * Unfortunately, this is not directly possible. Different functions are
 * needed for objdef-, fundef-, or typedef-chains.
 */

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
    tmp->id_cmod = NULL;
    tmp->attrib = ST_regular;
    tmp->status = ST_regular;
    tmp->tdef = NULL;

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
    tmp->info.id = NULL;
    tmp->flag = 0;
    tmp->varno = 0;
    tmp->lineno = linenum;
    tmp->src_file = filename;
    tmp->refcnt = 0;
    tmp->counter = 0;
    tmp->info2 = NULL;
    for (i = 0; i < MAX_MASK; i++) {
        tmp->mask[i] = NULL;
        tmp->dfmask[i] = NULL;
    }

    DBUG_PRINT ("MAKENODE",
                ("%d nodetype: %s " P_FORMAT, tmp->lineno, mdb_nodetype[nodetype], tmp));

    DBUG_RETURN (tmp);
}
