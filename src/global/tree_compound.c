/*
 *
 * $Log$
 * Revision 1.45  1999/02/09 14:47:07  dkr
 * removed unused var in NodeListFind()
 *
 * Revision 1.44  1999/02/06 12:49:59  srs
 * inserted the following functions
 * NodeListAppend()
 * NodeListDelete()
 * NodeListFree()
 * NodeListFind()
 *
 * Revision 1.43  1999/01/26 14:25:46  cg
 * Added functions MakePrf1(), MakePrf2(), and MakePrf3() to
 * create N_prf nodes with 1,2, or 3 arguments, repsectively.
 *
 * Revision 1.42  1999/01/19 14:20:38  sbs
 * some warnings concerning uninitialized values eliminated.
 *
 * Revision 1.41  1998/07/03 10:16:38  cg
 * new functions MakeIcm[012345] added
 * function AppendExpr renamed to AppendExprs
 *
 * Revision 1.40  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.39  1998/06/04 16:59:20  cg
 * added function LookupIds that looks for a given identifier
 * within an ids-chain.
 *
 * Revision 1.38  1998/05/12 12:19:48  dkr
 * added AppendAssign, AppendExpr
 *
 * Revision 1.37  1998/03/12 12:45:56  srs
 * fixed IsConstArray()
 *
 * Revision 1.36  1998/03/10 11:13:43  srs
 * changed parameters of IsConstantArray()
 *
 * Revision 1.35  1998/03/07 17:01:19  srs
 * added IsConstantArray()
 *
 * Revision 1.34  1998/02/12 10:34:02  srs
 * changed GetCompoundNode() to work with new WLs
 *
 * Revision 1.33  1997/11/05 16:30:34  dkr
 * moved nnode[] from tree_compound.[ch] to traverse.[ch]
 *
 * Revision 1.32  1997/11/05 09:37:22  dkr
 * introduces a new array nnode[]: nodetype -> int
 * this array replaces the node->nnode field
 *
 * Revision 1.31  1997/10/29 17:17:33  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.30  1997/10/29 14:33:38  srs
 * free -> FREE
 *
 * Revision 1.29  1997/04/25 13:02:06  sacbase
 * DBUG_PRINT in SearchObjdef and SearchTypedef :
 * mod -> MOD(mod) (mod may be NULL!!)
 *
 * Revision 1.28  1996/04/02  19:35:10  cg
 * function string2array moved to sac.y
 *
 * Revision 1.27  1996/03/21  18:00:20  cg
 * added function string2array for new-fashioned string handling
 *
 * Revision 1.26  1996/02/21  15:03:13  cg
 * added new function CopyNodelist
 *
 * Revision 1.25  1996/02/21  10:56:08  cg
 * function CmpDomain now handles negative dimensions of arrays
 *
 * Revision 1.24  1996/01/07  16:55:09  cg
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
#include "free.h"

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
                if (ARG_DIM (arg1) > 0) {
                    for (i = 0; i < ARG_DIM (arg1); i++)
                        if (ARG_SHAPE (arg1, i) != ARG_SHAPE (arg2, i))
                            break;
                    if (i != ARG_DIM (arg1))
                        break;
                    else {
                        arg1 = ARG_NEXT (arg1);
                        arg2 = ARG_NEXT (arg2);
                    }
                } else {
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
#ifndef NEWTREE
        next->nnode = 2;
#endif
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

    DBUG_PRINT ("CHECKDEC", ("Searching type '%s`", ModName (mod, name)));

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

    DBUG_PRINT ("CHECKDEC", ("Searching global object '%s`", ModName (mod, name)));

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
        list = NULL;
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
        FREE (tmp);
    }

    first = list;

    if (list != NULL) {
        last = list;
        list = NODELIST_NEXT (list);

        while (list != NULL) {
            if (NODELIST_STATUS (list) == ST_artificial) {
                tmp = list;
                NODELIST_NEXT (last) = NODELIST_NEXT (list);
                list = NODELIST_NEXT (list);
                FREE (tmp);
            } else {
                last = list;
                list = NODELIST_NEXT (list);
            }
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
 *** LookupIds
 ***/

ids *
LookupIds (char *name, ids *ids_chain)
{
    DBUG_ENTER ("LookupIds");

    while ((ids_chain != NULL) && (0 != strcmp (name, IDS_NAME (ids_chain)))) {
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (ids_chain);
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
        if (N_with == NODE_TYPE (arg_node) || N_Nwith == NODE_TYPE (arg_node))
            compound_node = arg_node;
        else
            compound_node = NULL;
        break;
    default:
        compound_node = NULL;
    }
    DBUG_RETURN (compound_node);
}

