/*
 *
 * $Log$
 * Revision 1.3  2000/02/22 11:59:06  jhs
 * Adapted NODE_TEXT.
 *
 * Revision 1.2  2000/01/25 13:41:15  dkr
 * some rearrangements made
 * all the constvec stuff moved from internal_lib.c
 * function FindVardec_Name and FindVardec_Varno added
 *
 * Revision 1.1  2000/01/21 15:38:27  dkr
 * Initial revision
 *
 * Revision 2.17  1999/11/15 14:24:16  dkr
 * assert in IsConstArray removed
 *
 * Revision 2.16  1999/11/12 12:41:40  dkr
 * IsConstArray revisited: accidentaly the new version was not activated.
 *   some comments added, some minor changes done.
 *
 * Revision 2.15  1999/11/11 20:07:21  dkr
 * function IsConstantArray changed:
 *   new name (IsConstArray)
 *   new signature (no type parameter anymore)
 *   new return value (boolean instead of #elements)
 *   NOW THIS FUNCTION (hopefully) works correct :)
 *
 * Revision 2.14  1999/11/09 21:15:31  dkr
 * Functions MakeIcm7, MakeAssignIcm7 added.
 *
 * Revision 2.13  1999/10/27 15:12:41  sbs
 * MakeAssignIcmX(...) added.
 *
 * Revision 2.12  1999/10/26 15:51:00  dkr
 * Bug in MakeIcm{4,5,6} fixed
 *
 * Revision 2.11  1999/09/20 11:33:32  jhs
 * Added (L_)VARDEC_OR_ARG_BNEXT.
 * Brushed MakeIcm(0..6).
 *
 * Revision 2.10  1999/09/01 12:31:35  sbs
 * IsConstantArray now is able to deal with NULL-args!
 *
 * Revision 2.9  1999/07/08 14:59:12  sbs
 * Array2BoolVec added
 *
 * Revision 2.8  1999/07/07 05:59:33  sbs
 * Added function MakeVinfoDollar
 *
 * Revision 2.7  1999/05/12 08:38:14  sbs
 * ID_CONSTARRAY changed into ID_ISCONST
 *
 * Revision 2.6  1999/04/26 10:53:59  bs
 * Function 'IsConstantArray' modified.
 * These changes are only temporary ones!!
 *
 * Revision 2.5  1999/04/16 18:48:10  bs
 * Bug fixed in IntVec2Array.
 *
 * Revision 2.4  1999/03/31 10:51:31  bs
 * Function Array2CharVec added.
 *
 * Revision 2.3  1999/03/15 14:09:04  bs
 * Access macros renamed (take a look at tree_basic.h).
 * Functions Array2FloatVec and Array2Dblvec added.
 *
 * Revision 2.2  1999/03/04 10:07:13  bs
 * Functions IntVec2Array() and Array2IntVec() added.
 *
 * Revision 2.1  1999/02/23 12:39:57  sacbase
 * new release made
 *
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
 * [...]
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "my_debug.h"
#include "free.h"

/*--------------------------------------------------------------------------*/
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  Shpseg :
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

/*--------------------------------------------------------------------------*/

/***
 ***  Ids :
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

ids *
LookupIds (char *name, ids *ids_chain)
{
    DBUG_ENTER ("LookupIds");

    while ((ids_chain != NULL) && (0 != strcmp (name, IDS_NAME (ids_chain)))) {
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (ids_chain);
}

/*--------------------------------------------------------------------------*/

/***
 ***  Nums :
 ***/

int
CountNums (nums *numsp)
{
    int cnt = 0;

    DBUG_ENTER ("CountNums");

    while (numsp != NULL) {
        cnt++;
        numsp = numsp->next;
    }

    DBUG_RETURN (cnt);
}

/*--------------------------------------------------------------------------*/

/***
 ***  ConstVec :
 ***/

/******************************************************************************
 *
 * function:
 *  void *CopyConstVec( simpletype vectype, int veclen, void *const_vec)
 *
 * description:
 *   allocates a new vector of size veclen*sizeof(vectype) and copies all
 *   elements from const_vec into it.
 *
 ******************************************************************************/

