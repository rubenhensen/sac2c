/*
 *
 * $Log$
 * Revision 3.38  2001/05/11 13:34:11  nmw
 * AdjustAvisData fixed for handling multiple inlined ssacounters
 *
 * Revision 3.37  2001/05/09 12:32:24  nmw
 * AdjustAvisData created ssacnt nodes only when used in ssaform
 *
 * Revision 3.36  2001/04/26 13:26:39  dkr
 * CountIds() added
 *
 * Revision 3.35  2001/04/26 12:30:09  dkr
 * GetExprsLength() renamed into CountExprs()
 *
 * Revision 3.34  2001/04/25 12:27:57  dkr
 * NodeListDelete: works correctly even for empty nodelists now
 *
 * Revision 3.33  2001/04/17 15:26:37  nmw
 * AppendTypes added
 *
 * Revision 3.32  2001/04/09 15:56:33  nmw
 * MakeArgFromVardec added
 *
 * Revision 3.31  2001/04/06 18:50:10  dkr
 * CountArgs() and CountTypes() added
 *
 * Revision 3.30  2001/04/05 12:29:51  nmw
 * debug messages added, big error with wrong macro corrected in AdjustAvisData
 *
 * Revision 3.29  2001/04/04 09:58:30  nmw
 * AdjustAvisData added
 *
 * Revision 3.28  2001/04/02 11:45:00  dkr
 * functions NodeOrInt...(), NameOrVal...() moved to wl_bounds.[ch]
 *
 * Revision 3.27  2001/04/02 11:17:31  nmw
 * bug fixed in NodeListDelete when removing last element in list
 *
 * Revision 3.26  2001/03/29 09:17:01  nmw
 * tabs2spaces done
 *
 * Revision 3.25  2001/03/29 01:33:35  dkr
 * functions for NodeOrInt, NameOrVal recoded
 *
 * Revision 3.21  2001/03/27 15:40:17  nmw
 * Array2Vec as wrapper for different Array2<XYZ>Vec added
 *
 * Revision 3.20  2001/03/21 17:48:12  dkr
 * comment for AppendAssign() modified
 *
 * Revision 3.19  2001/03/20 22:28:48  dkr
 * MakeVardecFromArg: VARDEC_OBJDEF set
 *
 * Revision 3.18  2001/03/19 14:24:54  nmw
 * AVIS_ASSIGN2 init in MakeVardecFromArg added
 *
 * Revision 3.17  2001/03/16 11:57:07  nmw
 * AVIS_SSAPHITRAGET type changed
 *
 * Revision 3.16  2001/03/15 21:23:53  dkr
 * signature of NodeOr..._MakeIndex modified:
 * parameter 'no_icm' added
 *
 * Revision 3.15  2001/03/15 10:54:49  nmw
 * MakeVardecFromArgs adjusts VARDEC_ATTRIB correctly
 *
 * Revision 3.12  2001/02/22 12:45:16  nmw
 * MakeVardecFromArg added
 *
 * Revision 3.11  2001/02/20 15:52:53  nmw
 * AppendAssign in now able to handle empty blocks
 *
 * Revision 3.10  2001/02/15 16:59:43  nmw
 * access macro for SSAstack added
 *
 * Revision 3.8  2001/01/29 16:08:51  dkr
 * NameOrVal_Le() and NodeOrInt_Le() added
 *
 * Revision 3.7  2001/01/24 23:34:42  dkr
 * NameOrVal_MakeIndex, NodeOrInt_MakeIndex added
 *
 * Revision 3.6  2001/01/19 14:16:59  dkr
 * include of DupTree.h added
 *
 * Revision 3.5  2001/01/19 11:54:55  dkr
 * NameOrVal_MakeNode() modified
 *
 * Revision 3.4  2001/01/17 14:17:19  dkr
 * functions NameOrVal_... and NodeOrInt_... added
 *
 * Revision 3.3  2001/01/10 14:33:00  dkr
 * function MakeWLsegX added
 *
 * Revision 3.2  2000/12/04 10:45:44  dkr
 * bug in Type2Shpseg fixed:
 * return value is now initialized correctly even for scalar types
 *
 * Revision 3.1  2000/11/20 18:03:35  sacbase
 * new release made
 *
 * Revision 1.30  2000/11/14 13:19:40  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 1.29  2000/10/31 23:31:53  dkr
 * signature of Type2Shpseg, Array2Shpseg modified
 *
 * Revision 1.28  2000/10/27 02:29:25  dkr
 * bug in GetTypes_Line fixed: condition for ABORT is no longer missing
 *
 * Revision 1.27  2000/10/27 00:06:07  dkr
 * Type2Shpseg and Type2Exprs added,
 * some code brushing done.
 *
 * Revision 1.26  2000/10/26 14:04:27  dkr
 * MakeShpseg used in Array2Shpseg
 *
 * Revision 1.25  2000/10/26 13:57:37  dkr
 * CopyShpseg replaced by DupShpseg (DupTree.[ch])
 *
 * Revision 1.24  2000/10/24 14:47:46  dkr
 * some append functions added
 *
 * Revision 1.23  2000/10/24 11:42:40  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 1.22  2000/10/24 10:06:01  dkr
 * GetBasetypeSize() added
 *
 * Revision 1.21  2000/10/24 09:44:04  dkr
 * GetSimpletype renamed into GetBasetype
 *
 * Revision 1.20  2000/10/20 15:38:30  dkr
 * some functions on types added
 *
 * Revision 1.19  2000/10/12 18:41:07  dkr
 * return value type of Is...() functions is bool, new
 *
 * Revision 1.18  2000/09/25 15:15:41  dkr
 * IsBoxed() simplified (uses IsHidden() now)
 *
 * Revision 1.17  2000/09/15 15:43:44  dkr
 * IsNonUniqueHidden() revisited
 *
 * Revision 1.16  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 1.15  2000/07/21 14:17:53  mab
 * added EqualShpseg
 *
 * Revision 1.14  2000/07/14 09:37:45  dkr
 * CopyNodelist renamed into DupNodelist and moved to DupTree.[ch]
 *
 * Revision 1.13  2000/07/12 15:19:40  dkr
 * function SearchDecl moved from Inline.c to tree_compound.c
 *
 * [...]
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"

#include "dbug.h"
#include "my_debug.h"
#include "free.h"

#include "typecheck.h"
#include "wltransform.h"

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
 *   int GetShpsegLength( int dims, shpseg *shape)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
GetShpsegLength (int dims, shpseg *shape)
{
    int length, i;

    DBUG_ENTER ("GetShpsegLength");

    if (dims > 0) {
        length = 1;
        for (i = 0; i < dims; i++) {
            length *= SHPSEG_SHAPE (shape, i);
        }
    } else {
        length = 0;
    }

    DBUG_RETURN (length);
}

/*****************************************************************************
 *
 * function:
 *   shpseg *DiffShpseg( int dim, shpseg *shape1, shpseg *shape2)
 *
 * description:
 *   calculate shape1 - shape2
 *
 *****************************************************************************/

