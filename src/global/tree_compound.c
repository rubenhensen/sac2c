/*
 *
 * $Log$
 * Revision 1.24  1996/01/07 16:55:09  cg
 * function CountFunctionParams now counts return type void
 *
 * Revision 1.23  1996/01/05  18:20:22  asi
 * bug fixed in GetCompoundNode
 *
 * Revision 1.22  1996/01/02  12:48:41  cg
 * added function StringsLength
 *
 * Revision 1.21  1995/12/29  12:53:23  cg
 * added function StoreString
 *
 * Revision 1.20  1995/12/29  10:33:11  cg
 * added ConcatNodelist
 *
 * Revision 1.19  1995/12/21  10:36:55  cg
 * added function CountFunctionParams
 *
 * Revision 1.18  1995/12/15  14:13:56  asi
 * added GetCompoundNode
 *
 * Revision 1.17  1995/12/07  16:25:14  asi
 * added function MakeAssignLet
 *
 * Revision 1.16  1995/11/16  19:42:36  cg
 * Function FreeNodelist moved to free.c
 *
 * Revision 1.15  1995/11/06  14:16:51  cg
 * added new internal function 'CompatibleAttributes.
 * used by function CmpDomain
 *
 * Revision 1.14  1995/11/01  16:25:01  cg
 * new function AppendIdsChain from tree.c and converted to new macros
 *
 * Revision 1.13  1995/10/31  09:40:31  cg
 * bug fixed: free.h now included.
 *
 * Revision 1.12  1995/10/31  08:54:43  cg
 * added new functions FreeNodelist and TidyUpNodelist.
 *
 * Revision 1.11  1995/10/26  15:59:54  cg
 * macro MOD_NAME_CON replaced by new global variable mod_name_con
 * Now, different strings can be used for combining module name and
 * item name with respect to the compilation phase.
 *
 * Revision 1.10  1995/10/24  13:13:53  cg
 * function CmpDomain now considers argument attributes
 *
 * Revision 1.9  1995/10/22  17:29:59  cg
 * new function SearchObjdef
 * new compound access macros for fundec and typedec
 * macro CMP_TYPE_USER now tests if argument actually is T_user.
 *
 * Revision 1.8  1995/10/20  16:52:35  cg
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

#include <malloc.h>

/***
 ***  mod_name_con
 ***/

char mod_name_con_1[] = "__";

char mod_name_con_2[] = ":";

char *mod_name_con = mod_name_con_1;

/***
 ***  StoreString
 ***/

strings *
StoreString (strings *list, char *str)
{
    strings *tmp;

    DBUG_ENTER ("StoreString");

    tmp = list;

    while ((tmp != NULL) && (strcmp (STRINGS_STRING (tmp), str) != 0)) {
        tmp = STRINGS_NEXT (tmp);
    }

    if (tmp == NULL) {
        tmp = MakeStrings (str, list);
    } else {
        tmp = list;
    }

    DBUG_RETURN (tmp);
}

/***
 ***  StringsLength
 ***/

int
StringsLength (strings *list)
{
    int counter = 0;

    DBUG_ENTER ("StringsLength");

    while (list != NULL) {
        counter += strlen (STRINGS_STRING (list)) + 1;
        list = STRINGS_NEXT (list);
    }

    DBUG_RETURN (counter);
}

/***
 ***  CompatibleAttributes
 ***/

