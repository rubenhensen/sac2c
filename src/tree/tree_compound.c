/*
 *
 * $Log$
 * Revision 3.77  2003/06/13 09:27:27  ktr
 * AdjustVectorShape now checks whether it was called with a NULL argument
 *
 * Revision 3.76  2003/06/11 22:03:09  ktr
 * ARRAY_SHAPE added.
 *
 * Revision 3.75  2002/10/30 16:31:07  dkr
 * HasDotTypes(), HasDotArgs() modified
 *
 * Revision 3.74  2002/10/10 23:51:53  dkr
 * no changes done
 *
 * Revision 3.73  2002/10/10 00:43:23  dkr
 * bug in CompareTypesImplementation() fixed
 *
 * Revision 3.72  2002/09/25 11:34:34  sbs
 * CompareTypesImplementation now allways initializes res !!!
 *
 * Revision 3.71  2002/09/06 10:03:15  sbs
 * Ids2Exprs added.
 *
 * Revision 3.70  2002/09/03 12:06:19  dkr
 * CompareTypesImplementation(): comment corrected
 *
 * Revision 3.69  2002/09/03 11:09:19  dkr
 * CompareTypesImplementation() modified
 *
 * Revision 3.68  2002/08/05 17:03:45  sbs
 * several extensions required for the alpha version of the new type checker
 *
 * Revision 3.67  2002/08/03 00:35:13  dkr
 * DBUG-output for CreateScalarWith() corrected
 *
 * Revision 3.66  2002/07/24 13:19:55  dkr
 * macros MUST_REFCOUNT renamed
 *
 * Revision 3.65  2002/07/15 17:25:07  dkr
 * LiftArg() moved from precompile.c to tree_compound.[ch]
 *
 * Revision 3.64  2002/07/12 09:07:16  dkr
 * CreateSel: no warning about uninitialized variable anymore :-)
 *
 * Revision 3.62  2002/06/27 16:57:00  dkr
 * signature of CreateSel() modified
 *
 * Revision 3.61  2002/06/27 13:32:04  dkr
 * - Ids2Array() added
 * - CreateSel() modified
 *
 * Revision 3.60  2002/06/27 11:01:55  dkr
 * - CreateScalarWith() and CreateSel() added
 * - bug in CreateScalarWith() fixed
 *
 * Revision 3.59  2002/06/26 08:49:05  dkr
 * CreateZero() creates vectors for WL-unrolling only :)
 *
 * Revision 3.58  2002/06/25 23:52:06  ktr
 * CreateZero now always creates a Withloop for non-scalar values, because
 * WLS can't handle ZeroVectors.
 *
 * Revision 3.57  2002/06/20 15:35:36  dkr
 * - AddVardecs() added
 * - MakeZero(), MakeZeroFromTypes() added
 *
 * Revision 3.56  2002/04/29 15:59:34  sbs
 * function HasDotTypes added.
 *
 * Revision 3.55  2002/03/07 16:42:03  sbs
 * HasDotArgs added.
 *
 * Revision 3.54  2002/03/07 02:19:02  dkr
 * AdjustAvisData() modified: DBUG_ASSERT added
 *
 * Revision 3.53  2002/03/01 02:39:12  dkr
 * GetArgtabIndexIn() and GetArgtabIndexOut() added
 *
 * Revision 3.52  2002/02/20 14:38:09  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.51  2002/02/08 10:02:31  dkr
 * function IsArray() modified: returns TRUE for type[*] objects as well.
 * function IsBoxed() modified: equals (IsArray || IsHidden) now.
 *
 * Revision 3.50  2001/12/17 21:30:46  dkr
 * MakeAssignInstr() modified
 *
 * Revision 3.49  2001/12/13 15:15:52  dkr
 * signature of MakeAssignIcm?() modified
 *
 * Revision 3.48  2001/12/12 14:40:25  dkr
 * function CombineExprs() added
 *
 * Revision 3.47  2001/12/11 15:58:16  dkr
 * GetDim() renamed into GetShapeDim()
 * GetDim() added
 *
 * Revision 3.46  2001/12/10 13:46:03  dkr
 * function MakeAssignInstr() added
 * functions MakeAssigns?() added
 *
 * Revision 3.45  2001/11/09 11:48:45  sbs
 * res now initialized in default case of functions IsExternal and friends!
 * (Just top please gcc 8-)
 *
 * Revision 3.44  2001/07/19 16:19:57  cg
 * Added new inquiery functions IsImported, IsExternal, IsFromModule
 * and IsFromClass to avoid clumsy direct checks for import status
 * of types, objects, and functions.
 *
 * Revision 3.43  2001/07/19 11:48:18  cg
 * AppendAssign now accepts N_empty nodes as
 * assign chain.
 *
 * Revision 3.42  2001/07/18 12:57:45  cg
 * Function ExprsConcat renamed to AppendExprs.
 *
 * Revision 3.41  2001/07/13 13:23:41  cg
 * DBUG tags brushed.
 *
 * Revision 3.40  2001/05/31 14:50:43  nmw
 * CompareTypesImplementation() added
 *
 * Revision 3.39  2001/05/17 14:43:58  dkr
 * FREE, MALLOC eliminated
 *
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
#include "DataFlowMask.h"
#include "wltransform.h"
#include "refcount.h"

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

    next = NULL;
    for (i = dim - 1; i >= 0; i--) {
        next = MakeExprs (MakeNum (SHPSEG_SHAPE (shape, i)), next);
    }

    array_node = MakeFlatArray (next);

    ARRAY_ISCONST (array_node) = TRUE;
    ARRAY_VECTYPE (array_node) = T_int;
    ARRAY_VECLEN (array_node) = dim;
    ((int *)ARRAY_CONSTVEC (array_node)) = Array2IntVec (next, NULL);

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
        if (TYPES_BASETYPE (type) != T_void) {
            count++;
        }
        type = TYPES_NEXT (type);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   int HasDotTypes( types *type)
 *
 * description:
 *   Checks whether any T_dot type is contained in the given types chain.
 *
 ******************************************************************************/