/******************************************************************************
 *
 * function:
 *   node *MakePrf1(prf prf, node* arg1)
 *   node *MakePrf2(prf prf, node* arg1, node* arg2)
 *   node *MakePrf3(prf prf, node* arg1, node* arg2, node* arg3)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
MakePrf1 (prf prf, node *arg1)
{
    node *res;

    DBUG_ENTER ("MakePrf1");

    res = MakePrf (prf, MakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}

node *
MakePrf2 (prf prf, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ("MakePrf2");

    res = MakePrf (prf, MakeExprs (arg1, MakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
MakePrf3 (prf prf, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ("MakePrf3");

    res = MakePrf (prf, MakeExprs (arg1, MakeExprs (arg2, MakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *AppendAssign( node *assigns, node *assign)
 *
 * description:
 *   appends 'assign' to the N_assign-chain 'assings' and returns the new
 *    chain.
 *
 ******************************************************************************/

node *
AppendAssign (node *assigns, node *assign)
{
    node *tmp;

    DBUG_ENTER ("AppendAssign");

    if (assign != NULL) {
        if (assigns != NULL) {
            tmp = assigns;
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            ASSIGN_NEXT (tmp) = assign;
        } else {
            assigns = assign;
        }
    }

    DBUG_RETURN (assigns);
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

/***
 ***  CopyNodelist
 ***/

extern nodelist *
CopyNodelist (nodelist *nl)
{
    nodelist *copy;

    DBUG_ENTER ("CopyNodelist");

    if (nl == NULL) {
        copy = NULL;
    } else {
        copy = MakeNodelist (NODELIST_NODE (nl), NODELIST_STATUS (nl),
                             CopyNodelist (NODELIST_NEXT (nl)));
        NODELIST_ATTRIB (copy) = NODELIST_ATTRIB (nl);
    }

    DBUG_RETURN (copy);
}

/******************************************************************************
 *
 * function:
 *   -
 *
 * description:
 *   the following functions realize basic functions on pure node lists.
 *
 *   Append: appends a node to the given list, returning a new list.
 *           Since the node list has no special order, the new node is
 *           not appended but put in front of the given list to speed
 *           up execution.
 *           Create a list: newlist = Append(NULL, newnode, attrib);
 *   Delete: deletes all elements of the given node. If free_attrib is 0,
 *           the attribut is not set free, else a FREE(attrib) is executed.
 *   Free  : frees whole list. If free_attrib is 0, the attributes are
 *           not set free, else a FREE(attrib) is executed.
 *   Find  : returns the nodelist node of the first found item
 *           with fitting node. If not found, returns NULL.
 *
 ******************************************************************************/

nodelist *
NodeListAppend (nodelist *nl, node *newnode, void *attrib)
{
    nl = MakeNodelistNode (newnode, nl);
    NODELIST_ATTRIB2 (nl) = attrib;
    return nl;
}

nodelist *
NodeListDelete (nodelist *nl, node *node, int free_attrib)
{
    nodelist *tmpnl, *prevnl;

    do {
        if (NODELIST_NODE (nl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (nl))
                FREE (NODELIST_ATTRIB2 (nl));
            nl = FreeNodelistNode (nl);
        }
    } while (nl && NODELIST_NODE (nl) == node);

    tmpnl = nl;
    prevnl = NULL;
    while (tmpnl) {
        if (NODELIST_NODE (tmpnl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (tmpnl))
                FREE (NODELIST_ATTRIB2 (tmpnl));
            prevnl = FreeNodelistNode (tmpnl);
        } else
            prevnl = tmpnl;
        tmpnl = NODELIST_NEXT (prevnl);
    }

    return nl;
}

nodelist *
NodeListFree (nodelist *nl, int free_attrib)
{
    while (nl) {
        if (free_attrib && NODELIST_ATTRIB2 (nl))
            FREE (NODELIST_ATTRIB2 (nl));
        nl = FreeNodelistNode (nl);
    }

    return nl;
}

nodelist *
NodeListFind (nodelist *nl, node *node)
{
    while (nl && NODELIST_NODE (nl) != node)
        nl = NODELIST_NEXT (nl);

    return nl;
}

/******************************************************************************
 *
 * function:
 *   int IsConstantArray(node *array)
 *
 * description:
 *   returns number of constant elements if argument is an N_array and all
 *   its elements are N_num, N_char, N_float, N_double, N_bool or otherwise
 *   returns 0.
 *
 *   The parameter type specified the necessary type all elements have to
 *   be of (nodetype, e.g. N_num). If N_ok is given, the type is ignored.
 *
 ******************************************************************************/

int
IsConstantArray (node *array, nodetype type)
{
    int elems = 0, ok = 1;

    DBUG_ENTER ("IsConstantArray");

    if (N_array == NODE_TYPE (array))
        array = ARRAY_AELEMS (array);
    else
        ok = 0;

    while (ok && array) {
        switch (NODE_TYPE (EXPRS_EXPR (array))) {
        case N_num:
        case N_char:
        case N_float:
        case N_double:
        case N_bool:
            if (N_ok != type && type != NODE_TYPE (EXPRS_EXPR (array)))
                ok = 0;
            elems++;
            array = EXPRS_NEXT (array);

            break;
        default:
            ok = 0;
        }
    }

    DBUG_RETURN (ok ? elems : 0);
}

/******************************************************************************
 *
 * function:
 *   node *AppendExprs(node *exprs1, node *exprs2)
 *
 * description:
 *
 *   This function concatenates two N_exprs chains of nodes.
 *
 *
 *
 ******************************************************************************/

node *
AppendExprs (node *exprs1, node *exprs2)
{
    node *res;

    DBUG_ENTER ("AppendExprs");

    if (exprs1 == NULL) {
        res = exprs2;
    } else {
        res = exprs1;

        while (EXPRS_NEXT (exprs1) != NULL) {
            exprs1 = EXPRS_NEXT (exprs1);
        }

        EXPRS_NEXT (exprs1) = exprs2;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *MakeIcm0(char *name)
 *   node *MakeIcm1(char *name, node *arg1)
 *   node *MakeIcm3(char *name, node *arg1, node *arg2)
 *   node *MakeIcm4(char *name, node *arg1, node *arg2, node *arg3, node *arg4)
 *   node *MakeIcm5(char *name, node *arg1, node *arg2, node *arg3, node *arg4, node
 **arg5)
 *
 * description:
 *
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *
 ******************************************************************************/

node *
MakeIcm0 (char *name)
{
    node *icm;

    DBUG_ENTER ("MakeIcm0");

    icm = MakeIcm (name, NULL, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm1 (char *name, node *arg1)
{
    node *icm;

    DBUG_ENTER ("MakeIcm1");

    if (NODE_TYPE (arg1) != N_exprs) {
        arg1 = MakeExprs (arg1, NULL);
    }

    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm2 (char *name, node *arg1, node *arg2)
{
    node *icm;

    DBUG_ENTER ("MakeIcm2");

    if (NODE_TYPE (arg2) != N_exprs) {
        arg2 = MakeExprs (arg2, NULL);
    }

    if (NODE_TYPE (arg1) != N_exprs) {
        arg1 = MakeExprs (arg1, arg2);
    } else {
        arg1 = AppendExprs (arg1, arg2);
    }

    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm3 (char *name, node *arg1, node *arg2, node *arg3)
{
    node *icm;

    DBUG_ENTER ("MakeIcm3");

    if (NODE_TYPE (arg3) != N_exprs) {
        arg3 = MakeExprs (arg3, NULL);
    }

    if (NODE_TYPE (arg2) != N_exprs) {
        arg2 = MakeExprs (arg2, arg3);
    } else {
        arg2 = AppendExprs (arg2, arg3);
    }

    if (NODE_TYPE (arg1) != N_exprs) {
        arg1 = MakeExprs (arg1, arg2);
    } else {
        arg1 = AppendExprs (arg1, arg2);
    }

    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *icm;

    DBUG_ENTER ("MakeIcm4");

    if (NODE_TYPE (arg4) != N_exprs) {
        arg4 = MakeExprs (arg4, NULL);
    }

    if (NODE_TYPE (arg3) != N_exprs) {
        arg3 = MakeExprs (arg3, arg4);
    } else {
        arg3 = AppendExprs (arg3, arg4);
    }

    if (NODE_TYPE (arg2) != N_exprs) {
        arg2 = MakeExprs (arg2, arg3);
    } else {
        arg2 = AppendExprs (arg2, arg3);
    }

    if (NODE_TYPE (arg1) != N_exprs) {
        arg1 = MakeExprs (arg1, arg2);
    } else {
        arg1 = AppendExprs (arg1, arg2);
    }

    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    node *icm;

    DBUG_ENTER ("MakeIcm5");

    if (NODE_TYPE (arg5) != N_exprs) {
        arg5 = MakeExprs (arg5, NULL);
    }

    if (NODE_TYPE (arg4) != N_exprs) {
        arg4 = MakeExprs (arg4, arg5);
    } else {
        arg4 = AppendExprs (arg4, arg5);
    }

    if (NODE_TYPE (arg3) != N_exprs) {
        arg3 = MakeExprs (arg3, arg4);
    } else {
        arg3 = AppendExprs (arg3, arg4);
    }

    if (NODE_TYPE (arg2) != N_exprs) {
        arg2 = MakeExprs (arg2, arg3);
    } else {
        arg2 = AppendExprs (arg2, arg3);
    }

    if (NODE_TYPE (arg1) != N_exprs) {
        arg1 = MakeExprs (arg1, arg2);
    } else {
        arg1 = AppendExprs (arg1, arg2);
    }

    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}
