/*
 *
 * $Log$
 * Revision 1.8  1995/10/20 16:52:35  cg
 * functions InsertNode, InsertNodes, and InsertUnresolvedNodes
 * transformed into void functions and renamed into
 * StoreNeededNode, StoreNeededNodes, and StoreUnresolvedNodes
 * respectively.
 *
 * Revision 1.7  1995/10/20  13:46:47  cg
 * added additional parameter in functions InsertNode, InsertNodes,
 * and InsertUnresolvedNodes.
 * Now the status of the nodelist entry can be given as well.
 *
 * Revision 1.6  1995/10/20  09:27:31  cg
 * bug fixes in 'InsertNode`
 *
 * Revision 1.5  1995/10/19  10:07:51  cg
 * functions InsertNode, InsertNodes and InsertUnresolvedNodes
 * modified in signature.
 *
 * Revision 1.4  1995/10/06  17:19:36  cg
 * functions InsertNode InsertNodes and InsertUnresolvedNodes for dealing with
 * type nodelist added.
 *
 * Revision 1.3  1995/10/01  16:40:31  cg
 * function SearchFundef added.
 *
 * Revision 1.2  1995/10/01  13:04:32  cg
 * added functions SearchTypedef, CountNums, CopyShpseg, MergeShpseg
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "my_debug.h"

/***
 ***  CmpDomain
 ***/

int
CmpDomain (node *arg1, node *arg2)
{
    int i, is_equal;

    DBUG_ENTER ("CmpDomain");

    while ((NULL != arg1) && (NULL != arg2)) {
        if (ARG_BASETYPE (arg1) == ARG_BASETYPE (arg2)) {
            if (ARG_BASETYPE (arg1) == T_user) {
                if (!CMP_TYPE_USER (ARG_TYPE (arg1), ARG_TYPE (arg2))) {
                    break;
                }
            }
            if (ARG_DIM (arg1) == ARG_DIM (arg2)) {
                for (i = 0; i < ARG_DIM (arg1); i++)
                    if (ARG_SHAPE (arg1, i) != ARG_SHAPE (arg2, i))
                        break;
                if (i != ARG_DIM (arg1))
                    break;
                else {
                    arg1 = ARG_NEXT (arg1);
                    arg2 = ARG_NEXT (arg2);
                }
            } else
                break;
        } else
            break;
    }
    if ((NULL == arg1) && (NULL == arg2)) {
        is_equal = 1;

        DBUG_PRINT ("TREE", ("Domain compare positive !"));
    } else {
        is_equal = 0;

        DBUG_PRINT ("TREE", ("Domain compare negative !"));
    }

    DBUG_RETURN (is_equal);
}

/***
 ***  SearchFundef
 ***/

