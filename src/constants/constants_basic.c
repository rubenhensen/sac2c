/*
 * $Log$
 * Revision 1.10  2001/05/08 13:15:34  nmw
 * signature for IsZero... changed
 *
 * Revision 1.9  2001/05/07 09:07:35  nmw
 * AST2Constant uses type information when called for an id node
 *
 * Revision 1.8  2001/05/03 16:53:43  nmw
 * COIsZero/COIsOne compares ALL elements
 *
 * Revision 1.7  2001/05/02 08:01:24  nmw
 * COIsZero, COIsOne, ... and COMakeZero, COMakeOne, ... added
 *
 * Revision 1.6  2001/04/30 12:29:15  nmw
 * GetDataVec() added
 *
 * Revision 1.5  2001/04/19 07:49:26  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 1.4  2001/03/30 16:31:32  nmw
 * *** empty log message ***
 *
 * Revision 1.3  2001/03/26 08:22:07  sbs
 * new_co in COAST2Constant now also initialized in default case as well
 *
 * Revision 1.2  2001/03/22 14:25:41  nmw
 * COAST2Constant, COIsConstant, DebugPrint added
 *
 * Revision 1.1  2001/03/02 14:32:55  sbs
 * Initial revision
 *
 */

/*
 * This module implements an abstract datatype for keeping "machine"
 * constants (scalars as well as arbitrary shaped arrays).
 *
 * In the long term they shall replace the constvec's at N_id nodes and
 * N_array nodes. Furthermore, they will serve as value representation for
 * the new type checker.
 *
 * The clue of this module is, that it merely constitutes an entire
 * MOA-machine, with all the primitives that are intrinsic!!
 * This can be usefull for handling the constants themselves AND it also
 * can be used directly for interpreting intrinsic operations during type
 * checking and constant folding. So when it comes to re-coding the Constant
 * Folding, ALL value transforming operations can be mapped directly to
 * the functions supplied by this module!!
 *
 * However, the implementation of this module spreads over 3 files:
 *
 *    constants_basic.c (this file) it contains all very basic operations
 *      on constants, in particular, all the conversions from and into the AST!
 *    constants_struc_ops.c contains all those operations that perform structural
 *      operations on constans, e.g., dim, shape, take, drop and friends...
 *    constants_ari_ops.c implements all the arithmetic operations available, e.g.
 *      +, -, ....
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a constant is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 *   There are EXACTLY ONE CLASS OF FUNCTIONS that is an EXEPTION OF
 *   THIS RULE: - the GETxyz - functions for extracting components of constants
 * - The only function for freeing a shape structure is COFreeConstant!
 * - If the result is a shape structure, it has been freshly allocated!
 */

#include <stdlib.h>

#include "globals.h"
#include "free.h"
#include "dbug.h"
#include "my_debug.h"
#include "shape.h"
#include "cv2scalar.h"
#include "cv2cv.h"
#include "cv2str.h"
#include "tree_compound.h"
#include "basecv.h"

/*
 * Now, we include the own interface! The reason fot this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 */

#include "constants.h"

/*
 * Since we want to make the definition of "struct CONSTANT" known
 * to all files within the module constant, we provide another
 * interface file for internal usage only!
 */

#include "constants_internal.h"

/******************************************************************************
 ***
 *** constant-internal helper functions:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant *MakeConstant( simpletype type, shape *shp, void *elems, int vlen)
 *
 * description:
 *    most generic function for creating constant structures. It simply assembles
 *    a constant from the function's arguments. However, if vlen - the length
 *    of the unrolling - is not yet known, one should use the exported version
 *    of this function, i.e. COMakeConstant, which is almost identical but does
 *    require vlen to be specified explicitly!
 *
 ******************************************************************************/

