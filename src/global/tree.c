/*
 *
 * $Log$
 * Revision 1.22  1995/09/07 09:49:48  sbs
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

#define PRF_IF(n, s, x, y) y

char *prf_name_str[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

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
        DBUG_PRINT ("APP", ("Append node chain behind line %d", tmp->lineno));
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
    tmp->attrib = ST_regular;
    tmp->status = ST_regular;
    tmp->next = NULL;
    tmp->def = NULL;

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : MakeNum
 *  arguments     : 1) int val
 *  description   : generates and initialises an N_num node
 *  global vars   :
 *  internal funs : MakeNode
 *  external funs :
 *  macros        : DBUG..., NUM_VAL
 *
 *  remarks       :
 *
 */

node *
MakeNum (int val)
{
    node *num;

    DBUG_ENTER ("MakeNum");

    num = MakeNode (N_num);
    NUM_VAL (num) = val;

    DBUG_RETURN (num);
}

/*
 *
 *  functionname  : MakeExprs
 *  arguments     : 1) node *expr
 *                  2) node *next
 *  description   : generates and initialises an N_exprs node
 *  global vars   :
 *  internal funs : MakeNode
 *  external funs :
 *  macros        : DBUG..., EXPRS_EXPR, EXPRS_NEXT
 *
 *  remarks       :
 *
 */

node *
MakeExprs (node *expr, node *next)
{
    node *exprs;

    DBUG_ENTER ("MakeExprs");

    exprs = MakeNode (N_exprs);
    EXPRS_EXPR (exprs) = expr;
    EXPRS_NEXT (exprs) = NULL;
    exprs->nnode = 1;

    DBUG_RETURN (exprs);
}

/*
 *
 *  functionname  : MakeArray
 *  arguments     : 1) node *aelems
 *  description   : generates and initialises an N_array node
 *  global vars   :
 *  internal funs : MakeNode
 *  external funs :
 *  macros        : DBUG..., EXPRS_EXPR, EXPRS_NEXT
 *
 *  remarks       :
 *
 */

node *
MakeArray (node *aelems)
{
    node *array;

    DBUG_ENTER ("MakeArray");

    array = MakeNode (N_array);
    ARRAY_AELEMS (array) = aelems;
    array->nnode = 1;

    DBUG_RETURN (array);
}

/*
 *
 *  functionname  : MakeVinfo
 *  arguments     : 1) node *aelems
 *  description   : generates and initialises an N_array node
 *  global vars   :
 *  internal funs : MakeNode
 *  external funs :
 *  macros        : DBUG..., EXPRS_EXPR, EXPRS_NEXT
 *
 *  remarks       :
 *
 */

node *
MakeVinfo (useflag flag, shapes *shp, node *next)
{
    node *vinfo;

    DBUG_ENTER ("MakeVinfo");

    vinfo = MakeNode (N_vinfo);
    VINFO_FLAG (vinfo) = flag;
    VINFO_SHP (vinfo) = shp;
    VINFO_NEXT (vinfo) = next;
    if (next == NULL)
        vinfo->nnode = 0;
    else
        vinfo->nnode = 1;

    DBUG_RETURN (vinfo);
}

/*
 * Some conversion functions:
 *
 * node *Shape2Array( shapes *shp) : creates an array-node of type int[n] with
 *                                   all sub-structures needed that represents
 *                                   the shape vector.
 */

/*
 *
 *  functionname  : Shape2Array
 *  arguments     : 1) shapes *shp
 *  description   : creates an array-node of type int[n] with all sub-structures
 *                  needed that represents the shape vector shp.
 *  global vars   :
 *  internal funs : MakeExprs, MakeNum
 *  external funs :
 *  macros        : DBUG..., SHAPES_SELEMS, SHAPES_DIM
 *
 *  remarks       :
 *
 */

node *
Shape2Array (shapes *shp)

{
    int i;
    node *next;

    DBUG_ENTER ("Shape2Array");

    i = SHAPES_DIM (shp) - 1;
    next = MakeExprs (MakeNum (SHAPES_SELEMS (shp)[i]), NULL);
    i--;
    for (; i >= 0; i--) {
        next = MakeExprs (MakeNum (SHAPES_SELEMS (shp)[i]), next);
        next->nnode = 2;
    }
    DBUG_RETURN (MakeArray (next));
}
