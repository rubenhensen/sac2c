/*
 *
 * $Log$
 * Revision 1.34  2004/12/14 17:05:43  ktr
 * COAST2constant corrected.
 *
 * Revision 1.33  2004/12/09 14:46:09  sah
 * fixed a not copied shape! this caused
 * lots of unpredicted behaviour!
 *
 * Revision 1.32  2004/12/08 18:03:14  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.31  2004/12/07 20:36:16  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 1.30  2004/11/27 01:18:05  ktr
 * Fixed some things.
 *
 * Revision 1.29  2004/11/27 00:17:33  jhb
 * functionsname fixed with the header names
 *
 * Revision 1.28  2004/11/26 23:52:43  mwe
 * *** empty log message ***
 *
 * Revision 1.27  2004/11/26 23:46:57  mwe
 * changes according to fit header file
 *
 * Revision 1.26  2004/11/26 16:09:53  jhb
 * compile
 *
 * Revision 1.25  2004/11/26 14:25:47  sbs
 * change run
 *
 * Revision 1.24  2004/11/09 14:03:00  mwe
 * code for type upgrade added
 * use ntype-structure instead of types-structure
 * new code deactivated by MWE_NTYPE_READY macro
 *
 * Revision 1.23  2004/07/23 15:23:45  ktr
 * unnecessary comment removed.
 *
 * Revision 1.22  2003/09/26 10:14:23  sbs
 * COisEmptyVect added
 *
 * [eliminated]
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
 * - The only function for freeing a shape structure is COfreeConstant!
 * - If the result is a shape structure, it has been freshly allocated!
 */

#include <stdlib.h>

#include "globals.h"
#include "free.h"
#include "dbug.h"
#include "shape.h"
#include "cv2scalar.h"
#include "cv2cv.h"
#include "cv2str.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "basecv.h"
#include "internal_lib.h"
#include "new_types.h"
#include "new_typecheck.h"

#include "constants.h"
/*
 * Now, we include the own interface! The reason fot this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 */

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
 *    constant *COINTmakeConstant( simpletype type, shape *shp, void *elems, int vlen)
 *
 * description:
 *    most generic function for creating constant structures. It simply assembles
 *    a constant from the function's arguments. However, if vlen - the length
 *    of the unrolling - is not yet known, one should use the exported version
 *    of this function, i.e. COmakeConstant, which is almost identical but does
 *    require vlen to be specified explicitly!
 *
 ******************************************************************************/

