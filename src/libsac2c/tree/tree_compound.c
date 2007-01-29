/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "shape.h"
#include "dbug.h"
#include "free.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "ctinfo.h"
#include "DataFlowMask.h"
#include "wltransform.h"
#include "globals.h"
#include "NameTuplesUtils.h"
#include "type_utils.h"
#include "internal_lib.h"
#include "namespaces.h"

/*
 * macro template for append functions
 */
#define APPEND(ret, type, atype, chain, item)                                            \
    if (item != NULL) {                                                                  \
        if (chain == NULL) {                                                             \
            chain = item;                                                                \
        } else {                                                                         \
            type __tmp;                                                                  \
            __tmp = chain;                                                               \
            while (atype##_NEXT (__tmp) != NULL) {                                       \
                __tmp = atype##_NEXT (__tmp);                                            \
            }                                                                            \
            atype##_NEXT (__tmp) = item;                                                 \
        }                                                                                \
    }                                                                                    \
    ret = chain

/*--------------------------------------------------------------------------*/
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***/

/******************************************************************************
 *
 * Function:
 *   int TCgetShpsegLength( int dims, shpseg *shape)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
TCgetShpsegLength (int dims, shpseg *shape)
{
    int length, i;

    DBUG_ENTER ("TCgetShpsegLength");

    if (dims > 0) {
        length = 1;
        for (i = 0; i < dims; i++) {
            length *= SHPSEG_SHAPE (shape, i);
        }
    } else if (dims == 0) {
        length = 0;
    } else {
        length = (-1);
    }

    DBUG_RETURN (length);
}

/*****************************************************************************
 *
 * function:
 *   shpseg *TCdiffShpseg( int dim, shpseg *shape1, shpseg *shape2)
 *
 * description:
 *   calculate shape1 - shape2
 *
 *****************************************************************************/

shpseg *
TCdiffShpseg (int dim, shpseg *shape1, shpseg *shape2)
{

    shpseg *shape_diff;
    int i;

    DBUG_ENTER ("TCdiffShpseg");

    shape_diff = TBmakeShpseg (NULL);

    for (i = 0; i < dim; i++) {
        SHPSEG_SHAPE (shape_diff, i)
          = SHPSEG_SHAPE (shape1, i) - SHPSEG_SHAPE (shape2, i);
    }

    DBUG_RETURN (shape_diff);
}

/*****************************************************************************
 *
 * function:
 *   bool TCshapeVarsMatch( node *avis1, node *avis2)
 *
 * description:
 *   Compares the shapes of two arrays, based on their N_avis SAA shapes.
 *
 *
 *****************************************************************************/

bool
TCshapeVarsMatch (node *avis1, node *avis2)
{
    bool res;

    DBUG_ENTER ("TCshapeVarsMatch");

    res = ((TYeqTypes (TYgetScalar (AVIS_TYPE (avis1)), TYgetScalar (AVIS_TYPE (avis2))))
           && (AVIS_DIM (avis1) != NULL) && (AVIS_DIM (avis2) != NULL)
           && (AVIS_SHAPE (avis1) != NULL) && (AVIS_SHAPE (avis2) != NULL)
           && (CMPTdoCompareTree (AVIS_DIM (avis1), AVIS_DIM (avis2)) == CMPT_EQ)
           && (CMPTdoCompareTree (AVIS_SHAPE (avis1), AVIS_SHAPE (avis2)) == CMPT_EQ));

    DBUG_RETURN (res);
}

/*****************************************************************************
 *
 * function:
 *   bool TCequalShpseg( int dim, shpseg *shape2, shpseg *shape1)
 *
 * description:
 *   compares two shapes, result is TRUE, if shapes are equal
 *
 *****************************************************************************/

bool
TCequalShpseg (int dim, shpseg *shape2, shpseg *shape1)
{

    bool equal_shapes;
    int i;

    DBUG_ENTER ("TCequalShpseg");

    equal_shapes = TRUE;

    i = 0;
    while (i < dim && equal_shapes) {
        if (SHPSEG_SHAPE (shape1, i) != SHPSEG_SHAPE (shape2, i)) {
            equal_shapes = FALSE;
        }
        i++;
    }

    DBUG_RETURN (equal_shapes);
}

shpseg *
TCmergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2)
{
    shpseg *new;
    int i;

    DBUG_ENTER ("TCmergeShpseg");

    new = TBmakeShpseg (NULL);

    for (i = 0; i < dim1; i++) {
        SHPSEG_SHAPE (new, i) = SHPSEG_SHAPE (first, i);
    }

    for (i = 0; i < dim2; i++) {
        SHPSEG_SHAPE (new, i + dim1) = SHPSEG_SHAPE (second, i);
    }

    DBUG_RETURN (new);
}

/*****************************************************************************
 *
 * function:
 *   shpseg *TCarray2Shpseg( node *array, int *ret_dim)
 *
 * description:
 *   Convert 'array' into a shpseg (requires int-array!!!).
 *   If 'dim' is != NULL the dimensionality is stored there.
 *
 *****************************************************************************/

shpseg *
TCarray2Shpseg (node *array, int *ret_dim)
{

    node *tmp;
    shpseg *shape;
    int i;

    DBUG_ENTER ("TCarray2Shpseg");

    shape = TBmakeShpseg (NULL);

    tmp = ARRAY_AELEMS (array);
    i = 0;
    while (tmp != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (tmp)) == N_num), "integer array expected!");
        SHPSEG_SHAPE (shape, i) = NUM_VAL (EXPRS_EXPR (tmp));
        i++;
        tmp = EXPRS_NEXT (tmp);
    }

    if (ret_dim != NULL) {
        (*ret_dim) = i;
    }

    DBUG_RETURN (shape);
}

/*****************************************************************************
 *
 * function:
 *   node *TCshpseg2Array( shpseg *shape, int dim)
 *
 * description:
 *   convert shpseg with given dimension into array with simpletype T_int
 *
 *****************************************************************************/

node *
TCshpseg2Array (shpseg *shape, int dim)
{
    int i;
    node *next;
    node *array_node;

    DBUG_ENTER ("TCshpseg2Array");

    next = NULL;
    for (i = dim - 1; i >= 0; i--) {
        next = TBmakeExprs (TBmakeNum (SHPSEG_SHAPE (shape, i)), next);
    }

    array_node = TCmakeIntVector (next);

    DBUG_RETURN (array_node);
}

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***/

/******************************************************************************
 *
 * function:
 *   types *TCappendTypes( types *chain, types *item)
 *
 * description:
 *   append item to list of types
 *
 ******************************************************************************/