node *
SearchFundef (node *fun, node *allfuns)
{
    node *tmp;

    DBUG_ENTER ("SearchFundef");

    tmp = allfuns;
    while ((tmp != NULL) && (CMP_FUNDEF (tmp, fun) == 0)) {
        tmp = FUNDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
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
 *  description   : creates an array-node of type int[n] with all
 *                  sub-structures
 *                  needed that represent the shape vector shp.
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

/***
 ***  ObjList2ArgList
 ***/

void
ObjList2ArgList (node *objdef)
{
    node *tmp;

    DBUG_ENTER ("ObjList2ArgList");

    tmp = objdef;
    while (tmp != NULL) {
        NODE_TYPE (tmp) = N_arg;
        ARG_NEXT (tmp) = OBJDEF_NEXT (tmp);
        tmp = ARG_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

/***
 ***  SearchTypedef
 ***/

node *
SearchTypedef (char *name, char *mod, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("SearchType");

    DBUG_PRINT ("WRITESIB", ("Searching type %s:%s", mod, name));

    tmp = implementations;
    while ((tmp != NULL) && (CMP_TYPE_TYPEDEF (name, mod, tmp) == 0)) {
        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
}

/***
 ***  CountNums
 ***/

int
CountNums (nums *numsp)
{
    int cnt = 0;

    DBUG_ENTER ("CountNums");

    while (numsp != NULL) {
        cnt++;
    }

    DBUG_RETURN (cnt);
}

/***
 ***  CopyShpseg
 ***/

shpseg *
CopyShpseg (shpseg *old)
{
    shpseg *new = NULL;
    int i;

    DBUG_ENTER ("CopyShpseg");

    if (old != NULL) {
        new = MakeShpseg (NULL);

        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (new, i) = SHPSEG_SHAPE (old, i);
        }
    }

    DBUG_RETURN (new);
}

/***
 ***  MergeShpseg
 ***/

shpseg *
MergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2)
{
    shpseg *new;
    int i;

    DBUG_ENTER ("MergeShpseg");

    new = MakeShpseg (NULL);

    for (i = 0; i < dim1; i++) {
        SHPSEG_SHAPE (new, i) = SHPSEG_SHAPE (first, i);
    }

    for (i = 0; i < dim2; i++) {
        SHPSEG_SHAPE (new, i + dim1) = SHPSEG_SHAPE (second, i);
    }

    DBUG_RETURN (new);
}

/***
 ***  StoreNeededNode
 ***/

void
StoreNeededNode (node *insert, node *fundef, statustype status)
{
    nodelist *act, *last, *list;

    DBUG_ENTER ("StoreNeededNode");

    DBUG_PRINT ("ANA", ("Function '%s` needs '%s` (%s)", ItemName (fundef),
                        ItemName (insert), mdb_nodetype[NODE_TYPE (insert)]));

    switch (NODE_TYPE (insert)) {
    case N_fundef:
        list = FUNDEF_NEEDFUNS (fundef);
        break;

    case N_objdef:
        list = FUNDEF_NEEDOBJS (fundef);
        break;

    case N_typedef:
        list = FUNDEF_NEEDTYPES (fundef);
        break;

    default:
        DBUG_ASSERT (0, "Wrong insert node in call to function 'StoreNeededNode`");
    }

    if (list == NULL) {
        switch (NODE_TYPE (insert)) {
        case N_fundef:
            FUNDEF_NEEDFUNS (fundef) = MakeNodelist (insert, status, NULL);
            break;

        case N_objdef:
            FUNDEF_NEEDOBJS (fundef) = MakeNodelist (insert, status, NULL);
            break;

        case N_typedef:
            FUNDEF_NEEDTYPES (fundef) = MakeNodelist (insert, status, NULL);
            break;

        default:
            DBUG_ASSERT (0, "Wrong insert node in call to function 'StoreNeededNode`");
        }
    } else {
        act = list;
        last = list;

        while ((act != NULL) && (NODELIST_NODE (act) != insert)) {
            last = act;
            act = NODELIST_NEXT (act);
        }

        if (act == NULL) {
            NODELIST_NEXT (last) = MakeNodelist (insert, status, NULL);
        }
    }

    DBUG_VOID_RETURN;
}

/***
 ***  StoreNeededNodes
 ***/

void
StoreNeededNodes (nodelist *inserts, node *fundef, statustype status)
{
    DBUG_ENTER ("StoreNeededNodes");

    while (inserts != NULL) {
        StoreNeededNode (NODELIST_NODE (inserts), fundef, status);
        inserts = NODELIST_NEXT (inserts);
    }

    DBUG_VOID_RETURN;
}

/***
 ***  StoreUnresolvedNodes
 ***/

void
StoreUnresolvedNodes (nodelist *inserts, node *fundef, statustype status)
{
    DBUG_ENTER ("StoreUnresolvedNodes");

    while (inserts != NULL) {
        if (NODELIST_ATTRIB (inserts) == ST_unresolved) {
            StoreNeededNode (NODELIST_NODE (inserts), fundef, status);
        }
        inserts = NODELIST_NEXT (inserts);
    }

    DBUG_VOID_RETURN;
}
