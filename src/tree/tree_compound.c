/*
 *
 * $Log$
 * Revision 3.131  2005/04/19 17:34:57  ktr
 * removed AVIS_SSAASSIGN2, AVIS_SUBSTUSSA
 *
 * Revision 3.130  2005/01/11 14:06:14  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.129  2004/12/19 23:16:37  ktr
 * removed TCcountFunctionParams
 *
 * Revision 3.128  2004/12/17 18:58:58  ktr
 * TCgetNthExpr reverted to old counting (n=1 yields the first expression).
 *
 * Revision 3.127  2004/12/17 09:27:20  khf
 * TCgetNthExpr corrected
 *
 * Revision 3.126  2004/12/08 18:15:24  ktr
 * usage of SHmakeShape corrected.
 *
 * Revision 3.125  2004/12/08 18:02:40  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 3.124  2004/12/08 11:40:50  ktr
 * nothing really changed.
 *
 * Revision 3.123  2004/12/07 20:35:53  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 3.122  2004/12/06 17:31:50  sbs
 * eliminated infinite recursion ....
 *
 * Revision 3.121  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 3.120  2004/12/01 16:31:18  ktr
 * Some cleanup
 *
 * Revision 3.119  2004/11/30 21:44:20  ktr
 * TCappendVardecs fixed.
 *
 * Revision 3.118  2004/11/27 01:42:07  mwe
 * SearchDecl to TCsearchDecl
 *
 * Revision 3.117  2004/11/27 00:40:03  khf
 * adjusted names of TCmakePrf*
 *
 * Revision 3.116  2004/11/26 21:52:28  ktr
 * LiftArg removed.
 *
 * Revision 3.115  2004/11/26 16:59:19  skt
 * some includes removed
 *
 * Revision 3.114  2004/11/26 16:53:42  skt
 * killed TCgetArgtabIndexOut/In during SDC2k4
 *
 * Revision 3.113  2004/11/26 14:11:47  skt
 * made it compilable during SACDevCampDK 2k4
 *
 * Revision 3.112  2004/11/26 02:17:26  sah
 * fixed a comment
 *
 * Revision 3.111  2004/11/26 00:04:56  skt
 * some changes during SACDevCampDK 2k4 - go on at HERE
 *
 * Revision 3.110  2004/11/25 15:43:26  skt
 * some brushing, incl. removing of node_compat.h
 *
 * Revision 3.109  2004/11/25 12:13:08  mwe
 * *** empty log message ***
 *
 * Revision 3.108  2004/11/24 17:24:46  ktr
 * Added TCcountParts
 *
 * Revision 3.107  2004/11/24 16:41:56  skt
 * FindVardec_Name deleted
 *
 * Revision 3.106  2004/11/24 16:27:42  ktr
 * Changed signature of MakeAssignLet
 *
 * Revision 3.105  2004/11/24 12:33:33  ktr
 * TCappendRets added.
 *
 * Revision 3.104  2004/11/24 11:45:14  sah
 * added TCgetNthExpr
 *
 * Revision 3.103  2004/11/24 10:56:11  sah
 * *** empty log message ***
 *
 * Revision 3.102  2004/11/24 10:50:14  sbs
 * TCmakeIdsFromVardecs added
 *
 * Revision 3.101  2004/11/23 22:36:58  sbs
 * TCcountRets added.
 *
 * Revision 3.100  2004/11/23 22:27:57  khf
 * added TCmakeFlatArray
 *
 * Revision 3.99  2004/11/23 22:18:50  skt
 * code brushing during SACDevCampDK 2k4
 *
 * Revision 3.98  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * [...]
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
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

/*****************************************************************************/
/**
 **  some ugly macros (better use modul CompareTree!), moved from
 **  tree_compound.h
 **/

/*
 *  macro name    : CMP_TYPE_USER
 *  arg types     : 1) types*
 *                  2) types*
 *  result type   : int
 *  description   : compares two user-defined types (name and module)
 *                  Names and module names must be equal.
 *  remarks       : result: 1 - equal, 0 - not equal
 */