types *
TCappendTypes (types *chain, types *item)
{
    types *ret;

    DBUG_ENTER ("TCappendTypes");

    APPEND (ret, types *, TYPES, chain, item);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   int TCcountTypes( types *type)
 *
 * description:
 *   Counts the number of types.
 *
 ******************************************************************************/

int
TCcountTypes (types *type)
{
    int count = 0;

    DBUG_ENTER ("TCcountTypes");

    while (type != NULL) {
        if (TYPES_BASETYPE (type) != T_void) {
            count++;
        }
        type = TYPES_NEXT (type);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * Function:
 *   type *TCgetTypesLine( types* type, int line)
 *
 * Description:
 *   line > 0:  generate an error message if error occurs.
 *   otherwise: DBUG assert.
 *
 ******************************************************************************/

types *
TCgetTypesLine (types *type, int line)
{
    node *tdef;
    types *res_type = NULL;

    DBUG_ENTER ("TCgetTypesLine");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);

        if ((tdef == NULL) && (global.compiler_phase <= PH_tc)) {
            tdef = NULL;
        }

        if (line > 0) {
            if (tdef == NULL) {
                CTIabortLine (line, "Type '%s:%s' is unknown", TYPES_MOD (type),
                              TYPES_NAME (type));
            }
        } else {
            DBUG_ASSERT ((tdef != NULL), "typedef not found!");
        }

        /*
         * we have to switch to ntypes here, as the typedefs do
         * not hold any old types anymore. maybe the backend
         * will move to newtypes one day so this hybrid function
         * can be removed.
         */

        if (TUisHidden (TYPEDEF_NTYPE (tdef))) {
            /*
             * Basic type is hidden therefore we have to use the original type
             * structure and rely on the belonging typedef!!
             */
            res_type = type;
        } else {
            /*
             * ok, we can resolve the type to the corresponding basetype
             */
            res_type = TYtype2OldType (TYPEDEF_NTYPE (tdef));
        }
    } else {
        res_type = type;
    }

    DBUG_RETURN (res_type);
}

/******************************************************************************
 *
 * Function:
 *   type *TCgetTypes( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

types *
TCgetTypes (types *type)
{
    types *res_type;

    DBUG_ENTER ("TCgetTypes");

    res_type = TCgetTypesLine (type, 0);

    DBUG_RETURN (res_type);
}

/******************************************************************************
 *
 * Function:
 *   int TCgetShapeDim( types* type)
 *
 * Description:
 *   returns encoded dimension (DIM) of 'type'
 *   ('encoded' means, that it contains also some shape information):
 *     >= 0 : dimension == DIM             and    known shape
 *     <  -2: dimension == -2 - DIM        and  unknown shape
 *     == -1: dimension unknown (but > 0)
 *     == -2: dimension unknown (>= 0)
 *
 ******************************************************************************/

int
TCgetShapeDim (types *type)
{
    types *impl_type;
    int dim, base_dim, impl_dim;

    DBUG_ENTER ("TCgetShapeDim");

    base_dim = TYPES_DIM (type);

    impl_type = TCgetTypes (type);

    if (impl_type != type) {
        /*
         * user-defined type
         */
        impl_dim = TYPES_DIM (impl_type);

        if ((UNKNOWN_SHAPE == impl_dim) || (UNKNOWN_SHAPE == base_dim)) {
            dim = UNKNOWN_SHAPE;
        } else if ((ARRAY_OR_SCALAR == impl_dim) && (ARRAY_OR_SCALAR == base_dim)) {
            dim = ARRAY_OR_SCALAR;
        } else if ((ARRAY_OR_SCALAR == impl_dim) && (SCALAR == base_dim)) {
            dim = ARRAY_OR_SCALAR;
        } else if ((SCALAR == impl_dim) && (ARRAY_OR_SCALAR == base_dim)) {
            dim = ARRAY_OR_SCALAR;
        } else if ((ARRAY_OR_SCALAR == impl_dim) || (ARRAY_OR_SCALAR == base_dim)) {
            dim = UNKNOWN_SHAPE;
        } else if (KNOWN_SHAPE (impl_dim) && KNOWN_SHAPE (base_dim)) {
            dim = impl_dim + base_dim;
        } else if (KNOWN_SHAPE (impl_dim) && KNOWN_DIMENSION (base_dim)) {
            dim = base_dim - impl_dim;
        } else if (KNOWN_DIMENSION (impl_dim) && KNOWN_SHAPE (base_dim)) {
            dim = impl_dim - base_dim;
        } else if (KNOWN_DIMENSION (impl_dim) && KNOWN_DIMENSION (base_dim)) {
            dim = impl_dim + base_dim - KNOWN_DIM_OFFSET;
        } else {
            dim = 0;
            DBUG_ASSERT ((0), "illegal shape/dim information found!");
        }
    } else {
        /*
         * basic type
         */
        dim = base_dim;
    }

    DBUG_RETURN (dim);
}

/******************************************************************************
 *
 * Function:
 *   int TCgetDim( types* type)
 *
 * Description:
 *   returns dimension of 'type':
 *     >= 0 : dimension known
 *     == -1: dimension unknown (but > 0)
 *     == -2: dimension unknown (>= 0)
 *
 ******************************************************************************/

int
TCgetDim (types *type)
{
    int dim;

    DBUG_ENTER ("TCgetDim");

    dim = TCgetShapeDim (type);
    dim = DIM_NO_OFFSET (dim);

    DBUG_RETURN (dim);
}

/******************************************************************************
 *
 * Function:
 *   simpletype TCgetBasetype( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

simpletype
TCgetBasetype (types *type)
{
    simpletype res;

    DBUG_ENTER ("TCgetBasetype");

    res = TYPES_BASETYPE (TCgetTypes (type));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   int TCgetBasetypeSize(types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
TCgetBasetypeSize (types *type)
{
    int size;

    DBUG_ENTER ("TCgetBasetypeSize");

    size = global.basetype_size[TCgetBasetype (type)];

    DBUG_RETURN (size);
}

/******************************************************************************
 *
 * Function:
 *   int GetTypesLength( types *type)
 *
 * Description:
 *   If 'type' is an array type the number of array elements is returned.
 *   Otherwise the return value is 0.
 *
 ******************************************************************************/

int
TCgetTypesLength (types *type)
{
    shpseg *shape;
    int dim, length;

    DBUG_ENTER ("TCgetTypesLength");

    shape = TCtype2Shpseg (type, &dim);

    length = TCgetShpsegLength (dim, shape);

    if (shape != NULL) {
        shape = FREEfreeShpseg (shape);
    }

    DBUG_RETURN (length);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *TCtype2Shpseg( types *type, int *ret_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
TCtype2Shpseg (types *type, int *ret_dim)
{
    int dim, base_dim, i;
    types *impl_type;
    shpseg *new_shpseg = NULL;

    DBUG_ENTER ("TCtype2Shpseg");

    dim = TCgetShapeDim (type);

    DBUG_ASSERT ((dim < SHP_SEG_SIZE), "shape is out of range");

    if (dim > SCALAR) {
        new_shpseg = TBmakeShpseg (NULL);
        impl_type = TCgetTypes (type);

        base_dim = TYPES_DIM (type);
        for (i = 0; i < base_dim; i++) {
            SHPSEG_SHAPE (new_shpseg, i) = TYPES_SHAPE (type, i);
        }

        if (impl_type != type) {
            /*
             * user-defined type
             */
            for (i = 0; i < TYPES_DIM (impl_type); i++) {
                SHPSEG_SHAPE (new_shpseg, base_dim + i) = TYPES_SHAPE (impl_type, i);
            }
        }
    } else {
        new_shpseg = NULL;
    }

    if (ret_dim != NULL) {
        (*ret_dim) = dim;
    }

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   shape *TCtype2Shape( types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

shape *
TCtype2Shape (types *type)
{
    shape *shp = NULL;
    shpseg *new_shpseg = NULL;
    int dim;

    DBUG_ENTER ("TCtype2Shape");

    dim = TCgetShapeDim (type);
    new_shpseg = TCtype2Shpseg (type, NULL);

    if (new_shpseg != NULL) {
        shp = SHoldShpseg2Shape (dim, new_shpseg);
        new_shpseg = ILIBfree (new_shpseg);
    } else {
        DBUG_ASSERT (dim <= 0, "shape inconsistency");
    }

    DBUG_RETURN (shp);
}

/******************************************************************************
 *
 * Function:
 *   node *TCtype2Exprs( types *type)
 *
 * Description:
 *   Computes the shape of corresponding type and stores it as N_exprs chain.
 *
 ******************************************************************************/

node *
TCtype2Exprs (types *type)
{
    node *tmp;
    types *impl_type;
    int dim, i;
    node *ret_node = NULL;

    DBUG_ENTER ("TCtype2Exprs");

    /* create a dummy node to append the shape items to */
    ret_node = TBmakeExprs (NULL, NULL);

    dim = TCgetShapeDim (type);

    if (dim > SCALAR) {
        tmp = ret_node;
        impl_type = TCgetTypes (type);

        for (i = 0; i < TYPES_DIM (type); i++) {
            EXPRS_NEXT (tmp) = TBmakeExprs (TBmakeNum (TYPES_SHAPE (type, i)), NULL);
            tmp = EXPRS_NEXT (tmp);
        }

        if (impl_type != type) {
            /*
             * user-defined type
             */
            for (i = 0; i < TYPES_DIM (impl_type); i++) {
                EXPRS_NEXT (tmp)
                  = TBmakeExprs (TBmakeNum (TYPES_SHAPE (impl_type, i)), NULL);
                tmp = EXPRS_NEXT (tmp);
            }
        }
    }

    /* remove dummy node at head of chain */
    ret_node = FREEdoFreeNode (ret_node);

    DBUG_RETURN (ret_node);
}

bool
TCisHidden (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsHidden");

    if (TYPES_BASETYPE (type) == T_hidden) {
        ret = TRUE;
    } else if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        /*
         * we have to move to the new types here, as the typedef does
         * not hold any old types anymore
         */
        if (TYisSimple (TYgetScalar (TYPEDEF_NTYPE (tdef)))) {
            ret = (TYgetSimpleType (TYgetScalar (TYPEDEF_NTYPE (tdef))) == T_hidden);
        }
    }

    DBUG_RETURN (ret);
}

bool
TCisUnique (types *type)
{
    bool ret = FALSE;

    DBUG_ENTER ("TCisUnique");

    if (TYPES_BASETYPE (type) == T_user) {
        ret = TUisUniqueUserType (TYoldType2Type (type));
    }

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***/

node *
TCappendIds (node *chain, node *item)
{
    node *ret;

    DBUG_ENTER ("TCappendIds");

    APPEND (ret, node *, IDS, chain, item);

    DBUG_RETURN (ret);
}

node *
TClookupIds (const char *name, node *ids_chain)
{
    DBUG_ENTER ("TClookupIds");

    while ((ids_chain != NULL) && (!ILIBstringCompare (name, IDS_NAME (ids_chain)))) {
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (ids_chain);
}

int
TCcountIds (node *ids_arg)
{
    int count = 0;

    DBUG_ENTER ("TCcountIds");

    while (ids_arg != NULL) {
        count++;
        ids_arg = IDS_NEXT (ids_arg);
    }

    DBUG_RETURN (count);
}

node *
TCmakeIdsFromVardecs (node *vardecs)
{
    node *ids = NULL;
    DBUG_ENTER ("TCmakeIdsFromVardecs");
    while (vardecs != NULL) {
        ids = TBmakeIds (VARDEC_AVIS (vardecs), ids);
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_RETURN (ids);
}

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
 ***/

int
TCcountNums (node *nums)
{
    int cnt = 0;

    DBUG_ENTER ("TCcountNums");

    while (nums != NULL) {
        cnt++;
        nums = NUMS_NEXT (nums);
    }

    DBUG_RETURN (cnt);
}

bool
TCnumsContains (int val, node *nums)
{
    bool result = FALSE;

    DBUG_ENTER ("TCnumsContains");

    while ((nums != NULL) && (!result)) {
        result = (NUMS_VAL (nums) == val);
        nums = NUMS_NEXT (nums);
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***/

/******************************************************************************
 *
 * function:
 *   nodelist *TCnodeListAppend(nodelist *nl, node *newnode, void *attrib)
 *   nodelist *TCnodeListDelete(nodelist *nl, node *node, bool free_attrib)
 *   nodelist *TcnodeListFree(nodelist *nl, bool free_attrib)
 *   nodelist *TCnodeListFind(nodelist *nl, node *node)
 *
 * description:
 *   the following functions realize basic functions on pure node lists.
 *
 *   Append: appends a node to the given list, returning a new list.
 *           Since the node list has no special order, the new node is
 *           not appended but put in front of the given list to speed
 *           up execution.
 *           Create a list: newlist = Append(NULL, newnode, attrib);
 *   Delete: deletes all elements of the given node. If free_attrib is FALSE,
 *           the attribut is not set free, else a FREE(attrib) is executed.
 *   Free  : frees whole list. If free_attrib is FALSE, the attributes are
 *           not set free, else a FREE(attrib) is executed.
 *   Find  : returns the nodelist node of the first found item
 *           with fitting node. If not found, returns NULL.
 *
 ******************************************************************************/

nodelist *
TCnodeListAppend (nodelist *nl, node *newnode, void *attrib)
{
    DBUG_ENTER ("TCnodeListAppend");

    nl = TBmakeNodelistNode (newnode, nl);
    NODELIST_ATTRIB2 (nl) = attrib;

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListDelete (nodelist *nl, node *node, bool free_attrib)
{
    nodelist *tmpnl, *prevnl;

    DBUG_ENTER ("TCnodeListDelete");

    while (nl && NODELIST_NODE (nl) == node) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            NODELIST_ATTRIB2 (nl) = ILIBfree (NODELIST_ATTRIB2 (nl));
        }
        nl = FREEfreeNodelistNode (nl);
    }

    tmpnl = nl;
    prevnl = NULL;
    while (tmpnl) {
        if (NODELIST_NODE (tmpnl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (tmpnl)) {
                NODELIST_ATTRIB2 (tmpnl) = ILIBfree (NODELIST_ATTRIB2 (tmpnl));
            }

            NODELIST_NEXT (prevnl) = FREEfreeNodelistNode (tmpnl);
        } else {
            prevnl = tmpnl;
        }

        tmpnl = NODELIST_NEXT (prevnl);
    }

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListFree (nodelist *nl, bool free_attrib)
{
    DBUG_ENTER ("TCnodeListFree");

    while (nl) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            NODELIST_ATTRIB2 (nl) = ILIBfree (NODELIST_ATTRIB2 (nl));
        }
        nl = FREEfreeNodelistNode (nl);
    }

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListFind (nodelist *nl, node *node)
{
    DBUG_ENTER ("TCnodeListFind");

    while (nl && NODELIST_NODE (nl) != node) {
        nl = NODELIST_NEXT (nl);
    }

    DBUG_RETURN (nl);
}

/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  N_modul :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_classdec :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_sib :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_implist :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_explist :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_typedef :
 ***/

node *
TCsearchTypedef (const char *name, const namespace_t *ns, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("TCsearchTypedef");

    tmp = implementations;

    while ((tmp != NULL)
           && (!ILIBstringCompare (name, TYPEDEF_NAME (tmp))
               || !NSequals (ns, TYPEDEF_NS (tmp)))) {
        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
}

node *
TCappendTypedef (node *tdef_chain, node *tdef)
{
    node *ret;

    DBUG_ENTER ("TCappendTypedef");

    DBUG_ASSERT (((tdef_chain == NULL) || (NODE_TYPE (tdef_chain) == N_typedef)),
                 ("First argument of TCappendTypedef() has wrong node type."));
    DBUG_ASSERT (((tdef == NULL) || (NODE_TYPE (tdef) == N_typedef)),
                 ("Second argument of TCappendTypedef() has wrong node type."));

    APPEND (ret, node *, TYPEDEF, tdef_chain, tdef);

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***/

node *
TCappendObjdef (node *objdef_chain, node *objdef)
{
    node *ret;

    DBUG_ENTER ("TCappendObjdef");

    DBUG_ASSERT (((objdef_chain == NULL) || (NODE_TYPE (objdef_chain) == N_objdef)),
                 ("First argument of AppendObjdef() has wrong node type."));
    DBUG_ASSERT (((objdef == NULL) || (NODE_TYPE (objdef) == N_objdef)),
                 ("Second argument of AppendObjdef() has wrong node type."));

    APPEND (ret, node *, OBJDEF, objdef_chain, objdef);

    DBUG_RETURN (ret);
}

node *
TCunAliasObjdef (node *objdef)
{
    node *result;

    DBUG_ENTER ("TCunAliasObjdef");

    result = objdef;

    while (OBJDEF_ISALIAS (result)) {
        DBUG_ASSERT ((NODE_TYPE (OBJDEF_EXPR (result)) == N_globobj),
                     "found objdef alias without proper init expression!");

        result = GLOBOBJ_OBJDEF (OBJDEF_EXPR (result));
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***/

node *
TCappendFundef (node *fundef_chain, node *fundef)
{
    node *ret;

    DBUG_ENTER ("TCappendFundef");

    DBUG_ASSERT (((fundef_chain == NULL) || (NODE_TYPE (fundef_chain) == N_fundef)),
                 ("First argument of TCappendFundef() has wrong node type."));
    DBUG_ASSERT (((fundef == NULL) || (NODE_TYPE (fundef) == N_fundef)),
                 ("Second argument of TCappendFundef() has wrong node type."));

    APPEND (ret, node *, FUNDEF, fundef_chain, fundef);

    DBUG_RETURN (ret);
}

node *
TCremoveFundef (node *fundef_chain, node *fundef)
{
    node *pos;
    DBUG_ENTER ("TCremoveFundef");

    if (fundef_chain == fundef) {
        fundef_chain = FUNDEF_NEXT (fundef_chain);
    } else {
        pos = fundef_chain;

        while ((FUNDEF_NEXT (pos) != NULL) && (FUNDEF_NEXT (pos) != fundef)) {
            pos = FUNDEF_NEXT (pos);
        }

        if (FUNDEF_NEXT (pos) == fundef) {
            FUNDEF_NEXT (pos) = FUNDEF_NEXT (fundef);
        }
    }

    FUNDEF_NEXT (fundef) = NULL;

    DBUG_RETURN (fundef_chain);
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
 * Function:
 *   node *TCaddVardecs( node *fundef, node *vardecs)
 *
 * Description:
 *   Inserts new declarations into the AST, updates the DFMbase and returns
 *   the modified N_fundef node.
 *
 ******************************************************************************/

node *
TCaddVardecs (node *fundef, node *vardecs)
{
    DBUG_ENTER ("TCaddVardecs");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no N_fundef node found!");

    /*
     * insert new vardecs into AST
     */
    FUNDEF_VARDEC (fundef) = TCappendVardec (vardecs, FUNDEF_VARDEC (fundef));

    /*
     * we must update FUNDEF_DFM_BASE!!
     */
    if (FUNDEF_DFM_BASE (fundef) != NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMupdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                               FUNDEF_VARDEC (fundef));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 ******************************************************************************/

node *
TCappendVardec (node *vardec_chain, node *vardec)
{
    node *ret;

    DBUG_ENTER ("TCappendVardec");

    DBUG_ASSERT (((vardec_chain == NULL) || (NODE_TYPE (vardec_chain) == N_vardec)),
                 ("First argument of AppendVardec() has wrong node type."));
    DBUG_ASSERT (((vardec == NULL) || (NODE_TYPE (vardec) == N_vardec)),
                 ("Second argument of AppendVardec() has wrong node type."));

    APPEND (ret, node *, VARDEC, vardec_chain, vardec);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   int TCcountVardecs( node *vardecs)
 *
 * description:
 *   Counts the number of N_vardec nodes.
 *
 ******************************************************************************/

int
TCcountVardecs (node *vardecs)
{
    int count = 0;

    DBUG_ENTER ("TCcountVardecs");

    while (vardecs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (vardecs) == N_vardec), "no N_vardec node found!");
        count++;
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN (count);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/******************************************************************************
 *
 * function:
 *   int TCcountArgs( node *args)
 *
 * description:
 *   Counts the number of N_arg nodes.
 *
 ******************************************************************************/

int
TCcountArgs (node *args)
{
    int count = 0;

    DBUG_ENTER ("TCcountArgs");

    while (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_arg node found!");
        count++;
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendArgs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_arg chains of nodes.
 *
 ******************************************************************************/

node *
TCappendArgs (node *arg_chain, node *arg)
{
    node *ret;

    DBUG_ENTER ("TCappendExprs");

    DBUG_ASSERT (((arg_chain == NULL) || (NODE_TYPE (arg_chain) == N_arg)),
                 ("First argument of TCappendArgs() has wrong node type."));
    DBUG_ASSERT (((arg == NULL) || (NODE_TYPE (arg) == N_arg)),
                 ("Second argument of TCappendArgs() has wrong node type."));

    APPEND (ret, node *, ARG, arg_chain, arg);

    DBUG_RETURN (ret);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCmakeExprsFromArgs( node *args)
 *
 * @brief Returns an N_exprs chain containing N_id nodes with
 *        the avis given by the given args.
 *
 * @param args N_arg chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCmakeExprsFromArgs (node *args)
{
    node *result;

    DBUG_ENTER ("TCmakeExprsFromArgs");

    if (args != NULL) {
        result = TBmakeExprs (TBmakeId (ARG_AVIS (args)),
                              TCmakeExprsFromArgs (ARG_NEXT (args)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ret :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCappendRet( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_ret chains of nodes.
 *
 ******************************************************************************/

node *
TCappendRet (node *ret_chain, node *item)
{
    node *ret;

    DBUG_ENTER ("TCappendRet");

    DBUG_ASSERT (((ret_chain == NULL) || (NODE_TYPE (ret_chain) == N_ret)),
                 ("First argument of TCappendRet() has wrong node type."));
    DBUG_ASSERT (((item == NULL) || (NODE_TYPE (item) == N_ret)),
                 ("Second argument of TCappendRet() has wrong node type."));

    APPEND (ret, node *, RET, ret_chain, item);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   int TCcountRets( node *rets)
 *
 * description:
 *   Counts the number of N_arg nodes.
 *
 ******************************************************************************/

int
TCcountRets (node *rets)
{
    int count = 0;

    DBUG_ENTER ("TCcountRets");

    while (rets != NULL) {
        DBUG_ASSERT ((NODE_TYPE (rets) == N_ret), "no N_ret node found!");
        count++;
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (count);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateIdsFromRets( node *rets, node **vardecs)
 *
 * @brief Creates a N_ids chain with the types given by the N_rets chain.
 *        Vardecs for the created N_ids are appended to the given vardecs
 *        chain.
 *
 * @param rets N_rets chain
 * @param vardecs address of a N_vardec chain
 *
 * @return created N_ids chain
 ******************************************************************************/
node *
TCcreateIdsFromRets (node *rets, node **vardecs)
{
    node *vardec;
    node *result;

    DBUG_ENTER ("TCcreateIdsFromRets");

    if (rets != NULL) {
        vardec
          = TBmakeVardec (TBmakeAvis (ILIBtmpVar (), TYcopyType (RET_TYPE (rets))), NULL);
        result = TBmakeIds (VARDEC_AVIS (vardec),
                            TCcreateIdsFromRets (RET_NEXT (rets), vardecs));

        *vardecs = TCappendVardec (vardec, *vardecs);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :  *and*  N_arg :
 ***/

/*
 *
 *  functionname  : SearchDecl
 *  arguments     : 1) character string
 *                  2) N_vardec - or N_arg - chain
 *                  R) true or false
 *  description   : returns ptr to N_vardec - or N_arg - node  if variable or
 *                  argument with same name as in 1) has been found, else NULL
 *
 */

node *
TCsearchDecl (const char *name, node *decl_node)
{
    node *found = NULL;

    DBUG_ENTER ("TCsearchDecl");

    while (NULL != decl_node) {
        if (N_vardec == NODE_TYPE (decl_node)) {
            if (!ILIBstringCompare (name, VARDEC_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = VARDEC_NEXT (decl_node);
            }
        } else {
            if (!ILIBstringCompare (name, ARG_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = ARG_NEXT (decl_node);
            }
        }
    }

    DBUG_RETURN (found);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***/

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/
static node *
TCmakeAssignInstr (node *instr, node *next)
{
    node *result;

    DBUG_ENTER ("TCmakeAssignInstr");

    if (instr == NULL) {
        result = next;
    } else if (NODE_TYPE (instr) == N_assign) {
        result = TCappendAssign (instr, next);
    } else {
        result = TBmakeAssign (instr, next);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn int TCcountAssigns( node *arg_node)
 *
 *****************************************************************************/
int
TCcountAssigns (node *arg_node)
{
    int res = 0;

    DBUG_ENTER ("TCcountAssigns");

    while (arg_node != NULL) {
        res += 1;
        arg_node = ASSIGN_NEXT (arg_node);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssign?( node *part?, ...)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCmakeAssigns1 (node *part1)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns1");

    assigns = TCmakeAssignInstr (part1, NULL);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns2 (node *part1, node *part2)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns2");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns1 (part2));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns3 (node *part1, node *part2, node *part3)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns3");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns2 (part2, part3));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns4 (node *part1, node *part2, node *part3, node *part4)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns4");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns3 (part2, part3, part4));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns5 (node *part1, node *part2, node *part3, node *part4, node *part5)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns5");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns4 (part2, part3, part4, part5));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns6 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns6");

    assigns
      = TCmakeAssignInstr (part1, TCmakeAssigns5 (part2, part3, part4, part5, part6));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns7 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns7");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns6 (part2, part3, part4, part5, part6,
                                                        part7));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns8 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7, node *part8)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns8");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns7 (part2, part3, part4, part5, part6,
                                                        part7, part8));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns9 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7, node *part8, node *part9)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssigns9");

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns8 (part2, part3, part4, part5, part6,
                                                        part7, part8, part9));

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeAssignIcm0( char *name, node *next)
 *   node *TCmakeAssignIcm?( char *name, node *arg?, ..., node *next)
 *
 * description:
 *   These functions generate an N_assign node with a complete ICM
 *   representations including arguments as body.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *   The ASSIGN_NEXT will be NULL!
 *
 ******************************************************************************/

node *
TCmakeAssignIcm0 (char *name, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm0");

    assigns = TBmakeAssign (TCmakeIcm0 (name), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm1 (char *name, node *arg1, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm1");

    assigns = TBmakeAssign (TCmakeIcm1 (name, arg1), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm2 (char *name, node *arg1, node *arg2, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm2");

    assigns = TBmakeAssign (TCmakeIcm2 (name, arg1, arg2), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm3");

    assigns = TBmakeAssign (TCmakeIcm3 (name, arg1, arg2, arg3), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm4");

    assigns = TBmakeAssign (TCmakeIcm4 (name, arg1, arg2, arg3, arg4), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                  node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm5");

    assigns = TBmakeAssign (TCmakeIcm5 (name, arg1, arg2, arg3, arg4, arg5), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                  node *arg6, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm6");

    assigns = TBmakeAssign (TCmakeIcm6 (name, arg1, arg2, arg3, arg4, arg5, arg6), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                  node *arg6, node *arg7, node *next)
{
    node *assigns;

    DBUG_ENTER ("TCmakeAssignIcm7");

    assigns
      = TBmakeAssign (TCmakeIcm7 (name, arg1, arg2, arg3, arg4, arg5, arg6, arg7), next);

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *TCgetCompoundNode( node *arg_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCgetCompoundNode (node *arg_node)
{
    node *compound_node;

    DBUG_ENTER ("TCgetCompoundNode");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "TCgetCompoundNode() can handle N_assign nodes only!");

    arg_node = ASSIGN_INSTR (arg_node);
    switch (NODE_TYPE (arg_node)) {
    case N_cond:
        /* here is no break missing */
    case N_do:
        /* here is no break missing */
    case N_while:
        compound_node = arg_node;
        break;

    case N_let:
        arg_node = LET_EXPR (arg_node);
        while (N_cast == NODE_TYPE (arg_node)) {
            arg_node = CAST_EXPR (arg_node);
        }
        if (N_with == NODE_TYPE (arg_node)) {
            compound_node = arg_node;
        } else {
            compound_node = NULL;
        }
        break;

    default:
        compound_node = NULL;
    }

    DBUG_RETURN (compound_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendAssign( node *assign_chain, node *assign)
 *
 * description:
 *   Appends 'assign' to 'assing_chain' and returns the new chain.
 *   If 'assign_chain' was a N_empty node, this node is removed first.
 *
 ******************************************************************************/

node *
TCappendAssign (node *assign_chain, node *assign)
{
    node *ret;

    DBUG_ENTER ("TCappendAssign");

    DBUG_ASSERT (((assign_chain == NULL) || (NODE_TYPE (assign_chain) == N_assign)
                  || (NODE_TYPE (assign_chain) == N_empty)),
                 ("First argument of TCappendAssign() has wrong node type."));
    DBUG_ASSERT (((assign == NULL) || (NODE_TYPE (assign) == N_assign)),
                 ("Second argument of TCappendAssign() has wrong node type."));

    if ((assign_chain != NULL) && (NODE_TYPE (assign_chain) == N_empty)) {
        assign_chain = FREEdoFreeNode (assign_chain);
    }

    APPEND (ret, node *, ASSIGN, assign_chain, assign);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends an new ICM with name and args given as an assign to the given
 *   chain of assignments.
 *
 ******************************************************************************/

node *
TCappendAssignIcm (node *assign, char *name, node *args)
{
    node *result;

    DBUG_ENTER ("TCappendAssignIcm");

    result = TCappendAssign (assign, TCmakeAssignIcm1 (name, args, NULL));

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCappendExprs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

node *
TCappendExprs (node *exprs_chain, node *exprs)
{
    node *ret;

    DBUG_ENTER ("TCappendExprs");

    DBUG_ASSERT (((exprs_chain == NULL) || (NODE_TYPE (exprs_chain) == N_exprs)),
                 ("First argument of TCappendExprs() has wrong node type."));
    DBUG_ASSERT (((exprs == NULL) || (NODE_TYPE (exprs) == N_exprs)),
                 ("Second argument of TCappendExprs() has wrong node type."));

    APPEND (ret, node *, EXPRS, exprs_chain, exprs);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *TCcombineExprs( node *first, node *second)
 *
 * description:
 *   'first' and 'second' are N_exprs chains or expression nodes (N_id, N_num,
 *   ...) that will be conactenated to a single N_exprs chain.
 *
 ******************************************************************************/

node *
TCcombineExprs (node *first, node *second)
{
    node *result;

    DBUG_ENTER ("CombineExprs");

    if (first != NULL) {
        if (NODE_TYPE (first) != N_exprs) {
            result = TBmakeExprs (first, second);
        } else {
            result = TCappendExprs (first, second);
        }
    } else if (second != NULL) {
        if (NODE_TYPE (second) != N_exprs) {
            result = TBmakeExprs (second, NULL);
        } else {
            result = second;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeExprsNum( int num)
 *
 * description:
 *   Makes an N_exprs with a N_num as EXPR, NEXT is NULL.
 *
 ******************************************************************************/

node *
TCmakeExprsNum (int num)
{
    node *result;

    DBUG_ENTER ("TCmakeExprsNum");

    result = TBmakeExprs (TBmakeNum (num), NULL);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int TCcountExprs( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

int
TCcountExprs (node *exprs)
{
    int count;

    DBUG_ENTER ("CountExprs");

    count = 0;
    while (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "no N_exprs node found!");
        count++;
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (count);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsFromIds( node *ids)
 *
 * @brief Creates a N_exprs chain containing N_id nodes corresponding to the
 *        given N_ids chain.
 *
 * @param ids N_ids chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsFromIds (node *ids)
{
    node *result;

    DBUG_ENTER ("TCcreateExprsFromIds");

    if (ids != NULL) {
        result = TBmakeExprs (TBmakeId (IDS_AVIS (ids)),
                              TCcreateExprsFromIds (IDS_NEXT (ids)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsFromArgs( node *args)
 *
 * @brief Creates a N_exprs chain containing N_id nodes corresponding to the
 *        given N_arg chain.
 *
 * @param args N_arg chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsFromArgs (node *args)
{
    node *result;

    DBUG_ENTER ("TCcreateExprsFromArgs");

    if (args != NULL) {
        result = TBmakeExprs (TBmakeId (ARG_AVIS (args)),
                              TCcreateExprsFromArgs (ARG_NEXT (args)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

node *
TCgetNthExpr (int n, node *exprs)
{
    int cnt;
    node *result = NULL;

    DBUG_ENTER ("TCgetNthExpr");

    for (cnt = 1; cnt < n; cnt++) {
        if (exprs == NULL) {
            break;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL) {
        result = EXPRS_EXPR (exprs);
    }

    DBUG_RETURN (result);
}

node *
TCfilterExprs (bool (*pred) (node *), node **exprs)
{
    node *res = NULL;

    DBUG_ENTER ("TCfilterExprs");

    if (*exprs != NULL) {
        if (EXPRS_NEXT (*exprs) != NULL) {
            res = TCfilterExprs (pred, &(EXPRS_NEXT (*exprs)));
        }

        if (pred (EXPRS_EXPR (*exprs))) {
            node *tmp = EXPRS_NEXT (*exprs);
            EXPRS_NEXT (*exprs) = res;
            res = *exprs;
            *exprs = tmp;
        }
    }

    DBUG_RETURN (res);
}

bool
TCfoldPredExprs (bool (*pred) (node *), node *exprs)
{
    bool res;

    DBUG_ENTER ("TCfoldPredExprs");

    if (exprs == NULL) {
        res = TRUE;
    } else {
        res = (pred (EXPRS_EXPR (exprs)) && TCfoldPredExprs (pred, EXPRS_NEXT (exprs)));
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

/*
 *
 *  functionname  : TCnodeBehindCast
 *  arguments     : 1) expression-node of a let-node
 *                  R) node behind various cast's
 *  description   : determine what node is hidden behind the cast-nodes
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..
 *
 *  remarks       :
 *
 */

node *
TCnodeBehindCast (node *arg_node)
{
    DBUG_ENTER ("TCnodeBehindCast");
    while (N_cast == NODE_TYPE (arg_node)) {
        arg_node = CAST_EXPR (arg_node);
    }
    DBUG_RETURN (arg_node);
}

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
 ***  N_while :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :  *and*  N_while :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeVector( ntype *basetype, node *aelems)
 *
 * description:
 *   Returns a vector.
 *
 *****************************************************************************/
node *
TCmakeVector (ntype *elemtype, node *aelems)
{
    DBUG_ENTER ("TCmakeVector");

    DBUG_RETURN (
      TBmakeArray (elemtype, SHcreateShape (1, TCcountExprs (aelems)), aelems));
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeIntVector( node *aelems)
 *
 * description:
 *   Returns an integer vector.
 *
 *****************************************************************************/
node *
TCmakeIntVector (node *aelems)
{
    DBUG_ENTER ("TCmakeIntVector");

    DBUG_RETURN (
      TCmakeVector (TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)), aelems));
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroScalar( simpletype btype)
 *
 * description:
 *   Returns a scalar 0.
 *
 *****************************************************************************/
node *
TCcreateZeroScalar (simpletype btype)
{
    node *ret_node;

    DBUG_ENTER ("TCcreateZeroScalar");

    DBUG_ASSERT ((btype != T_user), "unresolved user-type found");
    DBUG_ASSERT ((btype != T_hidden), "hidden-type found");

    switch (btype) {
    case T_int:
        ret_node = TBmakeNum (0);
        break;
    case T_float:
        ret_node = TBmakeFloat (0);
        break;
    case T_double:
        ret_node = TBmakeDouble (0);
        break;
    case T_bool:
        ret_node = TBmakeBool (0);
        break;
    case T_char:
        ret_node = TBmakeChar ('\0');
        break;
    default:
        ret_node = NULL;
        DBUG_ASSERT (0, ("unkown basetype found"));
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroVector( int length, simpletype btype)
 *
 * description:
 *   Returns an N_array node with 'length' components, each 0.
 *
 ******************************************************************************/

node *
TCcreateZeroVector (int length, simpletype btype)
{
    node *ret_node, *exprs_node;
    int i;

    DBUG_ENTER ("TCcreateZeroVector");

    DBUG_ASSERT ((btype != T_user), "unresolved user-type found");
    DBUG_ASSERT ((btype != T_hidden), "hidden-type found");

    exprs_node = NULL;
    for (i = 0; i < length; i++) {
        exprs_node = TBmakeExprs (TCcreateZeroScalar (btype), exprs_node);
    }

    ret_node
      = TCmakeVector (TYmakeAKS (TYmakeSimpleType (btype), SHmakeShape (0)), exprs_node);

    DBUG_RETURN (ret_node);
}

/*****************************************************************************
 *
 * function:
 *   node *TCids2Exprs( ids *ids_arg)
 *
 * description:
 *   convert ids into N_exprs chain
 *
 *****************************************************************************/

node *
TCids2Exprs (node *ids_arg)
{
    node *exprs;

    DBUG_ENTER ("TCids2Exprs");

    if (ids_arg != NULL) {
        exprs = TBmakeExprs (DUPdupIdsId (ids_arg), TCids2Exprs (IDS_NEXT (ids_arg)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

/*****************************************************************************
 *
 * function:
 *   node *TCids2ExprsNT( ids *ids_arg)
 *
 * description:
 *   convert ids into N_exprs chain
 *
 *****************************************************************************/

node *
TCids2ExprsNt (node *ids_arg)
{
    node *exprs;

    DBUG_ENTER ("TCids2ExprsNt");

    if (ids_arg != NULL) {
        exprs = TBmakeExprs (DUPdupIdsIdNt (ids_arg), TCids2ExprsNt (IDS_NEXT (ids_arg)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*--------------------------------------------------------------------------*/
node *
TCmakeIdCopyString (const char *str)
{
    node *result;
    DBUG_ENTER ("TCmakeIdCopyString");

    if (str == NULL) {
        str = "";
    }

    result = TBmakeId (NULL);

    ID_ICMTEXT (result) = ILIBstringCopy (str);

    DBUG_RETURN (result);
}

node *
TCmakeIdCopyStringNt (const char *str, types *type)
{
    node *result;

    DBUG_ENTER ("TCmakeIdCopyStringNt");

    result = TCmakeIdCopyString (str);
    ID_NT_TAG (result) = NTUcreateNtTag (str, type);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ids :
 ***/

/*--------------------------------------------------------------------------*/

/***************************************************************************
 *
 * function:
 *   bool TCisPhiFun(node *id)
 *
 * description:
 *   this function returns TRUE if the defining assignment of 'id'
 *   uses the primitive phi function 'F_phi'. In all other cases id
 *   returns FALSE.
 *   This function replaces the PHITARGET macro used to identify phi functions.
 *
 ****************************************************************************/

bool
TCisPhiFun (node *id)
{

    bool result;

    DBUG_ENTER ("TCisPhiFun");

    if ((AVIS_SSAASSIGN (ID_AVIS (id)) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (id)))) == N_funcond))
        result = TRUE;
    else
        result = FALSE;

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakePrf1( prf prf, node *arg1)
 *   node *TCmakePrf2( prf prf, node *arg1, node *arg2)
 *   node *TCmakePrf3( prf prf, node *arg1, node *arg2, node *arg3)
 *   node *TCmakePrf4( prf prf, node *arg1, node *arg2, node *arg3, node *arg4)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
TCmakePrf1 (prf prf, node *arg1)
{
    node *res;

    DBUG_ENTER ("TBmakePrf1");

    res = TBmakePrf (prf, TBmakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}

node *
TCmakePrf2 (prf prf, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ("TBmakePrf2");

    res = TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
TCmakePrf3 (prf prf, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ("TBmakePrf3");

    res
      = TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

node *
TCmakePrf4 (prf prf, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *res;

    DBUG_ENTER ("TBmakePrf4");

    res = TBmakePrf (prf,
                     TBmakeExprs (arg1,
                                  TBmakeExprs (arg2,
                                               TBmakeExprs (arg3,
                                                            TBmakeExprs (arg4, NULL)))));

    DBUG_RETURN (res);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeAp1( char *name, char *mod, node *arg1)
 *   node *TCmakeAp2( char *name, char *mod, node *arg1, node *arg2)
 *   node *TCmakeAp3( char *name, char *mod, node *arg1, node *arg2, node *arg3)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
TCmakeAp1 (node *fundef, node *arg1)
{
    node *res;

    DBUG_ENTER ("TCmakeAp1");

    res = TBmakeAp (fundef, TBmakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}
node *
TCmakeAp2 (node *fundef, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ("TCmakeAp2");

    res = TBmakeAp (fundef, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
TCmakeAp3 (node *fundef, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ("TCmakeAp3");

    res = TBmakeAp (fundef,
                    TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

node *
TCmakeSpap1 (namespace_t *ns, char *name, node *arg1)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result = TBmakeSpap (TBmakeSpid (ns, name), TBmakeExprs (arg1, NULL));

    DBUG_RETURN (result);
}

node *
TCmakeSpap2 (namespace_t *ns, char *name, node *arg1, node *arg2)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result
      = TBmakeSpap (TBmakeSpid (ns, name), TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (result);
}

node *
TCmakeSpap3 (namespace_t *ns, char *name, node *arg1, node *arg2, node *arg3)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result
      = TBmakeSpap (TBmakeSpid (ns, name),
                    TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (result);
}
/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeIcm0( char *name)
 *   node *TCmakeIcm?( char *name, node *arg1, ...)
 *
 * description:
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of ICM arguments (i.e.,
 *   a single expression node (N_id, N_num, ...) or a N_exprs chain!)
 *   These are concatenated correctly.
 *
 ******************************************************************************/

node *
TCmakeIcm0 (char *name)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm0");

    icm = TBmakeIcm (name, NULL);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm1 (char *name, node *arg1)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm1");

    arg1 = TCcombineExprs (arg1, NULL);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm2 (char *name, node *arg1, node *arg2)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm2");

    icm = TBmakeIcm (name, TCcombineExprs (arg1, TCcombineExprs (arg2, NULL)));

    DBUG_RETURN (icm);
}

node *
TCmakeIcm3 (char *name, node *arg1, node *arg2, node *arg3)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm3");

    arg1 = TCcombineExprs (arg1, TCcombineExprs (arg2, TCcombineExprs (arg3, NULL)));
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm4");

    arg4 = TCcombineExprs (arg4, NULL);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm5");

    arg5 = TCcombineExprs (arg5, NULL);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
            node *arg6)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm6");

    arg6 = TCcombineExprs (arg6, NULL);
    arg5 = TCcombineExprs (arg5, arg6);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
            node *arg6, node *arg7)
{
    node *icm;

    DBUG_ENTER ("TCmakeIcm7");

    arg7 = TCcombineExprs (arg7, NULL);
    arg6 = TCcombineExprs (arg6, arg7);
    arg5 = TCcombineExprs (arg5, arg6);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :   N_st :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_part:
 ***/

/** <!--********************************************************************-->
 *
 * @fn int TCcountParts( node *parts)
 *
 * @brief Counts the number of parts in a given chain of parts
 *
 * @param parts
 *
 * @return number of parts
 *
 *****************************************************************************/
int
TCcountParts (node *parts)
{
    int counter = 0;

    DBUG_ENTER ("TCcountParts");

    while (parts != NULL) {
        counter += 1;
        parts = PART_NEXT (parts);
    }

    DBUG_RETURN (counter);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_code :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_withop :
 ***/

int
TCcountWithops (node *withop)
{
    int counter = 0;

    DBUG_ENTER ("TCcountWithops");

    while (withop != NULL) {
        counter += 1;
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (counter);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_with2 :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :  *and*  N_with2 :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlseg :  *and*  N_wlsegVar :
 ***/

/******************************************************************************
 *
 * Function:
 *   node *TCmakeWLsegX( int dims, node *contents, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCmakeWlSegX (int dims, node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("TCmakeWlSegX");

    if (WLTRAallStridesAreConstant (contents, TRUE, TRUE)) {
        new_node = TBmakeWlseg (dims, contents, next);
    } else {
        new_node = TBmakeWlsegvar (dims, contents, next);
    }

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_str :
 ***/

node *
TCmakeStrCopy (const char *str)
{
    DBUG_ENTER ("TCmakeStrCopy");

    DBUG_RETURN (TBmakeStr (ILIBstringCopy (str)));
}

/*--------------------------------------------------------------------------*/

/***
 *** N_set
 ***/

int
TCSetAdd (node **links, node *link)
{
    int result = 0;

    DBUG_ENTER ("TCSetAdd");

    if (*links == NULL) {
        /*
         * it has not been found so far, so append it
         */
        *links = TBmakeSet (link, NULL);
        result = 1;
    } else if (SET_MEMBER (*links) != link) {
        /*
         * its not the current one, so go on
         */
        result = TCSetAdd (&SET_NEXT (*links), link);
    }

    DBUG_RETURN (result);
}

int
TCSetUnion (node **links, node *add)
{
    int result = 0;

    DBUG_ENTER ("TCSetUnion");

    while (add != NULL) {
        result += TCSetAdd (links, SET_MEMBER (add));
        add = SET_NEXT (add);
    }

    DBUG_RETURN (result);
}

bool
TCSetContains (node *set, node *link)
{
    bool result = FALSE;

    DBUG_ENTER ("TCSetContains");

    while ((set != NULL) && (!result)) {
        DBUG_ASSERT ((NODE_TYPE (set) == N_set),
                     "called TCSetContains with non N_set node!");

        result = (SET_MEMBER (set) == link);

        set = SET_NEXT (set);
    }

    DBUG_RETURN (result);
}

bool
TCSetIsSubset (node *super, node *sub)
{
    bool result = TRUE;

    DBUG_ENTER ("TClinklistIsSubset");

    while ((sub != NULL) && result) {
        DBUG_ASSERT ((NODE_TYPE (sub) == N_set),
                     "called TCSetIsSubset with non N_set node!");

        result = result && TCSetContains (super, SET_MEMBER (sub));

        sub = SET_NEXT (sub);
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  ERROR :
 ***/

node *
TCappendError (node *chain, node *item)
{
    node *ret;

    DBUG_ENTER ("TCappendError");

    APPEND (ret, node *, IDS, chain, item);

    DBUG_RETURN (ret);
}