int
HasDotTypes (types *type)
{
    int flag = FALSE;

    DBUG_ENTER ("HasDotTypes");

    while (type != NULL) {
        if (TYPES_BASETYPE (type) == T_dots) {
            flag = TRUE;
            break;
        }
        type = TYPES_NEXT (type);
    }

    DBUG_RETURN (flag);
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
 *   int GetShapeDim( types* type)
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
GetShapeDim (types *type)
{
    types *impl_type;
    int dim, base_dim, impl_dim;

    DBUG_ENTER ("GetShapeDim");

    base_dim = TYPES_DIM (type);

    impl_type = GetTypes (type);

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
 *   int GetDim( types* type)
 *
 * Description:
 *   returns dimension of 'type':
 *     >= 0 : dimension known
 *     == -1: dimension unknown (but > 0)
 *     == -2: dimension unknown (>= 0)
 *
 ******************************************************************************/

int
GetDim (types *type)
{
    int dim;

    DBUG_ENTER ("GetDim");

    dim = GetShapeDim (type);
    dim = DIM_NO_OFFSET (dim);

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
 *   int CompareTypesImplementation( types *t1, types *t2)
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
CompareTypesImplementation (types *t1, types *t2)
{
    int res;
    shpseg *shpseg1, *shpseg2;
    int shpdim1, shpdim2;
    int dim;

    DBUG_ENTER ("CompareTypesImplementation");

    DBUG_ASSERT (((t1 != NULL) && (t2 != NULL)),
                 "CompareTypesImplementation() called with NULL type");

    if (GetBasetype (t1) == GetBasetype (t2)) {
        shpdim1 = GetShapeDim (t1);
        shpdim2 = GetShapeDim (t2);

        if (shpdim1 == shpdim2) {
            if (KNOWN_SHAPE (shpdim1)) { /* -> KNOWN_SHAPE( shpdim2) */
                shpseg1 = Type2Shpseg (t1, &dim);
                shpseg2 = Type2Shpseg (t2, NULL);

                res = (EqualShpseg (dim, shpseg1, shpseg2)) ? 0 : 2;

                if (shpseg1 != NULL) {
                    shpseg1 = FreeShpseg (shpseg1);
                }
                if (shpseg2 != NULL) {
                    shpseg2 = FreeShpseg (shpseg2);
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

    dim = GetShapeDim (type);

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

    dim = GetShapeDim (type);

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
 * Function:
 *   node *CreateZeroFromType( types *type, bool unroll, node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
CreateZeroFromType (types *type, bool unroll, node *fundef)
{
    node *zero;
    shpseg *shape;
    simpletype btype;
    int dim;

    DBUG_ENTER ("CreateZeroFromType");

    shape = Type2Shpseg (type, &dim);
    btype = GetBasetype (type);
    zero = CreateZero (dim, shape, btype, unroll, fundef);
    if (shape != NULL) {
        shape = FreeShpseg (shape);
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
 * IsArray := Is not a (static) scalar
 */
bool
IsArray (types *type)
{
    node *tdef;
    bool ret = FALSE;

    DBUG_ENTER ("IsArray");

    if (TYPES_DIM (type) != SCALAR) {
        ret = TRUE;
    } else if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        if (TYPEDEF_DIM (tdef) != SCALAR) {
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
IsNonUniqueHidden (types *type)
{
    bool ret;

    DBUG_ENTER ("IsNonUniqueHidden");

    ret = (IsHidden (type) && (!IsUnique (type)));

    DBUG_RETURN (ret);
}

bool
IsBoxed (types *type)
{
    bool ret;

    DBUG_ENTER ("IsBoxed");

    ret = (IsHidden (type) || IsArray (type));

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
        tmp = Free (tmp);
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
                tmp = Free (tmp);
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
            NODELIST_ATTRIB2 (nl) = Free (NODELIST_ATTRIB2 (nl));
        }
        nl = FreeNodelistNode (nl);
    }

    tmpnl = nl;
    prevnl = NULL;
    while (tmpnl) {
        if (NODELIST_NODE (tmpnl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (tmpnl)) {
                NODELIST_ATTRIB2 (tmpnl) = Free (NODELIST_ATTRIB2 (tmpnl));
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
            NODELIST_ATTRIB2 (nl) = Free (NODELIST_ATTRIB2 (nl));
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

/***
 ***  ARGTAB :
 ***/

/******************************************************************************
 *
 * Function:
 *   int GetArgtabIndexOut( types *type, argtab_t *argtab)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
GetArgtabIndexOut (types *type, argtab_t *argtab)
{
    int idx;

    DBUG_ENTER ("GetArgtabIndexOut");

    idx = 0;
    while ((argtab->ptr_out[idx] != type) && (idx < argtab->size)) {
        idx++;
    }
    DBUG_ASSERT ((argtab->ptr_out[idx] == type), "no index for out-parameter found!");

    DBUG_RETURN (idx);
}

/******************************************************************************
 *
 * Function:
 *   int GetArgtabIndexIn( types *type, argtab_t *argtab)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
GetArgtabIndexIn (types *type, argtab_t *argtab)
{
    int idx;

    DBUG_ENTER ("GetArgtabIndexIn");

    idx = 0;
    while (((argtab->ptr_in[idx] == NULL) || (ARG_TYPE (argtab->ptr_in[idx]) != type))
           && (idx < argtab->size)) {
        idx++;
    }
    DBUG_ASSERT (((argtab->ptr_in[idx] != NULL)
                  && (ARG_TYPE (argtab->ptr_in[idx]) == type)),
                 "no index for in-parameter found!");

    DBUG_RETURN (idx);
}

/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  general :
 ***/

/*****************************************************************************
 *
 * function:
 *   bool IsImported( node* symbol)
 *   bool IsExternal( node* symbol)
 *   bool IsFromModule( node* symbol)
 *   bool IsFromClass( node* symbol)
 *
 * description:
 *
 *   These test functions are applicable to any kind of symbol,
 *   more precisely to N_typedef, N_objdef, and N_fundef nodes.
 *
 *****************************************************************************/

bool
IsImported (node *symbol)
{
    bool res;

    DBUG_ENTER ("IsImported");

    DBUG_ASSERT ((symbol != NULL), ("Function IsImported applied to NULL."));

    switch (NODE_TYPE (symbol)) {
    case N_typedef:
        res = ((TYPEDEF_STATUS (symbol) == ST_imported_mod)
               || (TYPEDEF_STATUS (symbol) == ST_imported_class)
               || (TYPEDEF_STATUS (symbol) == ST_imported_extmod)
               || (TYPEDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_objdef:
        res = ((OBJDEF_STATUS (symbol) == ST_imported_mod)
               || (OBJDEF_STATUS (symbol) == ST_imported_class)
               || (OBJDEF_STATUS (symbol) == ST_imported_extmod)
               || (OBJDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_fundef:
        res = ((FUNDEF_STATUS (symbol) == ST_imported_mod)
               || (FUNDEF_STATUS (symbol) == ST_imported_class)
               || (FUNDEF_STATUS (symbol) == ST_imported_extmod)
               || (FUNDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    default:
        res = 0;
        DBUG_ASSERT (0, ("Function IsImported applied to wrong node type."));
    }

    DBUG_RETURN (res);
}

bool
IsExternal (node *symbol)
{
    bool res;

    DBUG_ENTER ("IsExternal");

    DBUG_ASSERT ((symbol != NULL), ("Function IsExternal applied to NULL."));

    switch (NODE_TYPE (symbol)) {
    case N_typedef:
        res = ((TYPEDEF_STATUS (symbol) == ST_imported_extmod)
               || (TYPEDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_objdef:
        res = ((OBJDEF_STATUS (symbol) == ST_imported_extmod)
               || (OBJDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_fundef:
        res = ((FUNDEF_STATUS (symbol) == ST_imported_extmod)
               || (FUNDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    default:
        res = 0;
        DBUG_ASSERT (0, ("Function IsExternal applied to wrong node type."));
    }

    DBUG_RETURN (res);
}

bool
IsFromModule (node *symbol)
{
    bool res;

    DBUG_ENTER ("IsFromModule");

    DBUG_ASSERT ((symbol != NULL), ("Function IsFromModule applied to NULL."));

    switch (NODE_TYPE (symbol)) {
    case N_typedef:
        res = ((TYPEDEF_STATUS (symbol) == ST_imported_extmod)
               || (TYPEDEF_STATUS (symbol) == ST_imported_mod));
        break;
    case N_objdef:
        res = ((OBJDEF_STATUS (symbol) == ST_imported_extmod)
               || (OBJDEF_STATUS (symbol) == ST_imported_mod));
        break;
    case N_fundef:
        res = ((FUNDEF_STATUS (symbol) == ST_imported_extmod)
               || (FUNDEF_STATUS (symbol) == ST_imported_mod));
        break;
    default:
        res = 0;
        DBUG_ASSERT (0, ("Function IsFromModule applied to wrong node type."));
    }

    DBUG_RETURN (res);
}

bool
IsFromClass (node *symbol)
{
    bool res;

    DBUG_ENTER ("IsFromClass");

    DBUG_ASSERT ((symbol != NULL), ("Function IsFromClass applied to NULL."));

    switch (NODE_TYPE (symbol)) {
    case N_typedef:
        res = ((TYPEDEF_STATUS (symbol) == ST_imported_class)
               || (TYPEDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_objdef:
        res = ((OBJDEF_STATUS (symbol) == ST_imported_class)
               || (OBJDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    case N_fundef:
        res = ((FUNDEF_STATUS (symbol) == ST_imported_class)
               || (FUNDEF_STATUS (symbol) == ST_imported_extclass));
        break;
    default:
        res = 0;
        DBUG_ASSERT (0, ("Function IsFromClass applied to wrong node type."));
    }

    DBUG_RETURN (res);
}

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

    DBUG_ASSERT (((tdef_chain == NULL) || (NODE_TYPE (tdef_chain) == N_typedef)),
                 ("First argument of AppendTypedef() has wrong node type."));
    DBUG_ASSERT (((tdef == NULL) || (NODE_TYPE (tdef) == N_typedef)),
                 ("Second argument of AppendTypedef() has wrong node type."));

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

    DBUG_ASSERT (((fundef_chain == NULL) || (NODE_TYPE (fundef_chain) == N_fundef)),
                 ("First argument of AppendFundef() has wrong node type."));
    DBUG_ASSERT (((fundef == NULL) || (NODE_TYPE (fundef) == N_fundef)),
                 ("Second argument of AppendFundef() has wrong node type."));

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
 * Function:
 *   node *AddVardecs( node *fundef, node *vardecs)
 *
 * Description:
 *   Inserts new declarations into the AST, updates the DFMbase and returns
 *   the modified N_fundef node.
 *
 ******************************************************************************/

node *
AddVardecs (node *fundef, node *vardecs)
{
    DBUG_ENTER ("AddVardecs");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no N_fundef node found!");

    /*
     * insert new vardecs into AST
     */
    FUNDEF_VARDEC (fundef) = AppendVardec (vardecs, FUNDEF_VARDEC (fundef));

    /*
     * we must update FUNDEF_DFM_BASE!!
     */
    if (FUNDEF_DFM_BASE (fundef) != NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMUpdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                               FUNDEF_VARDEC (fundef));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *AppendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 ******************************************************************************/

node *
AppendVardec (node *vardec_chain, node *vardec)
{
    node *ret;

    DBUG_ENTER ("AppendVardec");

    DBUG_ASSERT (((vardec_chain == NULL) || (NODE_TYPE (vardec_chain) == N_vardec)),
                 ("First argument of AppendVardec() has wrong node type."));
    DBUG_ASSERT (((vardec == NULL) || (NODE_TYPE (vardec) == N_vardec)),
                 ("Second argument of AppendVardec() has wrong node type."));

    APPEND (ret, node *, TYPEDEF, vardec_chain, vardec);

    DBUG_RETURN (ret);
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
                             DupAllTypes (ARG_TYPE (arg_node)), NULL);

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
                       DupAllTypes (VARDEC_TYPE (vardec_node)),
                       VARDEC_STATUS (vardec_node), ST_undef, NULL);

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
        DBUG_ASSERT ((base_id != NULL), "no BASEID found in AVIS!");

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
 * function:
 *   int HasDotArgs( node *args)
 *
 * description:
 *   Checks whether any T_dot type is contained in the given argument chain.
 *
 ******************************************************************************/

int
HasDotArgs (node *args)
{
    int flag = FALSE;

    DBUG_ENTER ("HasDotArgs");

    while (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_arg node found!");
        if ((ARG_TYPE (args) != NULL) && (ARG_BASETYPE (args) == T_dots)) {
            flag = TRUE;
            break;
        }
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (flag);
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
    } else {
        is_equal = 0;
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
 * Function:
 *   node *MakeAssignLet( char *var_name, node *vardec_node, node *let_expr)
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
 * Function:
 *   node *MakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
MakeAssignInstr (node *instr, node *next)
{
    node *result;

    if (instr == NULL) {
        result = next;
    } else if (NODE_TYPE (instr) == N_assign) {
        result = AppendAssign (instr, next);
    } else {
        result = MakeAssign (instr, next);
    }

    return (result);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAssign?( node *part?, ...)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
MakeAssigns1 (node *part1)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns1");

    assigns = MakeAssignInstr (part1, NULL);

    DBUG_RETURN (assigns);
}

node *
MakeAssigns2 (node *part1, node *part2)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns2");

    assigns = MakeAssignInstr (part1, MakeAssigns1 (part2));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns3 (node *part1, node *part2, node *part3)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns3");

    assigns = MakeAssignInstr (part1, MakeAssigns2 (part2, part3));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns4 (node *part1, node *part2, node *part3, node *part4)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns4");

    assigns = MakeAssignInstr (part1, MakeAssigns3 (part2, part3, part4));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns5 (node *part1, node *part2, node *part3, node *part4, node *part5)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns5");

    assigns = MakeAssignInstr (part1, MakeAssigns4 (part2, part3, part4, part5));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns6 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns6");

    assigns = MakeAssignInstr (part1, MakeAssigns5 (part2, part3, part4, part5, part6));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns7 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns7");

    assigns
      = MakeAssignInstr (part1, MakeAssigns6 (part2, part3, part4, part5, part6, part7));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns8 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7, node *part8)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns8");

    assigns = MakeAssignInstr (part1, MakeAssigns7 (part2, part3, part4, part5, part6,
                                                    part7, part8));

    DBUG_RETURN (assigns);
}

node *
MakeAssigns9 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7, node *part8, node *part9)
{
    node *assigns;

    DBUG_ENTER ("MakeAssigns9");

    assigns = MakeAssignInstr (part1, MakeAssigns8 (part2, part3, part4, part5, part6,
                                                    part7, part8, part9));

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAssignIcm0( char *name, node *next)
 *   node *MakeAssignIcm?( char *name, node *arg?, ..., node *next)
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
MakeAssignIcm0 (char *name, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm0");

    assigns = MakeAssign (MakeIcm0 (name), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm1 (char *name, node *arg1, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm1");

    assigns = MakeAssign (MakeIcm1 (name, arg1), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm2 (char *name, node *arg1, node *arg2, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm2");

    assigns = MakeAssign (MakeIcm2 (name, arg1, arg2), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm3");

    assigns = MakeAssign (MakeIcm3 (name, arg1, arg2, arg3), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm4");

    assigns = MakeAssign (MakeIcm4 (name, arg1, arg2, arg3, arg4), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm5");

    assigns = MakeAssign (MakeIcm5 (name, arg1, arg2, arg3, arg4, arg5), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                node *arg6, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm6");

    assigns = MakeAssign (MakeIcm6 (name, arg1, arg2, arg3, arg4, arg5, arg6), next);

    DBUG_RETURN (assigns);
}

node *
MakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
                node *arg6, node *arg7, node *next)
{
    node *assigns;

    DBUG_ENTER ("MakeAssignIcm7");

    assigns
      = MakeAssign (MakeIcm7 (name, arg1, arg2, arg3, arg4, arg5, arg6, arg7), next);

    DBUG_RETURN (assigns);
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

    DBUG_ASSERT (((assign_chain == NULL) || (NODE_TYPE (assign_chain) == N_assign)
                  || (NODE_TYPE (assign_chain) == N_empty)),
                 ("First argument of AppendAssign() has wrong node type."));
    DBUG_ASSERT (((assign == NULL) || (NODE_TYPE (assign) == N_assign)),
                 ("Second argument of AppendAssign() has wrong node type."));

    if ((assign_chain != NULL) && (NODE_TYPE (assign_chain) == N_empty)) {
        assign_chain = FreeNode (assign_chain);
    }

    APPEND (ret, node *, ASSIGN, assign_chain, assign);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *AppendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends an new ICM with name and args given as an assign to the given
 *   chain of assignments.
 *
 ******************************************************************************/

node *
AppendAssignIcm (node *assign, char *name, node *args)
{
    node *result;

    DBUG_ENTER ("AppendAssignIcm");

    result = AppendAssign (assign, MakeAssignIcm1 (name, args, NULL));

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/******************************************************************************
 *
 * function:
 *   node *AppendExprs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

node *
AppendExprs (node *exprs_chain, node *exprs)
{
    node *ret;

    DBUG_ENTER ("AppendExprs");

    DBUG_ASSERT (((exprs_chain == NULL) || (NODE_TYPE (exprs_chain) == N_exprs)),
                 ("First argument of AppendExprs() has wrong node type."));
    DBUG_ASSERT (((exprs == NULL) || (NODE_TYPE (exprs) == N_exprs)),
                 ("Second argument of AppendExprs() has wrong node type."));

    APPEND (ret, node *, EXPRS, exprs_chain, exprs);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *CombineExprs( node *first, node *second)
 *
 * description:
 *   'first' and 'second' are N_exprs chains or expression nodes (N_id, N_num,
 *   ...) that will be conactenated to a single N_exprs chain.
 *
 ******************************************************************************/

node *
CombineExprs (node *first, node *second)
{
    node *result;

    DBUG_ENTER ("CombineExprs");

    if (first != NULL) {
        if (NODE_TYPE (first) != N_exprs) {
            result = MakeExprs (first, second);
        } else {
            result = AppendExprs (first, second);
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

/*****************************************************************************
 *
 * Function:
 *   node *AdjustVectorShape( node *array)
 *
 * Description:
 *   adjusts ARRAY_SHAPE according to the number of elements.
 *   Note that the array will always be one-dimensional
 *
 *****************************************************************************/

node *
AdjustVectorShape (node *array)
{
    DBUG_ENTER ("AdjustVectorShape");

    DBUG_ASSERT (array != NULL, "AdjustVectorShape called with NULL argument!");

    if (ARRAY_SHAPE (array) != NULL)
        SHFreeShape (ARRAY_SHAPE (array));

    ARRAY_SHAPE (array) = SHCreateShape (1, CountExprs (ARRAY_AELEMS (array)));

    DBUG_RETURN (array);
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

    ret_node = MakeFlatArray (exprs_node);
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

/** <!--********************************************************************-->
 *
 * @fn node *ConcatVecs( node* vec1, node *vec2 )
 *
 * @brief concatenates two vectors.
 *
 * @param vec1 A N_array node containing the first vector
 * @param vec2 A N_array node containing the second vector
 *
 * @return The concatenation vec1++vec2 as an N_array node
 *
 *****************************************************************************/
node *
ConcatVecs (node *vec1, node *vec2)
{
    shpseg *shpseg;
    node *res;

    DBUG_ENTER ("CONCAT_VECS");

    DBUG_ASSERT (((NODE_TYPE (vec1) == N_array) && (NODE_TYPE (vec2) == N_array)
                  && (SHGetDim (ARRAY_SHAPE (vec1)) == 1)
                  && (SHGetDim (ARRAY_SHAPE (vec2)) == 1)
                  && (ARRAY_BASETYPE (vec1) == ARRAY_BASETYPE (vec2))),
                 "ConcatVecs called with not N_array nodes in vector form");

    res = MakeFlatArray (
      CombineExprs (DupTree (ARRAY_AELEMS (vec1)), DupTree (ARRAY_AELEMS (vec2))));

    shpseg = SHShape2OldShpseg (ARRAY_SHAPE (res));

    ARRAY_TYPE (res) = MakeTypes (ARRAY_BASETYPE (vec1), 1, shpseg, NULL, NULL);

    ((int *)ARRAY_CONSTVEC (res))
      = Array2Vec (ARRAY_BASETYPE (vec1), ARRAY_AELEMS (res), NULL);

    DBUG_RETURN (res);
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

/*****************************************************************************
 *
 * function:
 *   node *Ids2Exprs( ids *ids_arg)
 *
 * description:
 *   convert ids into N_exprs chain
 *
 *****************************************************************************/

node *
Ids2Exprs (ids *ids_arg)
{
    node *exprs;

    DBUG_ENTER ("Ids2Exprs");

    if (ids_arg != NULL) {
        exprs = MakeExprs (DupIds_Id (ids_arg), Ids2Exprs (IDS_NEXT (ids_arg)));
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
Ids2Array (ids *ids_arg)
{
    node *array;
    types *array_type;
    shpseg *array_shape;
    int len, i;

    DBUG_ENTER ("Ids2Array");

    if (ids_arg != NULL) {
        len = CountIds (ids_arg);
        array = MakeFlatArray (Ids2Exprs (ids_arg));
    } else {
        len = 0;
        array = MakeFlatArray (NULL);
    }

    ARRAY_ISCONST (array) = FALSE;

    array_type = DupOneTypes (IDS_TYPE (ids_arg));
    if (TYPES_SHPSEG (array_type) == NULL) {
        TYPES_SHPSEG (array_type) = MakeShpseg (NULL);
    }
    array_shape = TYPES_SHPSEG (array_type);
    for (i = TYPES_DIM (array_type); i > 0; i--) {
        SHPSEG_SHAPE (array_shape, i) = SHPSEG_SHAPE (array_shape, (i - 1));
    }
    (TYPES_DIM (array_type))++;
    SHPSEG_SHAPE (array_shape, 0) = len;
    ARRAY_TYPE (array) = array_type;

    DBUG_RETURN (array);
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
    if (length != NULL) {
        *length = i;
    }

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
    if (length != NULL) {
        *length = i;
    }

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
    if (length != NULL) {
        *length = i;
    }

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
    if (length != NULL) {
        *length = i;
    }

    dblvec = Malloc (i * sizeof (double));

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

/******************************************************************************
 *
 * function:
 *   node *MakeAp1( char *name, char *mod, node *arg1)
 *   node *MakeAp2( char *name, char *mod, node *arg1, node *arg2)
 *   node *MakeAp3( char *name, char *mod, node *arg1, node *arg2, node *arg3)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
MakeAp1 (char *name, char *mod, node *arg1)
{
    node *res;

    DBUG_ENTER ("MakeAp1");

    res = MakeAp (name, mod, MakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}

node *
MakeAp2 (char *name, char *mod, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ("MakeAp2");

    res = MakeAp (name, mod, MakeExprs (arg1, MakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
MakeAp3 (char *name, char *mod, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ("MakeAp3");

    res = MakeAp (name, mod, MakeExprs (arg1, MakeExprs (arg2, MakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***/

/******************************************************************************
 *
 * Function:
 *   node *LiftArg( node *arg, node *fundef, types *new_type, bool do_rc,
 *                  node **new_assigns)
 *
 * Description:
 *   Lifts the given argument of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *    - Returns the new argument.
 *
 ******************************************************************************/

node *
LiftArg (node *arg, node *fundef, types *new_type, bool do_rc, node **new_assigns)
{
    char *new_name;
    node *new_vardec;
    node *new_arg;
    ids *new_ids;

    DBUG_ENTER ("LiftArg");

    if (NODE_TYPE (arg) == N_id) {
        new_name = TmpVarName (ID_NAME (arg));
        /* Insert vardec for new var */
        if (new_type == NULL) {
            new_type = ID_TYPE (arg);
        }
    } else {
        new_name = TmpVar ();
        DBUG_ASSERT ((new_type != NULL), "no type found!");
    }

    new_vardec = MakeVardec (StringCopy (new_name), DupAllTypes (new_type), NULL);
    fundef = AddVardecs (fundef, new_vardec);

    /*
     * Abstract the given argument out:
     *   ... = fun( A:n, ...);
     * is transformed into
     *   __A:1 = A:n;
     *   ... = fun( __A:1, ...);
     */
    new_ids = MakeIds (new_name, NULL, ST_regular);
    IDS_VARDEC (new_ids) = new_vardec;
    (*new_assigns) = MakeAssign (MakeLet (arg, new_ids), (*new_assigns));

    new_arg = DupIds_Id (new_ids);

    if (do_rc) {
        if (NODE_TYPE (arg) == N_id) {
            if (RC_IS_ACTIVE (ID_REFCNT (arg))) {
                IDS_REFCNT (new_ids) = ID_REFCNT (new_arg) = 1;
            } else if (RC_IS_INACTIVE (ID_REFCNT (new_arg))) {
                IDS_REFCNT (new_ids) = ID_REFCNT (new_arg) = RC_INACTIVE;
            } else {
                DBUG_ASSERT ((0), "illegal RC value found!");
            }
        } else {
            IDS_REFCNT (new_ids) = ID_REFCNT (new_arg)
              = TYPE_MUST_REFCOUNT (new_type) ? 1 : RC_INACTIVE;
        }
    }

    DBUG_RETURN (new_arg);
}

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
 *   node *MakeIcm0( char *name)
 *   node *MakeIcm?( char *name, node *arg1, ...)
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

/******************************************************************************
 *
 * Function:
 *   node *CreateScalarWith( int dim, shpseg *shape, simpletype btype,
 *                           node *expr, node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
CreateScalarWith (int dim, shpseg *shape, simpletype btype, node *expr, node *fundef)
{
    node *wl;
    node *id;
    node *vardecs = NULL;
    ids *vec_ids;
    ids *scl_ids = NULL;
    ids *tmp_ids;
    int i;

    DBUG_ENTER ("CreateScalarWith");

    DBUG_ASSERT ((dim >= 0), "CreateScalarWith() used with unknown shape!");

    vec_ids = MakeIds (TmpVar (), NULL, ST_regular);
    vardecs
      = MakeVardec (StringCopy (IDS_NAME (vec_ids)),
                    MakeTypes (T_int, 1, MakeShpseg (MakeNums (dim, NULL)), NULL, NULL),
                    vardecs);
    IDS_VARDEC (vec_ids) = vardecs;
    IDS_AVIS (vec_ids) = VARDEC_AVIS (vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids = MakeIds (TmpVar (), NULL, ST_regular);
        vardecs
          = MakeVardec (StringCopy (IDS_NAME (tmp_ids)), MakeTypes1 (T_int), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
        IDS_VARDEC (scl_ids) = vardecs;
        IDS_AVIS (scl_ids) = VARDEC_AVIS (vardecs);
    }

    id = MakeId (TmpVar (), NULL, ST_regular);
    vardecs = MakeVardec (StringCopy (ID_NAME (id)), MakeTypes1 (btype), vardecs);
    ID_VARDEC (id) = vardecs;
    ID_AVIS (id) = VARDEC_AVIS (vardecs);

    wl = MakeNWith (MakeNPart (MakeNWithid (vec_ids, scl_ids),
                               MakeNGenerator (CreateZeroVector (dim, T_int),
                                               Shpseg2Array (shape, dim), F_le, F_lt,
                                               NULL, NULL),
                               NULL),
                    MakeNCode (MakeBlock (MakeAssignLet (StringCopy (ID_NAME (id)),
                                                         vardecs, expr),
                                          NULL),
                               id),
                    MakeNWithOp (WO_genarray, Shpseg2Array (shape, dim)));
    NCODE_USED (NWITH_CODE (wl))++;
    NPART_CODE (NWITH_PART (wl)) = NWITH_CODE (wl);
    NWITH_PARTS (wl) = 1;

    fundef = AddVardecs (fundef, vardecs);

    DBUG_RETURN (wl);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateZero( int dim, shpseg *shape, simpletype btype, bool no_wl,
 *                     node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
CreateZero (int dim, shpseg *shape, simpletype btype, bool no_wl, node *fundef)
{
    node *zero;
    int length = GetShpsegLength (dim, shape);

    DBUG_ENTER ("CreateZero");

    DBUG_ASSERT ((dim >= 0), "CreateZero() used with unknown shape!");

    if (dim == 0) {
        zero = CreateZeroScalar (btype);
    } else if (no_wl && (dim == 1)) {
        zero = CreateZeroVector (length, btype);
    } else {
        if (no_wl) {
            DBUG_ASSERT ((length <= wlunrnum), "CreateZero(): Vector is too long!");

            zero
              = MakePrf (F_reshape,
                         MakeExprs (Shpseg2Array (shape, dim),
                                    MakeExprs (CreateZeroVector (length, btype), NULL)));
        } else {
            zero = CreateScalarWith (dim, shape, btype, CreateZeroScalar (btype), fundef);
        }
    }

    DBUG_RETURN (zero);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateSel( ids *sel_vec, ids *sel_ids, node *sel_array, bool no_wl,
 *                    node *fundef)
 *
 * Description:
 *   Creates an array of zeros.
 *
 ******************************************************************************/

node *
CreateSel (ids *sel_vec, ids *sel_ids, node *sel_array, bool no_wl, node *fundef)
{
    node *sel;
    int len_index, dim_array;

    DBUG_ENTER ("CreateSel");

    DBUG_ASSERT ((NODE_TYPE (sel_array) == N_id), "no N_id node found!");

    len_index = GetTypesLength (IDS_TYPE (sel_vec));
    DBUG_ASSERT ((len_index > 0), "illegal index length found!");

    dim_array = GetDim (ID_TYPE (sel_array));
    DBUG_ASSERT ((dim_array > 0), "illegal array dimensionality found!");

    if (len_index > dim_array) {
        DBUG_ASSERT ((0), "illegal array selection found!");
        sel = NULL;
    } else if ((len_index == dim_array) || no_wl) {
        sel = MakePrf (F_sel, MakeExprs (DupIds_Id (sel_vec),
                                         MakeExprs (DupNode (sel_array), NULL)));
    } else { /* (len_index < dim_array) */
        node *new_index, *id, *vardec, *ass;
        ids *tmp_ids;
        shpseg *shape, *shape2;
        simpletype btype;
        int dim, i;

        /*
         * compute basetype/dim/shape of WL
         */
        btype = GetBasetype (ID_TYPE (sel_array));

        dim = (dim_array - len_index);

        shape = Type2Shpseg (ID_TYPE (sel_array), NULL);
        shape2 = Type2Shpseg (ID_TYPE (sel_array), NULL);
        for (i = 0; i < dim; i++) {
            SHPSEG_SHAPE (shape, i) = SHPSEG_SHAPE (shape2, (len_index + i));
        }
        shape2 = FreeShpseg (shape2);

        /*
         * create WL without expression
         */
        sel = CreateScalarWith (dim, shape, btype, NULL, fundef);

        /*
         * create index vector for F_sel
         */
        tmp_ids = sel_ids;
        while (IDS_NEXT (tmp_ids) != NULL) {
            tmp_ids = IDS_NEXT (tmp_ids);
        }
        IDS_NEXT (tmp_ids) = NWITH_IDS (sel); /* concat ids chains */
        new_index = Ids2Array (sel_ids);
        IDS_NEXT (tmp_ids) = NULL; /* restore ids chains */

        /*
         * create new id
         */
        id = MakeId (TmpVar (), NULL, ST_regular);
        vardec = MakeVardec (StringCopy (ID_NAME (id)),
                             DupOneTypes (ARRAY_TYPE (new_index)), NULL);
        ID_VARDEC (id) = vardec;
        fundef = AddVardecs (fundef, vardec);

        /*
         * create expression 'sel( tmp, A)' and insert it into WL
         */
        ASSIGN_RHS (BLOCK_INSTR (NWITH_CBLOCK (sel)))
          = MakePrf (F_sel, MakeExprs (id, MakeExprs (DupNode (sel_array), NULL)));

        /*
         * create assignment 'tmp = [...];' and insert it into WL
         */
        ass = MakeAssignLet (StringCopy (ID_NAME (id)), vardec, new_index);
        ASSIGN_NEXT (ass) = BLOCK_INSTR (NWITH_CBLOCK (sel));
        BLOCK_INSTR (NWITH_CBLOCK (sel)) = ass;
    }

    DBUG_RETURN (sel);
}

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