void *
CopyConstVec (simpletype vectype, int veclen, void *const_vec)
{
    int n;
    void *res;

    DBUG_ENTER ("CopyConstVec");
    if (veclen > 0) {
        switch (vectype) {
        case T_bool:
        case T_int:
            n = veclen * sizeof (int);
            res = Malloc (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_float:
            n = veclen * sizeof (float);
            res = Malloc (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_double:
            n = veclen * sizeof (double);
            res = Malloc (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_char:
            n = veclen * sizeof (char);
            res = Malloc (n);
            res = memcpy (res, const_vec, n);
            break;
        default:
            DBUG_ASSERT ((0), "CopyConstVec called with non-const-type!");
            res = NULL;
        }
    } else {
        res = NULL;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  void *AllocConstVec( simpletype vectype, int veclen)
 *
 * description:
 *   allocates a new vector of size veclen*sizeof(vectype).
 *
 ******************************************************************************/

void *
AllocConstVec (simpletype vectype, int veclen)
{
    void *res;

    DBUG_ENTER ("AllocConstVec");
    if (veclen > 0) {
        switch (vectype) {
        case T_bool:
        case T_int:
            res = Malloc (veclen * sizeof (int));
            break;
        case T_float:
            res = Malloc (veclen * sizeof (float));
            break;
        case T_double:
            res = Malloc (veclen * sizeof (double));
            break;
        case T_char:
            res = Malloc (veclen * sizeof (int));
            break;
        default:
            DBUG_ASSERT ((0), "AllocConstVec called with non-const-type!");
            res = NULL;
        }
    } else {
        res = NULL;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  void *ModConstVec( simpletype vectype, void *const_vec, int idx,
 *                                                          node *const_node)
 *
 * description:
 *   modifies const_vec at position idx by inserting the value stored in
 *   const_node. It is assumed (!!!) that simpletype is compatible to the const_node;
 *   if this requires a cast (e.g. NODE_TYPE(const_node) == N_num && vectype==T_double)
 *   it will be done implicitly.
 *
 ******************************************************************************/

void *
ModConstVec (simpletype vectype, void *const_vec, int idx, node *const_node)
{
    DBUG_ENTER ("ModConstVec");
    switch (vectype) {
    case T_bool:
        DBUG_ASSERT ((NODE_TYPE (const_node) == N_bool),
                     "array element type does not match infered vectype!");
        ((int *)const_vec)[idx] = BOOL_VAL (const_node);
        break;
    case T_int:
        DBUG_ASSERT ((NODE_TYPE (const_node) == N_num),
                     "array element type does not match infered vectype!");
        ((int *)const_vec)[idx] = NUM_VAL (const_node);
        break;
    case T_float:
        DBUG_ASSERT (((NODE_TYPE (const_node) == N_num)
                      || (NODE_TYPE (const_node) == N_float)),
                     "array element type does not match infered vectype!");
        if (NODE_TYPE (const_node) == N_num)
            ((float *)const_vec)[idx] = (float)NUM_VAL (const_node);
        else
            ((float *)const_vec)[idx] = FLOAT_VAL (const_node);
        break;
    case T_double:
        DBUG_ASSERT (((NODE_TYPE (const_node) == N_num)
                      || (NODE_TYPE (const_node) == N_float)
                      || (NODE_TYPE (const_node) == N_double)),
                     "array element type does not match infered vectype!");
        if (NODE_TYPE (const_node) == N_num)
            ((double *)const_vec)[idx] = (double)NUM_VAL (const_node);
        else if (NODE_TYPE (const_node) == N_float)
            ((double *)const_vec)[idx] = FLOAT_VAL (const_node);
        else
            ((double *)const_vec)[idx] = DOUBLE_VAL (const_node);
        break;
    case T_char:
        DBUG_ASSERT ((NODE_TYPE (const_node) == N_char),
                     "array element type does not match infered vectype!");
        ((char *)const_vec)[idx] = CHAR_VAL (const_node);
        break;
    default:
        DBUG_ASSERT ((0), "ModConstVec called with non-const-type!");
    }
    DBUG_RETURN (const_vec);
}

/******************************************************************************
 *
 * function:
 *  node *AnnotateIdWithConstVec(node *expr, node *id)
 *
 * description:
 *   - if expr is an N_array-node or an N_id-node,
 *     the const-array decoration from expr is copied to id.
 *   - leading casts will be ignored.
 *   - returns the id-node.
 *
 ******************************************************************************/

node *
AnnotateIdWithConstVec (node *expr, node *id)
{
    node *behind_casts = expr;

    DBUG_ENTER ("AnnotateIdWithConstVec");

    DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                 "AnnotateIdWithConstVec called with non-compliant arguments!");

    while (NODE_TYPE (behind_casts) == N_cast)
        behind_casts = CAST_EXPR (behind_casts);

    if (NODE_TYPE (behind_casts) == N_array) {
        ID_ISCONST (id) = ARRAY_ISCONST (behind_casts);
        ID_VECTYPE (id) = ARRAY_VECTYPE (behind_casts);
        ID_VECLEN (id) = ARRAY_VECLEN (behind_casts);
        if (ID_ISCONST (id)) {
            ID_CONSTVEC (id)
              = CopyConstVec (ARRAY_VECTYPE (behind_casts), ARRAY_VECLEN (behind_casts),
                              ARRAY_CONSTVEC (behind_casts));
        }
    } else if (NODE_TYPE (behind_casts) == N_id) {
        ID_ISCONST (id) = ID_ISCONST (behind_casts);
        ID_VECTYPE (id) = ID_VECTYPE (behind_casts);
        ID_VECLEN (id) = ID_VECLEN (behind_casts);
        if (ID_ISCONST (id)) {
            ID_CONSTVEC (id)
              = CopyConstVec (ID_VECTYPE (behind_casts), ID_VECLEN (behind_casts),
                              ID_CONSTVEC (behind_casts));
        }
    }

    DBUG_RETURN (id);
}

/*--------------------------------------------------------------------------*/

/***
 ***  Nodelist :
 ***/

void
StoreNeededNode (node *insert, node *fundef, statustype status)
{
    nodelist *act, *last, *list;

    DBUG_ENTER ("StoreNeededNode");

    DBUG_PRINT ("ANA", ("Function '%s` needs '%s` (%s)", ItemName (fundef),
                        ItemName (insert), NODE_TEXT (insert)));

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

/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  N_typedef :
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

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
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

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
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

static int
CompatibleAttributes (statustype attrib1, statustype attrib2)
{
    int ret;

    DBUG_ENTER ("CompatibleAttributes");

    if ((attrib1 == ST_regular) && (attrib2 == ST_unique)) {
        ret = 1;
    } else {
        if ((attrib2 == ST_regular) && (attrib1 == ST_unique)) {
            ret = 1;
        } else {
            ret = (attrib1 == attrib2);
        }
    }

    DBUG_RETURN (ret);
}

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***/

/******************************************************************************
 *
 * function:
 *   node *FindVardec_Name( char *name, node *fundef)
 *
 * description:
 *   returns a pointer to the vardec of var with name 'name'
 *
 ******************************************************************************/

node *
FindVardec_Name (char *name, node *fundef)
{
    node *tmp, *vardec = NULL;

    DBUG_ENTER ("FindVardec_Name");

    FOREACH_VARDEC_AND_ARG (fundef, tmp,
                            if (strcmp (VARDEC_OR_ARG_NAME (tmp), name) == 0) {
                                vardec = tmp;
                                break;
                            }) /* FOREACH_VARDEC_AND_ARG */

    if (vardec == NULL) {
        DBUG_PRINT ("SPMDL", ("Cannot find %s", name));
    }

    DBUG_RETURN (vardec);
}

/******************************************************************************
 *
 * function:
 *   node *FindVardec_Varno(int varno, node *fundef)
 *
 * description:
 *   returns the vardec of var number 'varno'
 *   Search runs through arguments and vardecs of the function, so erery
 *   variable valid in the function has to be found here.
 *
 ******************************************************************************/

node *
FindVardec_Varno (int varno, node *fundef)
{
    node *tmp, *vardec = NULL;

    DBUG_ENTER ("FindVardec_Varno");

    FOREACH_VARDEC_AND_ARG (fundef, tmp, if (ARG_VARNO (tmp) == varno) {
        vardec = tmp;
        break;
    }) /* FOREACH_VARDEC_AND_ARG */

    DBUG_RETURN (vardec);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
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

/******************************************************************************
 *
 * function:
 *   node *MakeAssignIcm0(char *name)
 *   node *MakeAssignIcm1(char *name, node *arg1)
 *   node *MakeAssignIcm3(char *name, node *arg1, node *arg2)
 *   node *MakeAssignIcm4(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4)
 *   node *MakeAssignIcm5(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4, node *arg5)
 *   node *MakeAssignIcm6(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4, node *arg5, node *arg6)
 *
 * description:
 *
 *   These functions generate an N_assign node with a complete ICM
 *   representations including arguments as body.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *   The ASSIGN_NEXT will be NULL!
 *
 ******************************************************************************/

node *
MakeAssignIcm0 (char *name)
{
    return (MakeAssign (MakeIcm0 (name), NULL));
}

node *
MakeAssignIcm1 (char *name, node *arg1)
{
    return (MakeAssign (MakeIcm1 (name, arg1), NULL));
}

node *
MakeAssignIcm2 (char *name, node *arg1, node *arg2)
{
    return (MakeAssign (MakeIcm2 (name, arg1, arg2), NULL));
}

node *
MakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3)
{
    return (MakeAssign (MakeIcm3 (name, arg1, arg2, arg3), NULL));
}

node *
MakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4)
{
    return (MakeAssign (MakeIcm4 (name, arg1, arg2, arg3, arg4), NULL));
}

node *
MakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    return (MakeAssign (MakeIcm5 (name, arg1, arg2, arg3, arg4, arg5), NULL));
}

node *
MakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                node *arg6)
{
    return (MakeAssign (MakeIcm6 (name, arg1, arg2, arg3, arg4, arg5, arg6), NULL));
}

node *
MakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                node *arg6, node *arg7)
{
    return (MakeAssign (MakeIcm7 (name, arg1, arg2, arg3, arg4, arg5, arg6, arg7), NULL));
}

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/******************************************************************************
 *
 * function:
 *   node *AppendExprs(node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
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

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_return :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart:
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithop :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith2 :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***/

/*
 *  functionname  : Shape2Array
 *  arguments     : 1) shapes *shp
 *  description   : creates an array-node of type int[n] with all
 *                  sub-structures needed that represent the shape vector shp.
 *  global vars   :
 *  internal funs : MakeExprs, MakeNum
 *  external funs :
 *  macros        : DBUG..., SHAPES_SELEMS, SHAPES_DIM
 *  remarks       :
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
    }
    DBUG_RETURN (MakeArray (next));
}

/******************************************************************************
 *
 * function:
 *   int IsConstArray(node *array)
 *
 * description:
 *   Returns 1 if argument is an constant array and 0 otherwise.
 *
 * CAUTION:
 *   In this situation, 'constant array' means that all array elements are
 *   constant, irrespectively whether there exists an constant representation
 *   (ARRAY_ISCONST, ARRAY_CONSTVEC) or not!!!
 *
 ******************************************************************************/

int
IsConstArray (node *array)
{
    nodetype type;
    int isconst = 0;

    DBUG_ENTER ("IsConstArray");

    if (array != NULL) {
        switch (NODE_TYPE (array)) {
        case N_array:
            isconst = ARRAY_ISCONST (array);
            if (!isconst) {
                /*
                 * Although ARRAY_ISCONST is false, the array still may be constant:
                 *   1.) For the time being ISCONST is false for all non-int-arrays.
                 *   2.) ISCONST may be inconsistent to this respect.
                 *       (E.g. after some optimizations new constant arrays may occur.)
                 */
                array = ARRAY_AELEMS (array);
                isconst = 1;
                /*
                 * remark: array may be NULL (empty array) -> constant
                 */
                while (array != NULL) {
                    type = NODE_TYPE (EXPRS_EXPR (array));
                    if ((type == N_num) || (type == N_char) || (type == N_bool)
                        || (type == N_float) || (type == N_double)) {
                        array = EXPRS_NEXT (array);
                    } else {
                        isconst = 0;
                        break;
                    }
                }
            }
            break;
        case N_id:
            isconst = ID_ISCONST (array);
            break;
        default:
            isconst = 0;
        }
    } else {
        isconst = 0;
    }

    DBUG_RETURN (isconst);
}

node *
IntVec2Array (int length, int *intvec)
{
    int i;
    node *aelems = NULL;

    DBUG_ENTER ("IntVec2Array");

    for (i = length - 1; i >= 0; i--)
        aelems = MakeExprs (MakeNum (intvec[i]), aelems);

    DBUG_RETURN (aelems);
}

int *
Array2IntVec (node *aelems, int *length)
{
    int *intvec, i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("Array2IntVec");

    while (aelems != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_num),
                     "constant integer array exspected !");
        aelems = EXPRS_NEXT (aelems);
        i++;
    }
    /*
     *  if the length of the vector is not of interrest length may be NULL
     */
    if (length != NULL)
        *length = i;

    intvec = Malloc (i * sizeof (int));

    for (j = 0; j < i; j++) {
        intvec[j] = NUM_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (intvec);
}

int *
Array2BoolVec (node *aelems, int *length)
{
    int *intvec, i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("Array2IntVec");

    while (aelems != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_bool),
                     "constant bool array exspected !");
        aelems = EXPRS_NEXT (aelems);
        i++;
    }
    /*
     *  if the length of the vector is not of interrest length may be NULL
     */
    if (length != NULL)
        *length = i;

    intvec = Malloc (i * sizeof (int));

    for (j = 0; j < i; j++) {
        intvec[j] = BOOL_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (intvec);
}

char *
Array2CharVec (node *aelems, int *length)
{
    char *charvec;
    int i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("Array2CharVec");

    while (aelems != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_char),
                     "constant integer array exspected !");
        aelems = EXPRS_NEXT (aelems);
        i++;
    }
    /*
     *  if the length of the vector is not of interrest length may be NULL
     */
    if (length != NULL)
        *length = i;

    charvec = Malloc (i * sizeof (char));

    for (j = 0; j < i; j++) {
        charvec[j] = CHAR_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (charvec);
}