shpseg *
DiffShpseg (int dim, shpseg *shape1, shpseg *shape2)
{

    shpseg *shape_diff;
    int i;

    DBUG_ENTER ("DiffShpseg");

    shape_diff = MakeShpseg (NULL);

    for (i = 0; i < dim; i++) {
        SHPSEG_SHAPE (shape_diff, i)
          = SHPSEG_SHAPE (shape1, i) - SHPSEG_SHAPE (shape2, i);
    }

    DBUG_RETURN (shape_diff);
}

/*****************************************************************************
 *
 * function:
 *   bool EqualShpseg( int dim, shpseg *shape2, shpseg *shape1)
 *
 * description:
 *   compares two shapes, result is TRUE, if shapes are equal
 *
 *****************************************************************************/

bool
EqualShpseg (int dim, shpseg *shape2, shpseg *shape1)
{

    bool equal_shapes;
    int i;

    DBUG_ENTER ("EqualShpseg");

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

/*****************************************************************************
 *
 * function:
 *   shpseg *Array2Shpseg( node *array, int *ret_dim)
 *
 * description:
 *   Convert 'array' into a shpseg (requires int-array!!!).
 *   If 'dim' is != NULL the dimensionality is stored there.
 *
 *****************************************************************************/

shpseg *
Array2Shpseg (node *array, int *ret_dim)
{

    node *tmp;
    shpseg *shape;
    int i;

    DBUG_ENTER ("Array2Shpseg");

    shape = MakeShpseg (NULL);

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
 *   node *Shpseg2Array( shpseg *shape, int dim)
 *
 * description:
 *   convert shpseg with given dimension into array with simpletype T_int
 *
 *****************************************************************************/

node *
Shpseg2Array (shpseg *shape, int dim)
{

    int i;
    node *next;
    node *array_node;
    shpseg *array_shape;

    DBUG_ENTER ("Shpseg2Array");

    i = dim - 1;
    next = MakeExprs (MakeNum (SHPSEG_SHAPE (shape, i)), NULL);
    i--;
    for (; i >= 0; i--) {
        next = MakeExprs (MakeNum (SHPSEG_SHAPE (shape, i)), next);
    }

    array_node = MakeArray (next);
    array_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (array_shape, 0) = dim;
    ARRAY_TYPE (array_node) = MakeTypes (T_int, 1, array_shape, NULL, NULL);

    DBUG_RETURN (array_node);
}

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***/

/******************************************************************************
 *
 * function:
 *   types *AppendTypes( types *chain, types *item)
 *
 * description:
 *   append item to list of types
 *
 ******************************************************************************/
types *
AppendTypes (types *chain, types *item)
{
    types *ret;

    DBUG_ENTER ("AppendTypes");

    APPEND (ret, types *, TYPES, chain, item);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   int CountTypes( types *type)
 *
 * description:
 *   Counts the number of types.
 *
 ******************************************************************************/

int
CountTypes (types *type)
{
    int count = 0;

    DBUG_ENTER ("CountTypes");

    while (type != NULL) {
        count++;
        type = TYPES_NEXT (type);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * Function:
 *   type *GetTypes_Line( types* type, int line)
 *
 * Description:
 *   line > 0:  generate an error message if error occurs.
 *   otherwise: DBUG assert.
 *
 ******************************************************************************/

types *
GetTypes_Line (types *type, int line)
{
    node *tdef;
    types *res_type = NULL;

    DBUG_ENTER ("GetTypes_Line");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);

        if ((tdef == NULL) && (compiler_phase <= PH_typecheck)) {
            tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), line);
        }

        if (line > 0) {
            if (tdef == NULL) {
                ABORT (line, ("type '%s' is unknown",
                              ModName (TYPES_MOD (type), TYPES_NAME (type))));
            }
        } else {
            DBUG_ASSERT ((tdef != NULL), "typedef not found!");
        }

        DBUG_ASSERT ((TYPEDEF_BASETYPE (tdef) != T_user),
                     "unresolved nested user-defined type found");

        if (TYPEDEF_BASETYPE (tdef) == T_hidden) {
            /*
             * Basic type is hidden therefore we have to use the original type
             * structure and rely on the belonging typedef!!
             */
            res_type = type;
        } else {
            res_type = TYPEDEF_TYPE (tdef);
        }
    } else {
        res_type = type;
    }

    DBUG_RETURN (res_type);
}

/******************************************************************************
 *
 * Function:
 *   type *GetTypes( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

types *
GetTypes (types *type)
{
    types *res_type;

    DBUG_ENTER ("GetTypes");

    res_type = GetTypes_Line (type, 0);

    DBUG_RETURN (res_type);
}

/******************************************************************************
 *
 * Function:
 *   int GetDim( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
GetDim (types *type)
{
    types *impl_type;
    int dim, base_dim, impl_dim;

    DBUG_ENTER ("GetDim");

    base_dim = TYPES_DIM (type);

    impl_type = GetTypes (type);

    if (impl_type != type) {
        /*
         * user-defined type
         */
        impl_dim = TYPES_DIM (impl_type);

        if ((UNKNOWN_SHAPE == impl_dim) || (UNKNOWN_SHAPE == base_dim)) {
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
 *   simpletype GetBasetype( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

simpletype
GetBasetype (types *type)
{
    simpletype res;

    DBUG_ENTER ("GetBasetype");

    res = TYPES_BASETYPE (GetTypes (type));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   int GetBasetypeSize(types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
GetBasetypeSize (types *type)
{
    int size;

    DBUG_ENTER ("GetBasetypeSize");

    size = basetype_size[GetBasetype (type)];

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
GetTypesLength (types *type)
{
    shpseg *shape;
    int dim, length;

    DBUG_ENTER ("GetTypesLength");

    shape = Type2Shpseg (type, &dim);

    length = GetShpsegLength (dim, shape);

    if (shape != NULL) {
        shape = FreeShpseg (shape);
    }

    DBUG_RETURN (length);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *Type2Shpseg( types *type, int *ret_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
Type2Shpseg (types *type, int *ret_dim)
{
    int dim, base_dim, i;
    types *impl_type;
    shpseg *new_shpseg = NULL;

    DBUG_ENTER ("Type2Shpseg");

    dim = GetDim (type);

    DBUG_ASSERT ((dim < SHP_SEG_SIZE), "shape is out of range");

    if (dim > SCALAR) {
        new_shpseg = MakeShpseg (NULL);
        impl_type = GetTypes (type);

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
 *   node *Type2Exprs( types *type)
 *
 * Description:
 *   Computes the shape of corresponding type and stores it as N_exprs chain.
 *
 ******************************************************************************/

node *
Type2Exprs (types *type)
{
    node *tmp;
    types *impl_type;
    int dim, i;
    node *ret_node = NULL;

    DBUG_ENTER ("Type2Exprs");

    /* create a dummy node to append the shape items to */
    ret_node = MakeExprs (NULL, NULL);

    dim = GetDim (type);

    if (dim > SCALAR) {
        tmp = ret_node;
        impl_type = GetTypes (type);

        for (i = 0; i < TYPES_DIM (type); i++) {
            EXPRS_NEXT (tmp) = MakeExprs (MakeNum (TYPES_SHAPE (type, i)), NULL);
            tmp = EXPRS_NEXT (tmp);
        }

        if (impl_type != type) {
            /*
             * user-defined type
             */
            for (i = 0; i < TYPES_DIM (impl_type); i++) {
                EXPRS_NEXT (tmp) = MakeExprs (MakeNum (TYPES_SHAPE (impl_type, i)), NULL);
                tmp = EXPRS_NEXT (tmp);
            }
        }
    }

    /* remove dummy node at head of chain */
    ret_node = FreeNode (ret_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   bool IsBoxed( types *type)
 *   bool IsArray( types *type)
 *   bool IsUnique( types *type)
 *   bool IsHidden( types *type)
 *   bool IsNonUniqueHidden( types *type)
 *
 * description:
 *   These functions may be used to check for particular properties
 *   of a given data type.
 *
 ******************************************************************************/

bool
IsBoxed (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsBoxed");

    if (IsHidden (type)) {
        ret = TRUE;
    } else if (TYPES_DIM (type) != SCALAR) {
        ret = TRUE;
    } else {
        if (TYPES_BASETYPE (type) == T_user) {
            tdef = TYPES_TDEF (type);
            DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

            if (TYPEDEF_DIM (tdef) != SCALAR) {
                ret = TRUE;
            }
        }
    }

    DBUG_RETURN (ret);
}

bool
IsArray (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsArray");

    if ((SCALAR != TYPES_DIM (type)) && (ARRAY_OR_SCALAR != TYPES_DIM (type))) {
        ret = TRUE;
    } else {
        if (T_user == TYPES_BASETYPE (type)) {
            tdef = TYPES_TDEF (type);
            DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

            if ((SCALAR != TYPEDEF_DIM (tdef))
                && (ARRAY_OR_SCALAR != TYPEDEF_DIM (tdef))) {
                ret = TRUE;
            }
        }
    }

    DBUG_RETURN (ret);
}

bool
IsUnique (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsUnique");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
            ret = TRUE;
        }
    }

    DBUG_RETURN (ret);
}

bool
IsHidden (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsHidden");

    if (TYPES_BASETYPE (type) == T_hidden) {
        ret = TRUE;
    } else if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        if (TYPEDEF_BASETYPE (tdef) == T_hidden) {
            ret = TRUE;
        }
    }

    DBUG_RETURN (ret);
}

bool
IsNonUniqueHidden (types *type)
{
    bool ret = FALSE;

    DBUG_ENTER ("IsNonUniqueHidden");

#if 0
  if (TYPES_BASETYPE( type) == T_user) {
    node *tdef;

    tdef = TYPES_TDEF( type);
    DBUG_ASSERT( (tdef != NULL), "Failed attempt to look up typedef");

    if ((TYPEDEF_BASETYPE(tdef) == T_hidden) ||
        (TYPEDEF_BASETYPE(tdef) == T_user)) {
      if (TYPEDEF_TNAME(tdef) == NULL) {
        if (TYPEDEF_ATTRIB(tdef) == ST_regular) {
          ret = TRUE;
        }
      }
      else {
        tdef = TYPES_TDEF( TYPEDEF_TYPE( tdef));
        DBUG_ASSERT( (tdef != NULL), "Failed attempt to look up typedef");

        if (TYPEDEF_ATTRIB( tdef) == ST_regular) {
          ret = TRUE;
        }
      }
    }
  }
#else
    /* dkr */
    ret = (IsHidden (type) && (!IsUnique (type)));
#endif

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***/

ids *
AppendIds (ids *chain, ids *item)
{
    ids *ret;

    DBUG_ENTER ("AppendIds");

    APPEND (ret, ids *, IDS, chain, item);

    DBUG_RETURN (ret);
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

int
CountIds (ids *ids_arg)
{
    int count = 0;

    DBUG_ENTER ("CountIds");

    while (ids_arg != NULL) {
        count++;
        ids_arg = IDS_NEXT (ids_arg);
    }

    DBUG_RETURN (count);
}

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
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
            res = MALLOC (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_float:
            n = veclen * sizeof (float);
            res = MALLOC (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_double:
            n = veclen * sizeof (double);
            res = MALLOC (n);
            res = memcpy (res, const_vec, n);
            break;
        case T_char:
            n = veclen * sizeof (char);
            res = MALLOC (n);
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
            res = MALLOC (veclen * sizeof (int));
            break;
        case T_float:
            res = MALLOC (veclen * sizeof (float));
            break;
        case T_double:
            res = MALLOC (veclen * sizeof (double));
            break;
        case T_char:
            res = MALLOC (veclen * sizeof (int));
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
 *   const_node.
 *   It is assumed (!!!) that simpletype is compatible to the const_node;
 *   if this requires a cast (e.g. NODE_TYPE(const_node) == N_num &&
 *   vectype==T_double) it will be done implicitly.
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

    while (NODE_TYPE (behind_casts) == N_cast) {
        behind_casts = CAST_EXPR (behind_casts);
    }

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
 ***  NODELIST :
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

/******************************************************************************
 *
 * function:
 *   nodelist *NodeListAppend(nodelist *nl, node *newnode, void *attrib)
 *   nodelist *NodeListDelete(nodelist *nl, node *node, bool free_attrib)
 *   nodelist *NodeListFree(nodelist *nl, bool free_attrib)
 *   nodelist *NodeListFind(nodelist *nl, node *node)
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
NodeListAppend (nodelist *nl, node *newnode, void *attrib)
{
    DBUG_ENTER ("NodeListAppend");

    nl = MakeNodelistNode (newnode, nl);
    NODELIST_ATTRIB2 (nl) = attrib;

    DBUG_RETURN (nl);
}

nodelist *
NodeListDelete (nodelist *nl, node *node, bool free_attrib)
{
    nodelist *tmpnl, *prevnl;

    DBUG_ENTER ("NodeListDelete");

    while (nl && NODELIST_NODE (nl) == node) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            FREE (NODELIST_ATTRIB2 (nl));
        }
        nl = FreeNodelistNode (nl);
    }

    tmpnl = nl;
    prevnl = NULL;
    while (tmpnl) {
        if (NODELIST_NODE (tmpnl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (tmpnl)) {
                FREE (NODELIST_ATTRIB2 (tmpnl));
            }

            NODELIST_NEXT (prevnl) = FreeNodelistNode (tmpnl);
        } else {
            prevnl = tmpnl;
        }

        tmpnl = NODELIST_NEXT (prevnl);
    }

    DBUG_RETURN (nl);
}

nodelist *
NodeListFree (nodelist *nl, bool free_attrib)
{
    DBUG_ENTER ("NodeListFree");

    while (nl) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            FREE (NODELIST_ATTRIB2 (nl));
        }
        nl = FreeNodelistNode (nl);
    }

    DBUG_RETURN (nl);
}

nodelist *
NodeListFind (nodelist *nl, node *node)
{
    DBUG_ENTER ("NodeListFind");

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

node *
AppendTypedef (node *tdef_chain, node *tdef)
{
    node *ret;

    DBUG_ENTER ("AppendTypedef");

    APPEND (ret, node *, TYPEDEF, tdef_chain, tdef);

    DBUG_RETURN (ret);
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

node *
AppendObjdef (node *objdef_chain, node *objdef)
{
    node *ret;

    DBUG_ENTER ("AppendObjdef");

    APPEND (ret, node *, OBJDEF, objdef_chain, objdef);

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
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

/******************************************************************************
 *
 * function:
 *   int CountFunctionParams( node *fundef)
 *
 * description:
 *   Counts the number of arguments AND return-values, adds both and returns
 *   the sum.
 *
 ******************************************************************************/

int
CountFunctionParams (node *fundef)
{
    int count;

    DBUG_ENTER ("CountFunctionParams");

    count = CountTypes (FUNDEF_TYPES (fundef));
    count += CountArgs (FUNDEF_ARGS (fundef));

    DBUG_RETURN (count);
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

node *
AppendFundef (node *fundef_chain, node *fundef)
{
    node *ret;

    DBUG_ENTER ("AppendFundef");

    APPEND (ret, node *, FUNDEF, fundef_chain, fundef);

    DBUG_RETURN (ret);
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
 *   node *AppendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 * remark:
 *   In order to use this function in Compile() it can handle mixed chains
 *   containing N_vardec- *and* N_assign-nodes!
 *
 ******************************************************************************/

node *
AppendVardec (node *vardec_chain, node *vardec)
{
    node *tmp;

    DBUG_ENTER ("AppendVardec");

    tmp = vardec_chain;
    if (tmp != NULL) {
        while (((NODE_TYPE (tmp) == N_assign) ? (ASSIGN_NEXT (tmp)) : (VARDEC_NEXT (tmp)))
               != NULL) {
            tmp
              = (NODE_TYPE (tmp) == N_assign) ? (ASSIGN_NEXT (tmp)) : (VARDEC_NEXT (tmp));
        }
        if (NODE_TYPE (tmp) == N_assign) {
            ASSIGN_NEXT (tmp) = vardec;
        } else {
            VARDEC_NEXT (tmp) = vardec;
        }
    } else {
        vardec_chain = vardec;
    }

    DBUG_RETURN (vardec_chain);
}

/******************************************************************************
 *
 * function:
 *   node *MakeVardecFromArg( node *arg)
 *
 * description:
 *   copies all attributes from an arg node to a new allocated vardec node.
 *
 * remark:
 *   This function is used by ssa-transformation to rename a redefinition
 *   of an fundef argument. The Next pointer is set to NULL.
 *
 *
 ******************************************************************************/

node *
MakeVardecFromArg (node *arg_node)
{
    node *new_vardec;

    DBUG_ENTER ("MakeVardecFromArg");

    new_vardec = MakeVardec (StringCopy (ARG_NAME (arg_node)),
                             DupTypes (ARG_TYPE (arg_node)), NULL);

    VARDEC_STATUS (new_vardec) = ARG_STATUS (arg_node);

    VARDEC_TDEF (new_vardec) = ARG_TDEF (arg_node);
    VARDEC_OBJDEF (new_vardec) = ARG_OBJDEF (arg_node);

    /* duplicate avis node manually */
    FreeNode (VARDEC_AVIS (new_vardec));
    VARDEC_AVIS (new_vardec) = DupNode (ARG_AVIS (arg_node));
    AVIS_VARDECORARG (VARDEC_AVIS (new_vardec)) = new_vardec;

    /* delete wrong data in copied AVIS node */
    AVIS_SSAASSIGN (VARDEC_AVIS (new_vardec)) = NULL;
    AVIS_SSAASSIGN2 (VARDEC_AVIS (new_vardec)) = NULL;
    AVIS_SSAPHITARGET (VARDEC_AVIS (new_vardec)) = PHIT_NONE;
    AVIS_SSALPINV (VARDEC_AVIS (new_vardec)) = FALSE;
    AVIS_SSASTACK_TOP (VARDEC_AVIS (new_vardec)) = NULL;

    /*
     * reference parameter will be changed to unique vardecs that
     * will be re-renamed in UndoSSATransform
     */
    if (ARG_ATTRIB (arg_node) != ST_regular) {
        VARDEC_ATTRIB (new_vardec) = ST_unique;
        /* set the corresponding undo flag for UndoSSATransform */
        AVIS_SSAUNDOFLAG (VARDEC_AVIS (new_vardec)) = TRUE;
    } else {
        VARDEC_ATTRIB (new_vardec) = ARG_ATTRIB (arg_node);
    }

    DBUG_RETURN (new_vardec);
}

/******************************************************************************
 *
 * function:
 *   node *MakeArgFromVardec( node *varde_node)
 *
 * description:
 *   copies all attributes from an vardec node to a new allocated arg node.
 *
 * remark:
 *   This function is used in context of loop invariant removal in ssa form.
 *
 *
 ******************************************************************************/

node *
MakeArgFromVardec (node *vardec_node)
{
    node *new_arg;

    DBUG_ENTER ("MakeArgFromVardec");

    new_arg = MakeArg (StringCopy (VARDEC_NAME (vardec_node)),
                       DupTypes (VARDEC_TYPE (vardec_node)), VARDEC_STATUS (vardec_node),
                       ST_undef, NULL);

    ARG_TDEF (new_arg) = VARDEC_TDEF (vardec_node);
    ARG_OBJDEF (new_arg) = VARDEC_OBJDEF (vardec_node);

    /* duplicate avis node manually */
    FreeNode (ARG_AVIS (new_arg));
    ARG_AVIS (new_arg) = DupNode (VARDEC_AVIS (vardec_node));
    AVIS_VARDECORARG (ARG_AVIS (new_arg)) = new_arg;

    /* delete wrong data in copied AVIS node */
    AVIS_SSAASSIGN (ARG_AVIS (new_arg)) = NULL;
    AVIS_SSAASSIGN2 (ARG_AVIS (new_arg)) = NULL;
    AVIS_SSAPHITARGET (ARG_AVIS (new_arg)) = PHIT_NONE;
    AVIS_SSALPINV (ARG_AVIS (new_arg)) = FALSE;
    AVIS_SSASTACK_TOP (ARG_AVIS (new_arg)) = NULL;

    /*
     * unique vardecs will be changed to reference args that
     * will be re-renamed in UndoSSATransform
     */
    if (VARDEC_ATTRIB (vardec_node) == ST_unique) {
        ARG_ATTRIB (new_arg) = ST_reference;
        /* set the corresponding undo flag for UndoSSATransform */
        AVIS_SSAUNDOFLAG (ARG_AVIS (new_arg)) = TRUE;
    } else {
        ARG_ATTRIB (new_arg) = VARDEC_ATTRIB (vardec_node);
    }

    DBUG_RETURN (new_arg);
}

/******************************************************************************
 *
 * function:
 *   node *AdjustAvisData( node *new_vardec, node *fundef)
 *
 * description:
 *   when a vardec is duplicated via DupTree all dependend infomation in the
 *   corresponding avis node is duplicated, too. when this vardec is used in
 *   the same fundef as the original one everything is good, but if the
 *   duplicated vardec should be used in a different fundef the fundef related
 *   attributes have to be adjusted by this function:
 *     AVIS_SSACOUNT = (new fresh ssacnt node)
 *     AVIS_SSALPINV = FALSE
 *     AVIS_SSAPHITARGET = FALSE
 *     AVIS_SSADEFINED = FALSE
 *     AVIS_SSATHEN = FALSE
 *     AVIS_SSAELSE = FALSE
 *     AVIS_NEEDCOUNT = 0
 *     AVIS_SUBST = NULL
 *     AVIS_SUBSTUSSA = NULL
 *
 * remark:
 *   when creating a new ssacounter node this node is stored in the toplevel
 *   block of the given fundef (sideeffekt!!!)
 *
 ******************************************************************************/
node *
AdjustAvisData (node *new_vardec, node *fundef)
{
    node *avis_node;
    char *base_id;
    node *ssacnt;

    DBUG_ENTER ("AdjustAvisData");

    DBUG_ASSERT ((fundef != NULL), "missing fundef");
    DBUG_ASSERT ((FUNDEF_BODY (fundef) != NULL), "missing body in fundef");

    avis_node = VARDEC_AVIS (new_vardec);

    /* SSACOUNTER operations are only necessary when operating on ssa form */
    if (valid_ssaform) {
        DBUG_ASSERT ((AVIS_SSACOUNT (VARDEC_AVIS (new_vardec)) != NULL),
                     "corrupted ssa form found - unknown baseid");

        base_id = SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (new_vardec)));

        /*
         * first we check if there is already an ssacounter with this baseid in
         * the target fundef so we attach our new created avis to this ssa counter.
         * by doing this we avoid name conflicts when renaming during ssa updates
         */
        ssacnt = BLOCK_SSACOUNTER (FUNDEF_BODY (fundef));
        while ((ssacnt != NULL) && (strcmp (SSACNT_BASEID (ssacnt), base_id) != 0)) {
            ssacnt = SSACNT_NEXT (ssacnt);
        }

        if (ssacnt != NULL) {
            /* found a matching base id */
            AVIS_SSACOUNT (avis_node) = ssacnt;
            DBUG_PRINT ("AAD", ("use existing ssacounter with baseid %s for vardec %s",
                                base_id, VARDEC_NAME (new_vardec)));

        } else {
            /*
             * if we find no matching ssacounter we create a new one with the current
             * base id and add it to the ssacounter chain
             */
            DBUG_PRINT ("AAD", ("reuse base_id %s for vardec %s", base_id,
                                VARDEC_NAME (new_vardec)));

            BLOCK_SSACOUNTER (FUNDEF_BODY (fundef))
              = MakeSSAcnt (BLOCK_SSACOUNTER (FUNDEF_BODY (fundef)), 0,
                            StringCopy (base_id));

            AVIS_SSACOUNT (avis_node) = BLOCK_SSACOUNTER (FUNDEF_BODY (fundef));
        }
    } else {
        /* no ssacounter needed */
        AVIS_SSACOUNT (avis_node) = NULL;
    }
    AVIS_SSALPINV (avis_node) = FALSE;
    AVIS_SSAPHITARGET (avis_node) = FALSE;
    AVIS_SSADEFINED (avis_node) = FALSE;
    AVIS_SSATHEN (avis_node) = FALSE;
    AVIS_SSAELSE (avis_node) = FALSE;
    AVIS_NEEDCOUNT (avis_node) = 0;
    AVIS_SUBST (avis_node) = NULL;
    AVIS_SUBSTUSSA (avis_node) = NULL;

    DBUG_RETURN (avis_node);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/******************************************************************************
 *
 * function:
 *   int CountArgs( node *args)
 *
 * description:
 *   Counts the number of N_arg nodes.
 *
 ******************************************************************************/

int
CountArgs (node *args)
{
    int count = 0;

    DBUG_ENTER ("CountArgs");

    while (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_arg node found!");
        count++;
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * Function:
 *   int CmpDomain(node *arg1, node *arg2)
 *
 * Description:
 *
 *
 ******************************************************************************/

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
        DBUG_ASSERT (((NODE_TYPE (arg1) == N_arg) && (NODE_TYPE (arg2) == N_arg)),
                     "arguments must be N_arg chains!");

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
                    for (i = 0; i < ARG_DIM (arg1); i++) {
                        if (ARG_SHAPE (arg1, i) != ARG_SHAPE (arg2, i)) {
                            break;
                        }
                    }
                    if (i != ARG_DIM (arg1)) {
                        break;
                    } else {
                        arg1 = ARG_NEXT (arg1);
                        arg2 = ARG_NEXT (arg2);
                    }
                } else {
                    arg1 = ARG_NEXT (arg1);
                    arg2 = ARG_NEXT (arg2);
                }
            } else {
                break;
            }
        } else {
            break;
        }
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
 *  description   : returns ptr to N_vardec - or N_arg - node  if variable or argument
 *                  with same name as in 1) has been found, else NULL
 *
 */

node *
SearchDecl (char *name, node *decl_node)
{
    node *found = NULL;

    DBUG_ENTER ("SearchDecl");

    while (NULL != decl_node) {
        if (N_vardec == NODE_TYPE (decl_node)) {
            if (!strcmp (name, VARDEC_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = VARDEC_NEXT (decl_node);
            }
        } else {
            if (!strcmp (name, ARG_NAME (decl_node))) {
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
 * function:
 *   node *AppendAssign( node *assign_chain, node *assign)
 *
 * description:
 *   Appends 'assign' to 'assing_chain' and returns the new chain.
 *   If 'assign_chain' was a N_empty node, this node is removed first.
 *
 ******************************************************************************/

node *
AppendAssign (node *assign_chain, node *assign)
{
    node *ret;

    DBUG_ENTER ("AppendAssign");

    if ((assign != NULL) && (assign_chain != NULL)) {
        if (NODE_TYPE (assign_chain) == N_empty) {
            /* empty block */
            FreeNode (assign_chain);
            assign_chain = NULL;
        }
    }

    APPEND (ret, node *, ASSIGN, assign_chain, assign);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAssignLet(char *var_name, node *vardec_node, node *let_expr)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
MakeAssignLet (char *var_name, node *vardec_node, node *let_expr)
{
    ids *tmp_ids;
    node *tmp_node;

    DBUG_ENTER ("MakeAssignLet");

    tmp_ids = MakeIds (var_name, NULL, ST_regular);
    IDS_VARDEC (tmp_ids) = vardec_node;
    IDS_AVIS (tmp_ids) = VARDEC_OR_ARG_AVIS (vardec_node);
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

/******************************************************************************
 *
 * Function:
 *   node *GetCompoundNode( node *arg_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
GetCompoundNode (node *arg_node)
{
    node *compound_node;

    DBUG_ENTER ("GetCompoundNode");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "GetCompoundNode() can handle N_assign nodes only!");

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
        if (N_Nwith == NODE_TYPE (arg_node)) {
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
 *   node *AppendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends an new ICM with name and args given as an assign to the given
 *   chain of assignments assign.
 *
 ******************************************************************************/

node *
AppendAssignIcm (node *assign, char *name, node *args)
{
    node *result;

    DBUG_ENTER ("AppendAssignIcm");

    result = AppendAssign (assign, MakeAssignIcm1 (name, args));

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/******************************************************************************
 *
 * function:
 *   node *ExprsConcat( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

node *
ExprsConcat (node *exprs1, node *exprs2)
{
    node *ret;

    DBUG_ENTER ("ExprsConcat");

    APPEND (ret, node *, EXPRS, exprs1, exprs2);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *MakeExprsNum( int num)
 *
 * description:
 *   Makes an N_exprs with a N_num as EXPR, NEXT is NULL.
 *
 ******************************************************************************/

node *
MakeExprsNum (int num)
{
    node *result;

    DBUG_ENTER ("MakeExprsNum");

    result = MakeExprs (MakeNum (num), NULL);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int CountExprs( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

int
CountExprs (node *exprs)
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
 *   node *CreateZeroScalar( simpletype btype)
 *
 * description:
 *   Returns a scalar 0.
 *
 ******************************************************************************/

node *
CreateZeroScalar (simpletype btype)
{
    node *ret_node;

    DBUG_ENTER ("CreateZeroScalar");

    DBUG_ASSERT ((btype != T_user), "unresolved user-type found");
    DBUG_ASSERT ((btype != T_hidden), "hidden-type found");

    switch (btype) {
    case T_int:
        ret_node = MakeNum (0);
        break;
    case T_float:
        ret_node = MakeFloat (0);
        break;
    case T_double:
        ret_node = MakeDouble (0);
        break;
    case T_bool:
        ret_node = MakeBool (0);
        break;
    case T_char:
        ret_node = MakeChar ('\0');
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
 *   node *CreateZeroVector( int length, simpletype btype)
 *
 * description:
 *   Returns an N_array node with 'length' components, each 0.
 *
 ******************************************************************************/

node *
CreateZeroVector (int length, simpletype btype)
{
    node *ret_node, *exprs_node;
    int i;
    shpseg *shpseg;

    DBUG_ENTER ("CreateZeroVector");

    DBUG_ASSERT ((btype != T_user), "unresolved user-type found");
    DBUG_ASSERT ((btype != T_hidden), "hidden-type found");

    exprs_node = NULL;
    for (i = 0; i < length; i++) {
        exprs_node = MakeExprs (CreateZeroScalar (btype), exprs_node);
    }

    ret_node = MakeArray (exprs_node);
    ARRAY_ISCONST (ret_node) = TRUE;
    ARRAY_VECTYPE (ret_node) = btype;
    ARRAY_VECLEN (ret_node) = length;

    switch (btype) {
    case T_int:
        ((int *)ARRAY_CONSTVEC (ret_node)) = Array2IntVec (exprs_node, NULL);
        break;
    case T_float:
        ((float *)ARRAY_CONSTVEC (ret_node)) = Array2FloatVec (exprs_node, NULL);
        break;
    case T_double:
        ((double *)ARRAY_CONSTVEC (ret_node)) = Array2DblVec (exprs_node, NULL);
        break;
    case T_bool:
        ((int *)ARRAY_CONSTVEC (ret_node)) = Array2BoolVec (exprs_node, NULL);
        break;
    case T_char:
        ((char *)ARRAY_CONSTVEC (ret_node)) = Array2CharVec (exprs_node, NULL);
        break;
    default:
        DBUG_ASSERT (0, ("unkown basetype found"));
    }

    /* nums struct is freed inside of MakeShpseg() */
    shpseg = MakeShpseg (MakeNums (length, NULL));
    ARRAY_TYPE (ret_node) = MakeTypes (btype, 1, shpseg, NULL, NULL);

    DBUG_RETURN (ret_node);
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
    int *intvec;
    int j;
    node *tmp = aelems;
    int i = 0;

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
    if (length != NULL) {
        *length = i;
    }

    intvec = MALLOC (i * sizeof (int));

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
    if (length != NULL) {
        *length = i;
    }

    intvec = MALLOC (i * sizeof (int));

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
    if (length != NULL) {
        *length = i;
    }

    charvec = MALLOC (i * sizeof (char));

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
    if (length != NULL) {
        *length = i;
    }

    floatvec = MALLOC (i * sizeof (float));

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
    if (length != NULL) {
        *length = i;
    }

    dblvec = MALLOC (i * sizeof (double));

    for (j = 0; j < i; j++) {
        dblvec[j] = DOUBLE_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (dblvec);
}

/* type dispatch for some standard simpletypes */
void *
Array2Vec (simpletype t, node *aelems, int *length)
{
    void *res;

    DBUG_ENTER ("Array2Vec");

    switch (t) {
    case T_int:
        res = (void *)Array2IntVec (aelems, length);
        break;
    case T_float:
        res = (void *)Array2FloatVec (aelems, length);
        break;
    case T_double:
        res = (void *)Array2DblVec (aelems, length);
        break;
    case T_bool:
        res = (void *)Array2BoolVec (aelems, length);
        break;
    case T_char:
        res = (void *)Array2CharVec (aelems, length);
        break;
    default:
        DBUG_ASSERT ((0), "unknown type for array");
        res = NULL;
    }

    DBUG_RETURN (res);
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
 *   node *MakePrf1( prf prf, node *arg1)
 *   node *MakePrf2( prf prf, node *arg1, node *arg2)
 *   node *MakePrf3( prf prf, node *arg1, node *arg2, node *arg3)
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
 ***  N_ap :
 ***/

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
 *   node *CombineExprs( node *first, node *second)
 *
 * description:
 *   This function is used only within MakeIcmXXX.
 *   'first' and 'second' are N_exprs chains or expression nodes (N_id, N_num,
 *   ...) that will be conactenated to a single N_exprs chain.
 *
 ******************************************************************************/

static node *
CombineExprs (node *first, node *second)
{
    node *result;

    DBUG_ENTER ("CombineExprs");

    if (first != NULL) {
        if (NODE_TYPE (first) != N_exprs) {
            result = MakeExprs (first, second);
        } else {
            result = ExprsConcat (first, second);
        }
    } else if (second != NULL) {
        if (NODE_TYPE (second) != N_exprs) {
            result = MakeExprs (second, NULL);
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
 *   node *MakeIcm0(char *name)
 *   node *MakeIcm1(char *name,
 *                  node *arg1)
 *   node *MakeIcm3(char *name,
 *                  node *arg1, node *arg2)
 *   node *MakeIcm4(char *name,
 *                  node *arg1, node *arg2, node *arg3, node *arg4)
 *   node *MakeIcm5(char *name,
 *                  node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
 *
 * description:
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of ICM arguments (i.e.,
 *   a single expression node (N_id, N_num, ...) or a N_exprs chain!)
 *   These are concatenated correctly.
 *
 ******************************************************************************/

node *
MakeIcm0 (char *name)
{
    node *icm;

    DBUG_ENTER ("MakeIcm0");

    icm = MakeIcm (name, NULL);

    DBUG_RETURN (icm);
}

node *
MakeIcm1 (char *name, node *arg1)
{
    node *icm;

    DBUG_ENTER ("MakeIcm1");

    arg1 = CombineExprs (arg1, NULL);
    icm = MakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
MakeIcm2 (char *name, node *arg1, node *arg2)
{
    node *icm;

    DBUG_ENTER ("MakeIcm2");

    arg2 = CombineExprs (arg2, NULL);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1);

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
    icm = MakeIcm (name, arg1);

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
    icm = MakeIcm (name, arg1);

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
    icm = MakeIcm (name, arg1);

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
    icm = MakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
MakeIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
          node *arg6, node *arg7)
{
    node *icm;

    DBUG_ENTER ("MakeIcm7");

    arg7 = CombineExprs (arg7, NULL);
    arg6 = CombineExprs (arg6, arg7);
    arg5 = CombineExprs (arg5, arg6);
    arg4 = CombineExprs (arg4, arg5);
    arg3 = CombineExprs (arg3, arg4);
    arg2 = CombineExprs (arg2, arg3);
    arg1 = CombineExprs (arg1, arg2);
    icm = MakeIcm (name, arg1);

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
 ***  N_Nwith :  *and*  N_Nwith2 :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLseg :  *and*  N_WLsegVar :
 ***/

/******************************************************************************
 *
 * Function:
 *   node *MakeWLsegX( int dims, node *contents, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
MakeWLsegX (int dims, node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLsegX");

    if (AllStridesAreConstant (contents, TRUE, TRUE)) {
        new_node = MakeWLseg (dims, contents, next);
    } else {
        new_node = MakeWLsegVar (dims, contents, next);
    }

    DBUG_RETURN (new_node);
}