constant *
MakeConstant (simpletype type, shape *shp, void *elems, int vlen)
{
    constant *res;

    DBUG_ENTER ("MakeConstant");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = shp;
    CONSTANT_ELEMS (res) = elems;
    CONSTANT_VLEN (res) = vlen;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void *AllocCV( simpletype type, int length)
 *
 * description:
 *    internal function for allocating a CV of length len.
 *
 ******************************************************************************/

void *
AllocCV (simpletype type, int length)
{
    void *res;

    DBUG_ENTER ("AllocCV");
    res = (void *)MALLOC (basetype_size[type] * length);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void *PickNElemsFromCV( simpletype type, void *elems, int offset, int length)
 *
 * description:
 *    internal function for picking a tile from a cv (constant vector). The
 *    problem we face here, is that C is not polymorphic. To alliviate that
 *    problem, we use a function table which is parameterized over all
 *    simpletypes. For this particular function, a global function table
 *    cv2cv is used. It's member functions are defined in cv2cv.c and
 *    the function table is generated by applying the ".mac" mechanism
 *    to      type_info.mac     .
 *
 ******************************************************************************/

void *
PickNElemsFromCV (simpletype type, void *elems, int offset, int length)
{
    void *res;

    DBUG_ENTER ("PickNElemsFromCV");

    res = AllocCV (type, length);
    cv2cv[type](elems, offset, length, res, 0);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void CopyElemsFromCVToCV( simpletype type, void *from, int off, int len,
 *                                               void *to, int to_off)
 *
 * description:
 *    this internal function is very similar to PickNElemsFromCV. The only
 *    difference is that here, the target CV is not created, but expected
 *    to be specified as an extra argument!
 *
 ******************************************************************************/

void
CopyElemsFromCVToCV (simpletype type, void *from, int off, int len, void *to, int to_off)
{
    DBUG_ENTER ("CopyElemsFromCVToCV");

    cv2cv[type](from, off, len, to, to_off);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DbugPrintBinOp( char *fun,
 *                         constant *arg1,
 *                          constant *arg2,
 *                           constant *res)
 *
 * description:
 *   logs the args and result of binary ops to stderr.
 *
 ******************************************************************************/

#ifndef DBUG_OFF

void
DbugPrintBinOp (char *fun, constant *arg1, constant *arg2, constant *res)
{
    DBUG_ENTER ("DbugPrintBinOp");

    fprintf (stderr, "%s applied to\n ", fun);
    COPrintConstant (stderr, arg1);
    fprintf (stderr, " ");
    COPrintConstant (stderr, arg2);
    fprintf (stderr, "results in: ");
    COPrintConstant (stderr, res);

    DBUG_VOID_RETURN;
}

#endif

/******************************************************************************
 *
 * function:
 *   void DbugPrintUnaryOp( char *fun,
 *                          constant *arg1,
 *                          constant *res)
 *
 * description:
 *   logs the arg and result of unary ops to stderr.
 *
 ******************************************************************************/

#ifndef DBUG_OFF

void
DbugPrintUnaryOp (char *fun, constant *arg1, constant *res)
{
    DBUG_ENTER ("DbugPrintUnaryOp");

    fprintf (stderr, "%s applied to\n ", fun);
    COPrintConstant (stderr, arg1);
    fprintf (stderr, "results in: ");
    COPrintConstant (stderr, res);

    DBUG_VOID_RETURN;
}

#endif

/******************************************************************************
 ***
 *** local helper functions:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant *MakeScalarConstantFromCV( simpletype type, void *cv)
 *
 * description:
 *    internal function for creating a constant structure for scalars
 *    from a cv (constant vector) containg exactly one element.
 *
 ******************************************************************************/

constant *
MakeScalarConstantFromCV (simpletype type, void *cv)
{
    constant *res;

    DBUG_ENTER ("MakeScalarConstantFromCV");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = SHMakeShape (0);
    CONSTANT_ELEMS (res) = cv;
    CONSTANT_VLEN (res) = 1;

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 *** Functions for creating constants and extracting infos from them:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant *COMakeConstant( simpletype type, shape * shp, void *elems)
 *
 * description:
 *    most generic (exported) function for creating constant structures. It
 *    assembles a constant from the function's arguments and computes the length
 *    of the unrolling by itself. However, if the length of the unrolling is known,
 *    the local helper function MakeConstant should be called for efficiency
 *    reasons.
 *
 ******************************************************************************/

constant *
COMakeConstant (simpletype type, shape *shp, void *elems)
{
    constant *res;

    DBUG_ENTER ("COMakeConstant");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = shp;
    CONSTANT_ELEMS (res) = elems;
    CONSTANT_VLEN (res) = SHGetUnrLen (shp);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COMakeConstantFromInt( int val)
 *
 * description:
 *    creates an integer constant from a C-integer value.
 *
 ******************************************************************************/

constant *
COMakeConstantFromInt (int val)
{
    constant *res;
    int *intelems;

    DBUG_ENTER ("COMakeConstantFromInt");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHMakeShape (0);
    intelems = (int *)MALLOC (sizeof (int));
    intelems[0] = val;
    CONSTANT_ELEMS (res) = intelems;
    CONSTANT_VLEN (res) = 1;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COMakeConstantFromArray( node *a)
 *
 * description:
 *    translates an N_array node into a constant. It checks, whether a indeed
 *    is a constant. If so an according constant structure is created; otherwise
 *    NULL is returned!
 *
 ******************************************************************************/

constant *
COMakeConstantFromArray (node *a)
{
    types *type;
    constant *res;

    DBUG_ENTER ("COMakeConstantFromArray");

    type = ARRAY_TYPE (a);

    /*
     * This implementation does not yet comply to its desired functionality!
     * We assume here that a is a constant integer vector!
     */
    res = COMakeConstant (TYPES_BASETYPE (type), SHOldTypes2Shape (type),
                          Array2IntVec (ARRAY_AELEMS (a), NULL));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    simpletype COGetType( constant *a)
 *    int COGetDim( constant *a)
 *    shape *COGetShape( constant *a)
 *    void **GetDataVec( constant *a)
 *
 * description:
 *    several functions for extracting info from constants.
 *
 ******************************************************************************/

simpletype
COGetType (constant *a)
{
    DBUG_ENTER ("COGetType");
    DBUG_RETURN (CONSTANT_TYPE (a));
}

int
COGetDim (constant *a)
{
    DBUG_ENTER ("COGetDim");
    DBUG_RETURN (SHGetDim (CONSTANT_SHAPE (a)));
}

shape *
COGetShape (constant *a)
{
    DBUG_ENTER ("COGetShape");
    DBUG_RETURN (CONSTANT_SHAPE (a));
}

void **
COGetDataVec (constant *a)
{
    DBUG_ENTER ("COGetDataVec");
    DBUG_RETURN (CONSTANT_ELEMS (a));
}

/******************************************************************************
 ***
 *** Functions for handling / converting constants:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant *COCopyConstant( constant *a)
 *
 * description:
 *    copies a including all of its sub-structures.
 *
 ******************************************************************************/

constant *
COCopyConstant (constant *a)
{
    constant *res;

    DBUG_ENTER ("COCopyConstant");

    res = MakeConstant (CONSTANT_TYPE (a), SHCopyShape (CONSTANT_SHAPE (a)),
                        PickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), 0,
                                          CONSTANT_VLEN (a)),
                        CONSTANT_VLEN (a));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void COPrintConstant( FILE * file, constant *a)
 *
 * description:
 *    prints the value of a to file
 *
 ******************************************************************************/

void
COPrintConstant (FILE *file, constant *a)
{
    DBUG_ENTER ("COPrintConstant");

    fprintf (file, "constant at " F_PTR ": type %s, ", a, mdb_type[CONSTANT_TYPE (a)]);
    SHPrintShape (file, CONSTANT_SHAPE (a));
    fprintf (file, " [%s]\n",
             cv2str[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0, CONSTANT_VLEN (a)));

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    constant *COFreeConstant( constant *a)
 *
 * description:
 *    frees a including all of its sub-structures.
 *
 ******************************************************************************/

constant *
COFreeConstant (constant *a)
{
    DBUG_ENTER ("COFreeConstant");

    CONSTANT_SHAPE (a) = SHFreeShape (CONSTANT_SHAPE (a));
    FREE (CONSTANT_ELEMS (a));
    FREE (a);
    a = NULL;

    DBUG_RETURN (a);
}

/******************************************************************************
 *
 * function:
 *    node *COConstant2AST( constant *a)
 *
 * description:
 *    This function converts a constant into the according AST representation.
 *    Similar to PickNElemsFromCV, the missing polymorphism of C is resolved by
 *    using yet another function table called "cv2scalar", whose functions are
 *    defined in the file    cv2scalar.c   .
 *    In order to make these functions as minimal as possible, they contain
 *    functions   COCv2<simpletype>( void * elems, int offset),    which
 *    create a suitable node containing the value taken from elems at position
 *    offset.
 *
 ******************************************************************************/

node *
COConstant2AST (constant *a)
{
    node *res, *exprs;
    int i;

    DBUG_ENTER ("COConstant2AST");

    if (COGetDim (a) == 0) {
        res = cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0);
    } else {
        /* First, we build the exprs! */
        exprs = NULL;
        for (i = CONSTANT_VLEN (a) - 1; i >= 0; i--) {
            exprs
              = MakeExprs (cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), i), exprs);
        }
        /* Finally, the N_array node is created! */
        res = MakeArray (exprs);
        /*
         * After creating the array, we have to create a types-node to preserve
         * the shape of the array!
         */
        ARRAY_TYPE (res) = MakeTypes (CONSTANT_TYPE (a), CONSTANT_DIM (a),
                                      SHShape2OldShpseg (CONSTANT_SHAPE (a)), NULL, NULL);
        /*
         * Note here, that in some situation the calling function has to add
         * constvec infos. This is not done here, since it is not yet clear how
         * the representation of constant arrays should be in the long term....
         */
        ARRAY_ISCONST (res) = TRUE;
        ARRAY_VECTYPE (res) = CONSTANT_TYPE (a);
        ARRAY_VECLEN (res) = CONSTANT_VLEN (a);
        ARRAY_CONSTVEC (res) = Array2Vec (CONSTANT_TYPE (a), ARRAY_AELEMS (res), NULL);
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COAST2Constant( node *n)
 *
 * description:
 *    converts a given node from the AST to a constant.
 *    return NULL if n is not a simple constant.
 *
 *    this funtion can deal with scalar types like
 *      N_num, N_double, N_float, N_bool
 *    arrays (N_array)
 *    and identifier (N_id with marked AVIS_SSACONST attribute)
 *
 ******************************************************************************/
constant *
COAST2Constant (node *n)
{
    constant *new_co;
    void *element;

    DBUG_ENTER ("COAST2Constant");

    if ((n != NULL) && (COIsConstant (n))) {
        /* convert the given constant node */

        switch (NODE_TYPE (n)) {
        case N_num:
            element = (int *)MALLOC (sizeof (int));
            *((int *)element) = NUM_VAL (n);
            new_co = COMakeConstant (T_int, SHMakeShape (0), element);
            break;

        case N_double:
            element = (double *)MALLOC (sizeof (double));
            *((double *)element) = DOUBLE_VAL (n);
            new_co = COMakeConstant (T_double, SHMakeShape (0), element);
            break;

        case N_float:
            element = (float *)MALLOC (sizeof (float));
            *((float *)element) = FLOAT_VAL (n);
            new_co = COMakeConstant (T_float, SHMakeShape (0), element);
            break;

        case N_bool:
            element = (bool *)MALLOC (sizeof (bool));
            *((bool *)element) = BOOL_VAL (n);
            new_co = COMakeConstant (T_bool, SHMakeShape (0), element);
            break;

        case N_array:
            new_co = COMakeConstant (TYPES_BASETYPE (ARRAY_TYPE (n)),
                                     SHOldTypes2Shape (ARRAY_TYPE (n)),
                                     Array2Vec (TYPES_BASETYPE (ARRAY_TYPE (n)),
                                                ARRAY_AELEMS (n), NULL));
            break;

        case N_id:
            new_co = COCopyConstant (AVIS_SSACONST (ID_AVIS (n)));

            /* update constants shape info according to type info */
            DBUG_ASSERT ((TYPES_BASETYPE (
                            VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (n))))
                          == CONSTANT_TYPE (new_co)),
                         "different basetype in id and assigned array");
            CONSTANT_SHAPE (new_co) = SHFreeShape (CONSTANT_SHAPE (new_co));
            CONSTANT_SHAPE (new_co)
              = SHOldTypes2Shape (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (n))));
            break;

        default:
            DBUG_ASSERT ((FALSE), "missing implementation for given nodetype");
            new_co = NULL; /* just to please the compiler... */
        }
    } else {
        /* node is not a constant */
        new_co = NULL;
    }
    DBUG_RETURN (new_co);
}

/******************************************************************************
 *
 * function:
 *    bool COIsConstant( node *n)
 *
 * description:
 *    checks if a given node is constant
 *
 *    this funtion can deal with scalar types like
 *      N_num, N_double, N_float, N_bool
 *    arrays (N_array)
 *    and identifier (N_id with marked AVIS_SSACONST attribute)
 *
 ******************************************************************************/
bool
COIsConstant (node *n)
{
    bool res;

    DBUG_ENTER ("COIsConstant");

    if (n != NULL) {
        switch (NODE_TYPE (n)) {
        case N_num:
        case N_double:
        case N_float:
        case N_bool:
            res = TRUE;
            break;

        case N_array:
            res = IsConstArray (n);
            break;

        case N_id:
            res = (AVIS_SSACONST (ID_AVIS (n)) != NULL);
            break;

        default:
            /* unknown node is not constant */
            res = FALSE;
        }
    } else {
        /* NULL is not a constant */
        res = FALSE;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   constant *COMakeZero( simpletype type, shape *shp)
 *   constant *COMakeOne( simpletype type, shape *shp)
 *   constant *COMakeTrue( simpletype type, shape *shp)
 *   constant *COMakeFalse( simpletype type, shape *shp)
 *
 * description:
 *   create a constant of the given basetype and shape filled with 0 or 1
 *   (for boolean FALSE/TRUE). if no constant can be created for this basetype
 *   the result will be NULL.
 *
 ******************************************************************************/
constant *
COMakeZero (simpletype type, shape *shp)
{
    DBUG_ENTER ("COMakeZero");
    DBUG_RETURN (basecv_zero[type](shp));
}

constant *
COMakeOne (simpletype type, shape *shp)
{
    DBUG_ENTER ("COMakeOne");
    DBUG_RETURN (basecv_one[type](shp));
}

constant *
COMakeTrue (shape *shp)
{
    DBUG_ENTER ("COMakeTrue");
    DBUG_RETURN (basecv_one[T_bool](shp));
}

constant *
COMakeFalse (shape *shp)
{
    DBUG_ENTER ("COMakeFalse");
    DBUG_RETURN (basecv_zero[T_bool](shp));
}

/******************************************************************************
 *
 * function:
 *   bool COIsZero( constant *a, bool all)
 *   bool COIsOne( constant *a, bool all)
 *   bool COIsTrue( constant *a, bool all)
 *   bool COIsFalse( constant *a, bool all)
 *
 * description:
 *   checks if the given constant consists of 0 or 1 elements (TRUE/FALSE).
 *   if "all" is set to true, the condition must hold for all elements,
 *   if "all" is set to false, the condition must hold for at least one element.
 *
 ******************************************************************************/
bool
COIsZero (constant *a, bool all)
{
    bool result;
    constant *zero;
    constant *eq;
    int i;

    DBUG_ENTER ("COIsZero");
    DBUG_ASSERT ((a != NULL), "COIsZero called with NULL pointer");

    /* create a zero constant with one element */
    zero = COMakeZero (COGetType (a), SHMakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* compare constants (elements must be equal) */
        eq = COEq (a, zero);

        /* compute result dependend of flag "all" */
        if (all) {
            result = TRUE;
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        } else {
            result = FALSE;
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result || ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        }
        eq = COFreeConstant (eq);
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

bool
COIsOne (constant *a, bool all)
{
    bool result;
    constant *one;
    constant *eq;
    int i;

    DBUG_ENTER ("COIsOne");
    DBUG_ASSERT ((a != NULL), "COIsOne called with NULL pointer");

    /* create a "one" constant with one element */
    one = COMakeOne (COGetType (a), SHMakeShape (0));

    /* check for correct constant */
    if (one != NULL) {
        /* compare constants */
        eq = COEq (a, one);

        /* compute result dependend of flag "all" */
        if (all) {
            result = TRUE;
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        } else {
            result = FALSE;
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result || ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        }
        eq = COFreeConstant (eq);
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

bool
COIsTrue (constant *a, bool all)
{
    DBUG_ENTER ("COIsTrue");
    DBUG_RETURN (COIsOne (a, all));
}

bool
COIsFalse (constant *a, bool all)
{
    DBUG_ENTER ("COIsFalse");
    DBUG_RETURN (COIsZero (a, all));
}