float *
Array2FloatVec (node *aelems, int *length)
{
    float *floatvec;
    int i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("Array2FloatVec");

    while (aelems != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_float),
                     "constant float array exspected !");
        aelems = EXPRS_NEXT (aelems);
        i++;
    }
    /*
     *  if the length of the vector is not of interrest length may be NULL
     */
    if (length != NULL)
        *length = i;

    floatvec = Malloc (i * sizeof (float));

    for (j = 0; j < i; j++) {
        floatvec[j] = FLOAT_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (floatvec);
}

double *
Array2DblVec (node *aelems, int *length)
{
    int i = 0, j;
    double *dblvec;
    node *tmp = aelems;

    DBUG_ENTER ("Array2DblVec");

    while (aelems != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_double),
                     "constant double array exspected !");
        aelems = EXPRS_NEXT (aelems);
        i++;
    }
    /*
     *  if the length of the vector is not of interrest length may be NULL
     */
    if (length != NULL)
        *length = i;

    dblvec = Malloc (i * sizeof (double));

    for (j = 0; j < i; j++) {
        dblvec[j] = DOUBLE_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (dblvec);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***/

/******************************************************************************
 *
 * function:
 *   node *MakeVinfoDollar( node * next)
 *
 * description:
 *   create N_vinfo node with Dollar-Tag and self-ref in VINFO_DOLLAR!
 *
 ******************************************************************************/

node *
MakeVinfoDollar (node *next)
{
    node *vinfo;

    DBUG_ENTER ("MakeVinfoDollar");

    vinfo = MakeVinfo (DOLLAR, NULL, next, NULL);
    VINFO_DOLLAR (vinfo) = vinfo;

    DBUG_RETURN (vinfo);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***/

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***/

static node *
CombineExprs (node *first, node *second)
{
    node *result;

    DBUG_ENTER ("CombineExprs");

    if (NODE_TYPE (first) != N_exprs) {
        result = MakeExprs (first, second);
    } else {
        result = AppendExprs (first, second);
    }

    DBUG_RETURN (result);
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

    arg1 = CombineExprs (arg1, NULL);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm2 (char *name, node *arg1, node *arg2)
{
    node *icm;

    DBUG_ENTER ("MakeIcm2");

    arg2 = CombineExprs (arg2, NULL);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm3 (char *name, node *arg1, node *arg2, node *arg3)
{
    node *icm;

    DBUG_ENTER ("MakeIcm3");

    arg3 = CombineExprs (arg3, NULL);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *icm;

    DBUG_ENTER ("MakeIcm4");

    arg4 = CombineExprs (arg4, NULL);
    arg3 = CombineExprs (arg3, arg4);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    node *icm;

    DBUG_ENTER ("MakeIcm5");

    arg5 = CombineExprs (arg5, NULL);
    arg4 = CombineExprs (arg4, arg5);
    arg3 = CombineExprs (arg3, arg4);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
          node *arg6)
{
    node *icm;

    DBUG_ENTER ("MakeIcm6");

    arg6 = CombineExprs (arg6, NULL);
    arg5 = CombineExprs (arg5, arg6);
    arg4 = CombineExprs (arg4, arg5);
    arg3 = CombineExprs (arg3, arg4);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
          node *arg6, node *arg7)
{
    node *icm;

    DBUG_ENTER ("MakeIcm6");

    arg7 = CombineExprs (arg7, NULL);
    arg6 = CombineExprs (arg6, arg7);
    arg5 = CombineExprs (arg5, arg6);
    arg4 = CombineExprs (arg4, arg5);
    arg3 = CombineExprs (arg3, arg4);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1, NULL);

    DBUG_RETURN (icm);
}