#define CMP_TYPE_USER(a, b)                                                              \
    ((!ILIBstringCompare (TYPES_NAME (a), TYPES_NAME (b)))                               \
     && (!ILIBstringCompare (STR_OR_EMPTY (TYPES_MOD (a)),                               \
                             STR_OR_EMPTY (TYPES_MOD (b)))))

/*
 *  macro name    : CMP_TYPEDEF(a,b)
 *  arg types     : 1) node*  (N_typedef)
 *                  2) node*  (N_typedef)
 *  result type   : int
 *  description   : compares two typedef nodes (name and module)
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_TYPEDEF(a, b)                                                                \
    ((NULL == TYPEDEF_MOD (a))                                                           \
       ? (!ILIBstringCompare (TYPEDEF_NAME (a), TYPEDEF_NAME (b))                        \
          && (NULL == TYPEDEF_MOD (b)))                                                  \
       : ((NULL == TYPEDEF_MOD (b))                                                      \
            ? 0                                                                          \
            : ((!ILIBstringCompare (TYPEDEF_NAME (a), TYPEDEF_NAME (b)))                 \
               && (!ILIBstringCompare (TYPEDEF_MOD (a), TYPEDEF_MOD (b))))))

/*
 *  macro name    : CMP_TYPE_TYPEDEF(name, mod, typedef)
 *  arg types     : 1) char*
 *                  2) char*
 *                  3) node*  (N_typedef)
 *  result type   : int
 *  description   : compares name and module name of a type with the
 *                  defined name and module name of a typedef
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_TYPE_TYPEDEF(name, mod, tdef)                                                \
    ((!ILIBstringCompare (name, TYPEDEF_NAME (tdef)))                                    \
     && (!ILIBstringCompare (STR_OR_EMPTY (mod), STR_OR_EMPTY (TYPEDEF_MOD (tdef)))))

/*
 *  macro name    : CMP_OBJ_OBJDEF
 *  arg types     : 1) char*
 *                  2) char*
 *                  3) node*  (N_objdef)
 *  result type   : int
 *  description   : compares name and module name of an object with the
 *                  defined name and module name of an objdef
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_OBJ_OBJDEF(name, mod, odef)                                                  \
    ((mod == NULL) ? (!ILIBstringCompare (name, OBJDEF_NAME (odef)))                     \
                   : ((!ILIBstringCompare (name, OBJDEF_NAME (odef)))                    \
                      && (!ILIBstringCompare (mod, OBJDEF_MOD (odef)))))

/*
 *  macro name    : CMP_FUN_ID(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two fundef nodes (name and module only)
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_FUN_ID(a, b)                                                                 \
    ((!ILIBstringCompare (FUNDEF_NAME (a), FUNDEF_NAME (b)))                             \
     && (!ILIBstringCompare (STR_OR_EMPTY (FUNDEF_MOD (a)),                              \
                             STR_OR_EMPTY (FUNDEF_MOD (b)))))

/*****************************************************************************/

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

    array_node = TCmakeFlatArray (next);

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

        if ((tdef == NULL) && (global.compiler_phase <= PH_typecheck)) {
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

#if 0 /* TODO - macros for old types */
    DBUG_ASSERT( (TYPEDEF_BASETYPE( tdef) != T_user),
                 "unresolved nested user-defined type found");

    if (TYPEDEF_BASETYPE( tdef) == T_hidden) {
      /*
       * Basic type is hidden therefore we have to use the original type
       * structure and rely on the belonging typedef!!
       */
      res_type = type;
    }
    else {
      res_type = TYPEDEF_TYPE( tdef);
    }
#endif
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
 *   int TCcompareTypesImplementation( types *t1, types *t2)
 *
 * Description:
 *   compares two types for having the equal implementation types (resolving
 *   user defined types) - that means double[2] and Complex are equal!
 *
 *   return value:
 *     (t1 == t2)   ->   0
 *     (t1 <  t2)   ->  -1
 *     (t1 >  t2)   ->   1
 *     (t1 != t2)   ->   2
 *
 ******************************************************************************/

int
TCcompareTypesImplementation (types *t1, types *t2)
{
    int res;
    shpseg *shpseg1, *shpseg2;
    int shpdim1, shpdim2;
    int dim;

    DBUG_ENTER ("TCcompareTypesImplementation");

    DBUG_ASSERT (((t1 != NULL) && (t2 != NULL)),
                 "TCcompareTypesImplementation() called with NULL type");

    if (TCgetBasetype (t1) == TCgetBasetype (t2)) {
        shpdim1 = TCgetShapeDim (t1);
        shpdim2 = TCgetShapeDim (t2);

        if (shpdim1 == shpdim2) {
            if (KNOWN_SHAPE (shpdim1)) { /* -> KNOWN_SHAPE( shpdim2) */
                shpseg1 = TCtype2Shpseg (t1, &dim);
                shpseg2 = TCtype2Shpseg (t2, NULL);

                res = (TCequalShpseg (dim, shpseg1, shpseg2)) ? 0 : 2;

                if (shpseg1 != NULL) {
                    shpseg1 = FREEfreeShpseg (shpseg1);
                }
                if (shpseg2 != NULL) {
                    shpseg2 = FREEfreeShpseg (shpseg2);
                }
            } else {
                res = 0;
            }
        } else if (shpdim1 == ARRAY_OR_SCALAR) {
            res = 1;
        } else if (shpdim2 == ARRAY_OR_SCALAR) {
            res = -1;
        } else if (shpdim1 == UNKNOWN_SHAPE) {
            res = (shpdim2 == SCALAR) ? 2 : 1;
        } else if (shpdim2 == UNKNOWN_SHAPE) {
            res = (shpdim1 == SCALAR) ? 2 : -1;
        } else if (shpdim1 < KNOWN_DIM_OFFSET) {
            res = (DIM_NO_OFFSET (shpdim1) != DIM_NO_OFFSET (shpdim2)) ? 2 : 1;
        } else if (shpdim2 < KNOWN_DIM_OFFSET) {
            res = (DIM_NO_OFFSET (shpdim1) != DIM_NO_OFFSET (shpdim2)) ? 2 : -1;
        } else {
            DBUG_ASSERT ((0), "illegal shape constellation found!");
            res = 0; /* just to please gcc */
        }
    } else {
        res = 2;
    }

    DBUG_RETURN (res);
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

/******************************************************************************
 *
 * Function:
 *   node *TCcreateZeroFromType( types *type, bool unroll, node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
TCcreateZeroFromType (types *type, bool unroll, node *fundef)
{
    node *zero;
    shpseg *shape;
    simpletype btype;
    int dim;

    DBUG_ENTER ("CreateZeroFromType");

    shape = TCtype2Shpseg (type, &dim);
    btype = TCgetBasetype (type);
    zero = TCcreateZero (dim, shape, btype, unroll, fundef);
    if (shape != NULL) {
        shape = FREEfreeShpseg (shape);
    }

    DBUG_RETURN (zero);
}

/******************************************************************************
 *
 * function:
 *   bool IsArray( types *type)
 *   bool IsHidden( types *type)
 *   bool IsUnique( types *type)
 *   bool IsNonUniqueHidden( types *type)
 *   bool IsBoxed( types *type)
 *
 * description:
 *   These functions may be used to check for particular properties
 *   of a given data type.
 *
 ******************************************************************************/

/*
 * TCisArray := Is not a (static) scalar
 */
bool
TCisArray (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsArray");

    if (TYPES_DIM (type) != SCALAR) {
        ret = TRUE;
    } else if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

#if 0 /* TODO - macro for old types */
    if (TYPEDEF_DIM( tdef) != SCALAR) {
      ret = TRUE;
    }
#endif
    }

    DBUG_RETURN (ret);
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

#if 0 /* TODO - macros for old types */
    if (TYPEDEF_BASETYPE( tdef) == T_hidden) {
      ret = TRUE;
    }
#endif
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

bool
TCisNonUniqueHidden (types *type)
{
    bool ret;

    DBUG_ENTER ("IsNonUniqueHidden");

    ret = (TCisHidden (type) && (!TCisUnique (type)));

    DBUG_RETURN (ret);
}

bool
TCisBoxed (types *type)
{
    bool ret;

    DBUG_ENTER ("IsBoxed");

    ret = (TCisHidden (type) || TCisArray (type));

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
TCsearchTypedef (char *name, char *mod, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("TCsearchTypedef");

    tmp = implementations;
    while ((tmp != NULL) && (CMP_TYPE_TYPEDEF (name, mod, tmp) == 0)) {
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
TCsearchObjdef (const char *name, char *mod, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("TCsearchObjdef");

    tmp = implementations;
    while ((tmp != NULL) && (CMP_OBJ_OBJDEF (name, mod, tmp) == 0)) {
        tmp = OBJDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
}

void
TCobjList2ArgList (node *objdef)
{
    node *tmp;

    DBUG_ENTER ("TCobjList2ArgList");

    tmp = objdef;
    while (tmp != NULL) {
        NODE_TYPE (tmp) = N_arg;
        ARG_NEXT (tmp) = OBJDEF_NEXT (tmp);
        tmp = ARG_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

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
 *   node *TCmakeVardecFromArg( node *arg)
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
TCmakeVardecFromArg (node *arg_node)
{
    node *new_vardec;

    DBUG_ENTER ("MakeVardecFromArg");

    new_vardec = TBmakeVardec (DUPdoDupNode (ARG_AVIS (arg_node)), NULL);

    if (ARG_TYPE (arg_node) != NULL) {
        VARDEC_TYPE (new_vardec) = DUPdupAllTypes (ARG_TYPE (arg_node));
    }

    AVIS_DECL (VARDEC_AVIS (new_vardec)) = new_vardec;

    /* delete wrong data in copied AVIS node */
    AVIS_SSAASSIGN (VARDEC_AVIS (new_vardec)) = NULL;
    AVIS_SSALPINV (VARDEC_AVIS (new_vardec)) = FALSE;
    AVIS_SSASTACK_TOP (VARDEC_AVIS (new_vardec)) = NULL;

    DBUG_RETURN (new_vardec);
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeArgFromVardec( node *varde_node)
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
TCmakeArgFromVardec (node *vardec_node)
{
    node *new_arg;
    node *new_avis;

    DBUG_ENTER ("TCmakeArgFromVardec");

    new_avis = DUPdoDupNode (VARDEC_AVIS (vardec_node));

    /* delete wrong data in copied AVIS node */
    AVIS_SSAASSIGN (new_avis) = NULL;
    AVIS_SSALPINV (new_avis) = FALSE;
    AVIS_SSASTACK_TOP (new_avis) = NULL;

    new_arg = TBmakeArg (new_avis, NULL);

    if (VARDEC_TYPE (vardec_node) != NULL) {
        ARG_TYPE (new_arg) = DUPdupAllTypes (VARDEC_TYPE (vardec_node));
    }

    AVIS_DECL (ARG_AVIS (new_arg)) = new_arg;

    DBUG_RETURN (new_arg);
}

/******************************************************************************
 *
 * function:
 *   node *TCadjustAvisData( node *new_vardec, node *fundef)
 *
 * description:
 *   when a vardec is duplicated via DupTree all dependend infomation in the
 *   corresponding avis node is duplicated, too. when this vardec is used in
 *   the same fundef as the original one everything is good, but if the
 *   duplicated vardec should be used in a different fundef the fundef related
 *   attributes have to be adjusted by this function:
 *     AVIS_SSACOUNT = (new fresh ssacnt node)
 *     AVIS_SSALPINV = FALSE
 *     AVIS_SSADEFINED = FALSE
 *     AVIS_SSATHEN = FALSE
 *     AVIS_SSAELSE = FALSE
 *     AVIS_NEEDCOUNT = 0
 *     AVIS_SUBST = NULL
 *
 * remark:
 *   when creating a new ssacounter node this node is stored in the toplevel
 *   block of the given fundef (sideeffekt!!!)
 *
 ******************************************************************************/
node *
TCadjustAvisData (node *new_vardec, node *fundef)
{
    node *avis_node;
    char *base_id;
    node *ssacnt;

    DBUG_ENTER ("TCadjustAvisData");

    DBUG_ASSERT ((fundef != NULL), "missing fundef");
    DBUG_ASSERT ((FUNDEF_BODY (fundef) != NULL), "missing body in fundef");

    avis_node = VARDEC_AVIS (new_vardec);

    /* SSACOUNTER operations are only necessary when operating on ssa form */
    if (global.valid_ssaform) {
        DBUG_ASSERT ((AVIS_SSACOUNT (VARDEC_AVIS (new_vardec)) != NULL),
                     "corrupted ssa form found - unknown baseid");

        base_id = SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (new_vardec)));
        DBUG_ASSERT ((base_id != NULL), "no BASEID found in AVIS!");

        /*
         * first we check if there is already an ssacounter with this baseid in
         * the target fundef so we attach our new created avis to this ssa counter.
         * by doing this we avoid name conflicts when renaming during ssa updates
         */
        ssacnt = BLOCK_SSACOUNTER (FUNDEF_BODY (fundef));
        while ((ssacnt != NULL) && ILIBstringCompare (SSACNT_BASEID (ssacnt), base_id)) {
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
              = TBmakeSsacnt (0, ILIBstringCopy (base_id),
                              BLOCK_SSACOUNTER (FUNDEF_BODY (fundef)));

            AVIS_SSACOUNT (avis_node) = BLOCK_SSACOUNTER (FUNDEF_BODY (fundef));
        }
    } else {
        /* no ssacounter needed */
        AVIS_SSACOUNT (avis_node) = NULL;
    }
    AVIS_SSALPINV (avis_node) = FALSE;
    AVIS_SSADEFINED (avis_node) = FALSE;
    AVIS_SSATHEN (avis_node) = FALSE;
    AVIS_SSAELSE (avis_node) = FALSE;
    AVIS_NEEDCOUNT (avis_node) = 0;
    AVIS_SUBST (avis_node) = NULL;

    DBUG_RETURN (avis_node);
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

/****************************************************************************
 *
 * function:
 *   node *TCreturnTypes2Ret(types* type)
 *
 * description:
 *   returns a chain of N_ret nodes according to type
 *
 ***************************************************************************/

node *
TCreturnTypes2Ret (types *type)
{
    node *rets, *tmp, *tmp2;

    DBUG_ENTER ("TCreturnTypes2Ret");

    rets = NULL;

    if (type != NULL) {
        tmp2 = rets = TBmakeRet (TYoldType2Type (type), NULL);
        if (TYPES_STATUS (type) == ST_artificial) {
            RET_ISARTIFICIAL (rets) = TRUE;
        }
        type = TYPES_NEXT (type);
        while (type != NULL) {
            tmp = TBmakeRet (TYoldType2Type (type), NULL);
            if (TYPES_STATUS (type) == ST_artificial) {
                RET_ISARTIFICIAL (tmp) = TRUE;
            }
            RET_NEXT (tmp2) = tmp;
            tmp2 = tmp;
            type = TYPES_NEXT (type);
        }
    }

    DBUG_RETURN (rets);
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
 *   node *TCmakeAssignLet( node *avis, node *let_expr)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCmakeAssignLet (node *avis, node *let_expr)
{
    DBUG_ENTER ("TCmakeAssignLet");

    DBUG_RETURN (TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), let_expr), NULL));
}

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCmakeAssignInstr (node *instr, node *next)
{
    node *result;

    if (instr == NULL) {
        result = next;
    } else if (NODE_TYPE (instr) == N_assign) {
        result = TCappendAssign (instr, next);
    } else {
        result = TBmakeAssign (instr, next);
    }

    return (result);
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
 *   node *TCmakeFlatArray( node *aelems)
 *
 * description:
 *   Returns a vector.
 *
 *****************************************************************************/
node *
TCmakeFlatArray (node *aelems)
{
    DBUG_ENTER ("MakeFlatArray");

    DBUG_RETURN (TBmakeArray (SHcreateShape (1, TCcountExprs (aelems)), aelems));
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

    ret_node = TCmakeFlatArray (exprs_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   bool TCisConstArray(node *array)
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

bool
TCisConstArray (node *array)
{
    ntype *atype;
    nodetype type;
    bool isconst = FALSE;

    DBUG_ENTER ("TCisConstArray");

    if (array != NULL) {
        switch (NODE_TYPE (array)) {
        case N_array:
            atype = NTCnewTypeCheck_Expr (array);
            isconst = TYisAKV (atype);
            atype = TYfreeType (atype);

            if (!isconst) {
                /*
                 * Although ARRAY_ISCONST is false, the array still may be constant:
                 *   1.) For the time being ISCONST is false for all non-int-arrays.
                 *   2.) ISCONST may be inconsistent to this respect.
                 *       (E.g. after some optimizations new constant arrays may occur.)
                 */
                array = ARRAY_AELEMS (array);
                isconst = TRUE;
                /*
                 * remark: array may be NULL (empty array) -> constant
                 */
                while (array != NULL) {
                    type = NODE_TYPE (EXPRS_EXPR (array));
                    if ((type == N_num) || (type == N_char) || (type == N_bool)
                        || (type == N_float) || (type == N_double)) {
                        array = EXPRS_NEXT (array);
                    } else {
                        isconst = FALSE;
                        break;
                    }
                }
            }
            break;
        case N_id:
            isconst = TYisAKV (ID_NTYPE (array));
            break;
        default:
            isconst = FALSE;
        }
    } else {
        isconst = FALSE;
    }

    DBUG_RETURN (isconst);
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
 *   node *Ids2Array( ids *ids_arg)
 *
 * description:
 *   convert ids into array
 *
 *****************************************************************************/

node *
TCids2Array (node *ids_arg)
{
    node *array;

    DBUG_ENTER ("TCids2Array");

    if (ids_arg != NULL) {
        array = TCmakeFlatArray (TCids2Exprs (ids_arg));
    } else {
        array = TCmakeFlatArray (NULL);
    }

    DBUG_RETURN (array);
}

int *
TCarray2IntVec (node *aelems, int *length)
{
    int *intvec;
    int j;
    node *tmp = aelems;
    int i = 0;

    DBUG_ENTER ("TCarray2IntVec");

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

    intvec = ILIBmalloc (i * sizeof (int));

    for (j = 0; j < i; j++) {
        intvec[j] = NUM_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (intvec);
}

int *
TCarray2BoolVec (node *aelems, int *length)
{
    int *intvec, i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("TCarray2IntVec");

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

    intvec = ILIBmalloc (i * sizeof (int));

    for (j = 0; j < i; j++) {
        intvec[j] = BOOL_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (intvec);
}

char *
TCarray2CharVec (node *aelems, int *length)
{
    char *charvec;
    int i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("TCarray2CharVec");

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

    charvec = ILIBmalloc (i * sizeof (char));

    for (j = 0; j < i; j++) {
        charvec[j] = CHAR_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (charvec);
}

float *
TCarray2FloatVec (node *aelems, int *length)
{
    float *floatvec;
    int i = 0, j;
    node *tmp = aelems;

    DBUG_ENTER ("TCarray2FloatVec");

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

    floatvec = ILIBmalloc (i * sizeof (float));

    for (j = 0; j < i; j++) {
        floatvec[j] = FLOAT_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (floatvec);
}

double *
TCarray2DblVec (node *aelems, int *length)
{
    int i = 0, j;
    double *dblvec;
    node *tmp = aelems;

    DBUG_ENTER ("TCarray2DblVec");

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

    dblvec = ILIBmalloc (i * sizeof (double));

    for (j = 0; j < i; j++) {
        dblvec[j] = DOUBLE_VAL (EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    DBUG_RETURN (dblvec);
}

/* type dispatch for some standard simpletypes */
void *
TCarray2Vec (simpletype t, node *aelems, int *length)
{
    void *res;

    DBUG_ENTER ("TCarray2Vec");

    switch (t) {
    case T_int:
        res = (void *)TCarray2IntVec (aelems, length);
        break;
    case T_float:
        res = (void *)TCarray2FloatVec (aelems, length);
        break;
    case T_double:
        res = (void *)TCarray2DblVec (aelems, length);
        break;
    case T_bool:
        res = (void *)TCarray2BoolVec (aelems, length);
        break;
    case T_char:
        res = (void *)TCarray2CharVec (aelems, length);
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
 *   node *TCmakeVinfoDollar( node * next)
 *
 * description:
 *   create N_vinfo node with Dollar-Tag and self-ref in VINFO_DOLLAR!
 *
 ******************************************************************************/

node *
TCmakeVinfoDollar (node *next)
{
    node *vinfo;

    DBUG_ENTER ("TCmakeVinfoDollar");

    vinfo = TBmakeVinfo (DOLLAR, NULL, NULL, next);
    VINFO_DOLLAR (vinfo) = vinfo;

    DBUG_RETURN (vinfo);
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
TCmakeSpap1 (char *mod, char *name, node *arg1)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result = TBmakeSpap (TBmakeSpid (mod, name), TBmakeExprs (arg1, NULL));

    DBUG_RETURN (result);
}

node *
TCmakeSpap2 (char *mod, char *name, node *arg1, node *arg2)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result
      = TBmakeSpap (TBmakeSpid (mod, name), TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (result);
}

node *
TCmakeSpap3 (char *mod, char *name, node *arg1, node *arg2, node *arg3)
{
    node *result;

    DBUG_ENTER ("TCmakeSpap1");

    result
      = TBmakeSpap (TBmakeSpid (mod, name),
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

/******************************************************************************
 *
 * Function:
 *   node *TCcreateScalarWith( int dim, shpseg *shape, simpletype btype,
 *                             node *expr, node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
TCcreateScalarWith (int dim, shpseg *shape, simpletype btype, node *expr, node *fundef)
{
    node *wl;
    node *id;
    node *vardecs = NULL;
    node *vec_ids;
    node *scl_ids = NULL;
    node *new_avis;
    int i;

    DBUG_ENTER ("TCcreateScalarWith");

    DBUG_ASSERT ((dim >= 0), "TCcreateScalarWith() used with unknown shape!");

    new_avis = TBmakeAvis (ILIBtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));
    vardecs = TBmakeVardec (new_avis, vardecs);
    VARDEC_TYPE (vardecs) = TYtype2OldType (AVIS_TYPE (new_avis));
    vec_ids = TBmakeIds (new_avis, NULL);

    for (i = 0; i < dim; i++) {
        new_avis = TBmakeAvis (ILIBtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        vardecs = TBmakeVardec (new_avis, vardecs);
        VARDEC_TYPE (vardecs) = TYtype2OldType (AVIS_TYPE (new_avis));
        scl_ids = TBmakeIds (new_avis, scl_ids);
    }

    new_avis
      = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    id = TBmakeId (new_avis);
    vardecs = TBmakeVardec (new_avis, vardecs);
    VARDEC_TYPE (vardecs) = TYtype2OldType (AVIS_TYPE (new_avis));

    /* BORDER */

    wl = TBmakeWith (TBmakePart (NULL, TBmakeWithid (vec_ids, scl_ids),
                                 TBmakeGenerator (F_le, F_lt,
                                                  TCcreateZeroVector (dim, T_int),
                                                  TCshpseg2Array (shape, dim), NULL,
                                                  NULL)),
                     TBmakeCode (TBmakeBlock (TCmakeAssignLet (new_avis, expr), NULL),
                                 TBmakeExprs (id, NULL)),
                     TBmakeGenarray (TCshpseg2Array (shape, dim), NULL));
    CODE_USED (WITH_CODE (wl))++;
    PART_CODE (WITH_PART (wl)) = WITH_CODE (wl);
    WITH_PARTS (wl) = 1;

    fundef = TCaddVardecs (fundef, vardecs);

    DBUG_RETURN (wl);
}

/******************************************************************************
 *
 * Function:
 *   node *TCcreateZero( int dim, shpseg *shape, simpletype btype, bool no_wl,
 *                       node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
TCcreateZero (int dim, shpseg *shape, simpletype btype, bool no_wl, node *fundef)
{
    node *zero;
    int length = TCgetShpsegLength (dim, shape);

    DBUG_ENTER ("RCcreateZero");

    DBUG_ASSERT ((dim >= 0), "CreateZero() used with unknown shape!");

    if (dim == 0) {
        zero = TCcreateZeroScalar (btype);
    } else if (no_wl && (dim == 1)) {
        zero = TCcreateZeroVector (length, btype);
    } else {
        if (no_wl) {
            DBUG_ASSERT ((length <= global.wlunrnum),
                         "CreateZero(): Vector is too long!");

            zero
              = TBmakePrf (F_reshape,
                           TBmakeExprs (TCshpseg2Array (shape, dim),
                                        TBmakeExprs (TCcreateZeroVector (length, btype),
                                                     NULL)));
        } else {
            zero = TCcreateScalarWith (dim, shape, btype, TCcreateZeroScalar (btype),
                                       fundef);
        }
    }

    DBUG_RETURN (zero);
}

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
 *** N_linklist
 ***/

int
TCaddLinkToLinks (node **links, node *link)
{
    int result = 0;

    DBUG_ENTER ("TCaddLinkToLinks");

    if (*links == NULL) {
        /*
         * it has not been found so far, so append it
         */
        *links = TBmakeLinklist (link, NULL);
        result = 1;
    } else if (LINKLIST_LINK (*links) != link) {
        /*
         * its not the current one, so go on
         */
        result = TCaddLinkToLinks (&LINKLIST_NEXT (*links), link);
    }

    DBUG_RETURN (result);
}

int
TCaddLinksToLinks (node **links, node *add)
{
    int result = 0;

    DBUG_ENTER ("TCaddLinksToLinks");

    while (add != NULL) {
        result += TCaddLinkToLinks (links, LINKLIST_LINK (add));
        add = LINKLIST_NEXT (add);
    }

    DBUG_RETURN (result);
}

bool
TClinklistContains (node *set, node *link)
{
    bool result = FALSE;

    DBUG_ENTER ("TClinklistContains");

    while ((set != NULL) && (!result)) {
        DBUG_ASSERT ((NODE_TYPE (set) == N_linklist),
                     "called TClinklistContains with non N_linklist node!");

        result = (LINKLIST_LINK (set) == link);

        set = LINKLIST_NEXT (set);
    }

    DBUG_RETURN (result);
}

bool
TClinklistTCisSubset (node *super, node *sub)
{
    bool result = TRUE;

    DBUG_ENTER ("TClinklistTCisSubset");

    while ((sub != NULL) && result) {
        DBUG_ASSERT ((NODE_TYPE (sub) == N_linklist),
                     "called TClinklistTCisSubset with non N_linklist node!");

        result = result && TClinklistContains (super, LINKLIST_LINK (sub));

        sub = LINKLIST_NEXT (sub);
    }

    DBUG_RETURN (result);
}