constant *
COINTmakeConstant (simpletype type, shape *shp, void *elems, int vlen)
{
    constant *res;

    DBUG_ENTER ("COINTmakeConstant");

    res = (constant *)ILIBmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = shp;
    CONSTANT_ELEMS (res) = elems;
    CONSTANT_VLEN (res) = vlen;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void *COINTallocCV( simpletype type, int length)
 *
 * description:
 *    internal function for allocating a CV of length len.
 *
 ******************************************************************************/

void *
COINTallocCV (simpletype type, int length)
{
    void *res;

    DBUG_ENTER ("COINTallocCV");

    res = (void *)ILIBmalloc (global.basetype_size[type] * length);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void *COINTickNElemsFromCV( simpletype type, void *elems, int offset, int length)
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
COINTpickNElemsFromCV (simpletype type, void *elems, int offset, int length)
{
    void *res;

    DBUG_ENTER ("COINTpickNElemsFromCV");

    res = COINTallocCV (type, length);
    global.cv2cv[type](elems, offset, length, res, 0);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void COINTcopyElemsFromCVToCV( simpletype type, void *from, int off, int len,
 *                                               void *to, int to_off)
 *
 * description:
 *    this internal function is very similar to PickNElemsFromCV. The only
 *    difference is that here, the target CV is not created, but expected
 *    to be specified as an extra argument!
 *
 ******************************************************************************/

void
COINTcopyElemsFromCVToCV (simpletype type, void *from, int off, int len, void *to,
                          int to_off)
{
    DBUG_ENTER ("COINTcopyElemsFromCVToCV");

    global.cv2cv[type](from, off, len, to, to_off);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void COINTdbugPrintBinOp( char *fun,
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
COINTdbugPrintBinOp (char *fun, constant *arg1, constant *arg2, constant *res)
{
    DBUG_ENTER ("COINTdbugPrintBinOp");

    fprintf (stderr, "%s applied to\n ", fun);
    COprintConstant (stderr, arg1);
    fprintf (stderr, " ");
    COprintConstant (stderr, arg2);
    fprintf (stderr, "results in: ");
    COprintConstant (stderr, res);

    DBUG_VOID_RETURN;
}

#endif

/******************************************************************************
 *
 * function:
 *   void COINTdbugPrintUnaryOp( char *fun,
 *                          constant *arg1,
 *                          constant *res)
 *
 * description:
 *   logs the arg and result of unary ops to stderr.
 *
 ******************************************************************************/

#ifndef DBUG_OFF

void
COINTdbugPrintUnaryOp (char *fun, constant *arg1, constant *res)
{
    DBUG_ENTER ("COINTdbugPrintUnaryOp");

    fprintf (stderr, "%s applied to\n ", fun);
    COprintConstant (stderr, arg1);
    fprintf (stderr, "results in: ");
    COprintConstant (stderr, res);

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

    res = (constant *)ILIBmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = SHmakeShape (0);
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
 *    constant *COmakeConstant( simpletype type, shape * shp, void *elems)
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
COmakeConstant (simpletype type, shape *shp, void *elems)
{
    constant *res;

    DBUG_ENTER ("COmakeConstant");

    res = (constant *)ILIBmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = shp;
    CONSTANT_ELEMS (res) = elems;
    CONSTANT_VLEN (res) = SHgetUnrLen (shp);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COmakeConstantFromInt( int val)
 *
 * description:
 *    creates an integer constant from a C-integer value.
 *
 ******************************************************************************/

constant *
COmakeConstantFromInt (int val)
{
    constant *res;
    int *intelems;

    DBUG_ENTER ("COmakeConstantFromInt");

    res = (constant *)ILIBmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHmakeShape (0);
    intelems = (int *)ILIBmalloc (sizeof (int));
    intelems[0] = val;
    CONSTANT_ELEMS (res) = intelems;
    CONSTANT_VLEN (res) = 1;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COmakeConstantFromShape( shape *shp)
 *
 *   @brief  create a constant from a shape structure.
 *           ATTENTION: shp is NOT REUSED here!!!
 *
 *   @param shp shape structure to be converted.
 *   @return the freshly created constant.
 *
 ******************************************************************************/

constant *
COmakeConstantFromShape (shape *shp)
{
    constant *res;
    int vlen;

    DBUG_ENTER ("COmakeConstantFromShape");

    vlen = SHgetDim (shp);
    res = (constant *)ILIBmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHcreateShape (1, vlen);
    CONSTANT_ELEMS (res) = SHshape2IntVec (shp);
    CONSTANT_VLEN (res) = vlen;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    simpletype COgetType( constant *a)
 *    int COgetDim( constant *a)
 *    shape *COgetShape( constant *a)
 *    void *GetDataVec( constant *a)
 *
 * description:
 *    several functions for extracting info from constants.
 *
 ******************************************************************************/

simpletype
COgetType (constant *a)
{
    DBUG_ENTER ("COgetType");

    DBUG_RETURN (CONSTANT_TYPE (a));
}

int
COgetDim (constant *a)
{
    DBUG_ENTER ("COgetDim");

    DBUG_RETURN (SHgetDim (CONSTANT_SHAPE (a)));
}

shape *
COgetShape (constant *a)
{
    DBUG_ENTER ("COgetShape");

    DBUG_RETURN (CONSTANT_SHAPE (a));
}

void *
COgetDataVec (constant *a)
{
    DBUG_ENTER ("COgetDataVec");

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
 *    constant *COcopyConstant( constant *a)
 *
 * description:
 *    copies a including all of its sub-structures.
 *
 ******************************************************************************/

constant *
COcopyConstant (constant *a)
{
    constant *res;

    DBUG_ENTER ("COcopyConstant");

    res = COINTmakeConstant (CONSTANT_TYPE (a), SHcopyShape (CONSTANT_SHAPE (a)),
                             COINTpickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a),
                                                    0, CONSTANT_VLEN (a)),
                             CONSTANT_VLEN (a));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COcopyScalar2OneElementVector( constant *a)
 *
 *   @brief creates a 1-element vector from a given scalar.
 *   @param a        scalar constant to be converted.
 *   @return         a new constant vector
 *
 ******************************************************************************/

constant *
COcopyScalar2OneElementVector (constant *a)
{
    constant *res;

    DBUG_ENTER ("COcopyScalar2OneElementVector");

    res = COINTmakeConstant (CONSTANT_TYPE (a), SHcreateShape (1, 1),
                             COINTpickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a),
                                                    0, 1),
                             1);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char * COconstantData2String( int max_char, constant *a);
 *
 *   @brief converts the data vector of the given constant into a string of
 *          comma separated values.
 *
 *          The length of the string to be generated is guaranteed not to exceed
 *          ( max_char + 3 ) characters! E.g. let a = reshape([5],[1,2,3,4,5]),
 *          then COconstantData2String( 20, a) yields: "1,2,3,4,5"
 *               COconstantData2String(  3, a) yields: "1,2..."
 *   @param max_char maximum number of characters used for value printing
 *   @param a        constant whose data vector is to be printed
 *   @return         a freshly allocated string containing the printed values.
 *
 ******************************************************************************/

char *
COconstantData2String (int max_char, constant *a)
{
    char *res;

    DBUG_ENTER ("COconstantData2String");

    res = global.cv2str[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0, CONSTANT_VLEN (a),
                                           max_char);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char * COconstant2String( constant *a);
 *
 *   @brief converts the given constant into a string.
 *
 *   @param a        constant that is to be printed
 *   @return         a freshly allocated string containing the printed values.
 *
 ******************************************************************************/

char *
COconstant2String (constant *a)
{
    static str_buf *buf = NULL;
    char *tmp_str, *tmp2_str, *res;

    DBUG_ENTER ("COconstant2String");

    if (buf == NULL) {
        buf = ILIBstrBufCreate (64);
    }
    tmp_str = SHshape2String (0, CONSTANT_SHAPE (a));
    tmp2_str = COconstantData2String (10000, a);
    buf = ILIBstrBufPrintf (buf, "reshape( %s, [%s])", tmp_str, tmp2_str);
    tmp_str = ILIBfree (tmp_str);
    tmp2_str = ILIBfree (tmp2_str);

    res = ILIBstrBuf2String (buf);
    ILIBstrBufFlush (buf);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape * COconstant2Shape( constant *a)
 *
 *   @brief converts the given constant into a shape vector.
 *
 *          It is implicitly assumed, that a has been checked to be an
 *          int vector!
 *
 *   @param a        constant to be converted
 *   @return         a freshly allocated shape.
 *
 ******************************************************************************/

shape *
COconstant2Shape (constant *a)
{
    int dim, i;
    shape *shp;
    int *dv;

    DBUG_ENTER ("COconstant2Shape");

    DBUG_ASSERT (CONSTANT_TYPE (a) == T_int,
                 "COconstant2Shape applied to non int array!");
    DBUG_ASSERT (SHgetDim (CONSTANT_SHAPE (a)) == 1,
                 "COconstant2Shape applied to non vector!");

    dim = CONSTANT_VLEN (a);
    shp = SHmakeShape (dim);
    dv = (int *)CONSTANT_ELEMS (a);
    for (i = 0; i < dim; i++) {
        shp = SHsetExtent (shp, i, dv[i]);
    }

    DBUG_RETURN (shp);
}

/******************************************************************************
 *
 * function:
 *    void COprintConstant( FILE *file, constant *a)
 *
 * description:
 *    prints the value of a to file
 *
 ******************************************************************************/

void
COprintConstant (FILE *file, constant *a)
{
    char *tmp_str;
    DBUG_ENTER ("COprintConstant");

    fprintf (file, "constant at " F_PTR ": %s ", a, global.mdb_type[CONSTANT_TYPE (a)]);
    SHprintShape (file, CONSTANT_SHAPE (a));
    tmp_str = COconstantData2String (10000, a);
    fprintf (file, " [%s]\n", tmp_str);
    tmp_str = ILIBfree (tmp_str);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    constant *COfreeConstant( constant *a)
 *
 * description:
 *    frees a including all of its sub-structures.
 *
 ******************************************************************************/

constant *
COfreeConstant (constant *a)
{
    DBUG_ENTER ("COfreeConstant");

    CONSTANT_SHAPE (a) = SHfreeShape (CONSTANT_SHAPE (a));
    CONSTANT_ELEMS (a) = ILIBfree (CONSTANT_ELEMS (a));
    a = ILIBfree (a);

    DBUG_RETURN (a);
}

/******************************************************************************
 *
 * function:
 *    node *COconstant2AST( constant *a)
 *
 * description:
 *    This function converts a constant into the according AST representation.
 *    Similar to PickNElemsFromCV, the missing polymorphism of C is resolved by
 *    using yet another function table called "cv2scalar", whose functions are
 *    defined in the file    cv2scalar.c   .
 *    In order to make these functions as minimal as possible, they contain
 *    functions   COcv2<simpletype>( void * elems, int offset),    which
 *    create a suitable node containing the value taken from elems at position
 *    offset.
 *
 ******************************************************************************/

node *
COconstant2AST (constant *a)
{
    node *res, *exprs;
    int dim, i;

    DBUG_ENTER ("COconstant2AST");

    dim = COgetDim (a);
    if (dim == 0) {
        res = global.cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0);
    } else {
        /* First, we build the exprs! */
        exprs = NULL;
        for (i = CONSTANT_VLEN (a) - 1; i >= 0; i--) {
            exprs
              = TBmakeExprs (global.cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), i),
                             exprs);
        }
        /* Finally, the N_array node is created! */
        res = TBmakeArray (SHcopyShape (COgetShape (a)), exprs);
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
COaST2Constant (node *n)
{
    constant *new_co;
    void *element;
    ntype *atype;
    simpletype simple;

    DBUG_ENTER ("COAST2Constant");

    if ((n != NULL) && (COisConstant (n))) {
        /* convert the given constant node */

        switch (NODE_TYPE (n)) {
        case N_num:
            element = (int *)ILIBmalloc (sizeof (int));
            *((int *)element) = NUM_VAL (n);
            new_co = COmakeConstant (T_int, SHmakeShape (0), element);
            break;

        case N_double:
            element = (double *)ILIBmalloc (sizeof (double));
            *((double *)element) = DOUBLE_VAL (n);
            new_co = COmakeConstant (T_double, SHmakeShape (0), element);
            break;

        case N_float:
            element = (float *)ILIBmalloc (sizeof (float));
            *((float *)element) = FLOAT_VAL (n);
            new_co = COmakeConstant (T_float, SHmakeShape (0), element);
            break;

        case N_bool:
            element = (bool *)ILIBmalloc (sizeof (bool));
            *((bool *)element) = BOOL_VAL (n);
            new_co = COmakeConstant (T_bool, SHmakeShape (0), element);
            break;

        case N_array:
            atype = NTCnewTypeCheck_Expr (n);
            if (TYisAKS (atype) || TYisAKV (atype)) {
                simple = TYgetSimpleType (TYgetScalar (atype));
                new_co = COmakeConstant (simple, SHcopyShape (TYgetShape (atype)),
                                         TCarray2Vec (simple, ARRAY_AELEMS (n), NULL));
            } else {
                new_co = NULL;
            }
            atype = TYfreeType (atype);
            break;

        case N_id:
            /*
             * TODO
             * replace AVIS_SSACONST with AKV types
             */
            new_co = COcopyConstant (AVIS_SSACONST (ID_AVIS (n)));
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
 *    bool COisConstant( node *n)
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
COisConstant (node *n)
{
    bool res;

    DBUG_ENTER ("COisConstant");

    if (n != NULL) {
        switch (NODE_TYPE (n)) {
        case N_num:
        case N_double:
        case N_float:
        case N_bool:
            res = TRUE;
            break;

        case N_array:
            res = TCisConstArray (n);
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
 *   constant *COmakeZero( simpletype type, shape *shp)
 *   constant *COmakeOne( simpletype type, shape *shp)
 *   constant *COmakeTrue( simpletype type, shape *shp)
 *   constant *COmakeFalse( simpletype type, shape *shp)
 *
 * description:
 *   create a constant of the given basetype and shape filled with 0 or 1
 *   (for boolean FALSE/TRUE). if no constant can be created for this basetype
 *   the result will be NULL.
 *
 ******************************************************************************/
constant *
COmakeZero (simpletype type, shape *shp)
{
    DBUG_ENTER ("COmakeZero");

    DBUG_RETURN (global.basecv_zero[type](shp));
}

constant *
COmakeOne (simpletype type, shape *shp)
{
    DBUG_ENTER ("COmakeOne");

    DBUG_RETURN (global.basecv_one[type](shp));
}

constant *
COmakeTrue (shape *shp)
{
    DBUG_ENTER ("COmakeTrue");

    DBUG_RETURN (global.basecv_one[T_bool](shp));
}

constant *
COmakeFalse (shape *shp)
{
    DBUG_ENTER ("COmakeFalse");

    DBUG_RETURN (global.basecv_zero[T_bool](shp));
}

/******************************************************************************
 *
 * function:
 *   bool COisZero( constant *a, bool all)
 *   bool COisOne( constant *a, bool all)
 *   bool COisTrue( constant *a, bool all)
 *   bool COisFalse( constant *a, bool all)
 *
 * description:
 *   checks if the given constant consists of 0 or 1 elements (TRUE/FALSE).
 *   if "all" is set to true, the condition must hold for all elements,
 *   if "all" is set to false, the condition must hold for at least one element.
 *
 ******************************************************************************/

bool
COisZero (constant *a, bool all)
{
    bool result;
    constant *zero;
    constant *eq;
    int i;

    DBUG_ENTER ("COisZero");

    DBUG_ASSERT ((a != NULL), "COisZero called with NULL pointer");

    /* create a zero constant with one element */
    zero = COmakeZero (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* compare constants (elements must be equal) */
        eq = COeq (a, zero);

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
        eq = COfreeConstant (eq);
        zero = COfreeConstant (zero);
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

bool
COisOne (constant *a, bool all)
{
    bool result;
    constant *one;
    constant *eq;
    int i;

    DBUG_ENTER ("COisOne");

    DBUG_ASSERT ((a != NULL), "COisOne called with NULL pointer");

    /* create a "one" constant with one element */
    one = COmakeOne (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (one != NULL) {
        /* compare constants */
        eq = COeq (a, one);

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
        eq = COfreeConstant (eq);
        one = COfreeConstant (one);
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

bool
COisTrue (constant *a, bool all)
{
    DBUG_ENTER ("COisTrue");

    DBUG_RETURN (COisOne (a, all));
}

bool
COisFalse (constant *a, bool all)
{
    DBUG_ENTER ("COisFalse");

    DBUG_RETURN (COisZero (a, all));
}

bool
COisEmptyVect (constant *a)
{
    bool result;
    DBUG_ENTER ("COisEmptyVect");
    result = ((COgetDim (a) == 1) && (SHgetExtent (COgetShape (a), 0) == 0));
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    bool COcompareConstants( constant *c1, constant *c2)
 *
 * description:
 *    checks two constants for being equal in type, shape and all elements.
 *
 ******************************************************************************/

bool
COcompareConstants (constant *c1, constant *c2)
{
    bool result;
    constant *eq;
    int i;

    DBUG_ENTER ("COcompareConstants");

    result = FALSE;

    /* compare structural data */
    if ((COgetType (c1) == COgetType (c1)) && (COgetDim (c1) == COgetDim (c1))
        && (SHcompareShapes (COgetShape (c1), COgetShape (c2)))) {

        /* compare constant elements */
        eq = COeq (c1, c2);

        /* check result for all elements */
        result = TRUE;
        for (i = 0; i < CONSTANT_VLEN (eq); i++) {
            result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
        }
        eq = COfreeConstant (eq);

    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}