int
CompatibleAttributes (statustype attrib1, statustype attrib2)
{
    int ret;

    DBUG_ENTER ("CompatibleAttributes");

    if ((attrib1 == ST_regular) && (attrib2 == ST_unique)) {
        ret = 1;
    } else if ((attrib2 == ST_regular) && (attrib1 == ST_unique)) {
        ret = 1;
    } else {
        ret = (attrib1 == attrib2);
    }

    DBUG_RETURN (ret);
}

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
                if (!CompatibleAttributes (ARG_ATTRIB (arg1), ARG_ATTRIB (arg2))) {
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

    DBUG_ENTER ("SearchTypedef");

    DBUG_PRINT ("CHECKDEC", ("Searching type %s:%s", mod, name));

    tmp = implementations;
    while ((tmp != NULL) && (CMP_TYPE_TYPEDEF (name, mod, tmp) == 0)) {
        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
}

/***
 ***  SearchObjdef
 ***/

node *
SearchObjdef (char *name, char *mod, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("SearchObjdef");

    DBUG_PRINT ("CHECKDEC", ("Searching type %s:%s", mod, name));

    tmp = implementations;
    while ((tmp != NULL) && (CMP_OBJ_OBJDEF (name, mod, tmp) == 0)) {
        tmp = OBJDEF_NEXT (tmp);
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

/***
 ***  TidyUpNodelist
 ***/

nodelist *
TidyUpNodelist (nodelist *list)
{
    nodelist *tmp, *first, *last;

    DBUG_ENTER ("TidyUpNodelist");

    while ((list != NULL) && (NODELIST_STATUS (list) == ST_artificial)) {
        tmp = list;
        list = NODELIST_NEXT (list);
        free (tmp);
    }

    first = list;

    while (list != NULL) {
        if (NODELIST_STATUS (list) == ST_artificial) {
            tmp = list;
            NODELIST_NEXT (last) = NODELIST_NEXT (list);
            list = NODELIST_NEXT (list);
            free (tmp);
        } else {
            last = list;
            list = NODELIST_NEXT (list);
        }
    }

    DBUG_RETURN (first);
}

/***
 ***  ConcatNodelist
 ***/

nodelist *
ConcatNodelist (nodelist *first, nodelist *second)
{
    nodelist *tmp;

    DBUG_ENTER ("ConcatNodelist");

    if (first == NULL) {
        first = second;
    } else {
        tmp = first;

        while (NODELIST_NEXT (tmp) != NULL) {
            tmp = NODELIST_NEXT (tmp);
        }

        NODELIST_NEXT (tmp) = second;
    }

    DBUG_RETURN (first);
}

/***
 ***  AppendIdsChain
 ***/

ids *
AppendIdsChain (ids *first, ids *second)
{
    ids *tmp;

    DBUG_ENTER ("AppendIdsChain");

    if (first == NULL)
        first = second;
    else {
        tmp = first;
        while (IDS_NEXT (tmp) != NULL)
            tmp = IDS_NEXT (tmp);
        IDS_NEXT (tmp) = second;
    }

    DBUG_RETURN (first);
}

/***
 ***  MakeAssignLet
 ***/
node *
MakeAssignLet (char *var_name, node *vardec_node, node *let_expr)
{
    ids *tmp_ids;
    node *tmp_node;

    DBUG_ENTER ("MakeAssignLet");
    tmp_ids = MakeIds (var_name, NULL, ST_regular);
    IDS_VARDEC (tmp_ids) = vardec_node;
    tmp_node = MakeLet (let_expr, tmp_ids);
    tmp_node = MakeAssign (tmp_node, NULL);
    DBUG_RETURN (tmp_node);
}

/***
 ***  GetCompoundNode
 ***/
node *
GetCompoundNode (node *arg_node)
{
    node *compound_node;

    DBUG_ENTER ("GetCompoundNode");
    arg_node = ASSIGN_INSTR (arg_node);
    switch (NODE_TYPE (arg_node)) {
    case N_cond:
    case N_do:
    case N_while:
        compound_node = arg_node;
        break;
    case N_let:
        arg_node = LET_EXPR (arg_node);
        while (N_cast == NODE_TYPE (arg_node))
            arg_node = CAST_EXPR (arg_node);
        if (N_with == NODE_TYPE (arg_node))
            compound_node = arg_node;
        else
            compound_node = NULL;
        break;
    default:
        compound_node = NULL;
    }
    DBUG_RETURN (compound_node);
}

/***
 ***  CountFunctionParams
 ***/

int
CountFunctionParams (node *fundef)
{
    int count = 0;
    types *tmp;
    node *tmp2;

    DBUG_ENTER ("CountFunctionParams");

    tmp = FUNDEF_TYPES (fundef);

    while (tmp != NULL) {
        count++;
        tmp = TYPES_NEXT (tmp);
    }

    tmp2 = FUNDEF_ARGS (fundef);

    while (tmp2 != NULL) {
        count++;
        tmp2 = ARG_NEXT (tmp2);
    }

    DBUG_RETURN (count);
}
