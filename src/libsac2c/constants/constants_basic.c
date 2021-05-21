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
#include <stdarg.h>

#include "constants.h"

#include "globals.h"
#include "free.h"

#define DBUG_PREFIX "CO"
#include "debug.h"

#include "shape.h"
#include "cv2scalar.h"
#include "cv2cv.h"
#include "cv2str.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "basecv.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "check_mem.h"

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
COINTmakeConstant (simpletype type, shape *shp, void *elems, size_t vlen)
{
    constant *res;

    DBUG_ENTER ();

    res = (constant *)MEMmalloc (sizeof (constant));
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
COINTallocCV (simpletype type, size_t length)
{
    void *res;

    DBUG_ENTER ();

    res = (void *)MEMmalloc (global.basetype_size[type] * length);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * function:
 *    void *COINTcopyCVToMem(simpletype type, int length, void *cv)
 *
 * description:
 *    internal function to copy a cv into a new mem area.
 *
 ******************************************************************************/

static void *
COINTcopyCVToMem (simpletype type, size_t length, void *cv)
{
    DBUG_ENTER ();

    void *res;

    res = MEMcopy ((global.basetype_size[type] * length), cv);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * function:
 *    void *COINTcopyCVVaListToMem(simpletype type, int length, va_list cv)
 *
 * description:
 *    internal function to copy a cv variable list into a new mem area.
 *
 ******************************************************************************/

static void *
COINTcopyCVVaListToMem (simpletype type, size_t length, va_list cv)
{
    DBUG_ENTER ();

    void *res;
    size_t i;

    res = MEMmalloc (global.basetype_size[type] * length);
    switch (type) {
#define CASE(SacType, CType)                                                             \
    case SacType: {                                                                      \
        CType *dst = (CType *)res;                                                       \
        for (i = 0; i < length; ++i)                                                     \
            dst[i] = va_arg (cv, CType);                                                 \
    } break

        CASE (T_int, int);
        CASE (T_long, long);

    default:
        CTIabort ("unknown type in CV list");
    }

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
COINTpickNElemsFromCV (simpletype type, void *elems, size_t offset, size_t length)
{
    void *res;

    DBUG_ENTER ();

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
COINTcopyElemsFromCVToCV (simpletype type, void *from, size_t off, size_t len, void *to,
                          size_t to_off)
{
    DBUG_ENTER ();

    global.cv2cv[type](from, off, len, to, to_off);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (stderr, "%s applied to\n ", fun);
    COprintConstant (stderr, arg1);
    fprintf (stderr, " ");
    COprintConstant (stderr, arg2);
    fprintf (stderr, "results in: ");
    COprintConstant (stderr, res);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (stderr, "%s applied to\n ", fun);
    COprintConstant (stderr, arg1);
    fprintf (stderr, "results in: ");
    COprintConstant (stderr, res);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    res = (constant *)MEMmalloc (sizeof (constant));
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

    DBUG_ENTER ();

    res = (constant *)MEMmalloc (sizeof (constant));
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

    DBUG_ENTER ();

    res = (constant *)MEMmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHmakeShape (0);
    intelems = (int *)MEMmalloc (sizeof (int));
    intelems[0] = val;
    CONSTANT_ELEMS (res) = intelems;
    CONSTANT_VLEN (res) = 1;

    DBUG_RETURN (res);
}

constant *
COmakeConstantFromFloat (float val)
{
    constant *res;
    float *floatelems;

    DBUG_ENTER ();

    res = (constant *)MEMmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = T_float;
    CONSTANT_SHAPE (res) = SHmakeShape (0);
    floatelems = (float *)MEMmalloc (sizeof (float));
    floatelems[0] = val;
    CONSTANT_ELEMS (res) = floatelems;
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
    size_t vlen;

    DBUG_ENTER ();

    vlen = SHgetDim (shp);
    res = (constant *)MEMmalloc (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHcreateShape (1, vlen);
    CONSTANT_ELEMS (res) = SHshape2IntVec (shp);
    CONSTANT_VLEN (res) = vlen;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COmakeConstantFromDynamicArguments(int dim, ...)
 *
 *   @brief creates a constant from dynamic list of arguments.
 *
 *   @param simpletype type type of the elements
 *   @param int dim dimensionality of the constant.
 *   @param ... shapevalues and initializationvalues
 *
 *   @return the freshly created constant.
 *
 ******************************************************************************/

constant *
COmakeConstantFromDynamicArguments (simpletype type, int dim, ...)
{
    DBUG_ENTER ();

    /* Pointer for dynamic arguments*/
    va_list Argp;

    /* constant parts*/
    shape *res_shape = NULL;
    void *res_elems = NULL;
    constant *res = NULL;

    /* constant facilities*/
    size_t res_elems_num = 0;

    /* Counter*/
    int i = 0;

    /* Construct shape*/
    res_shape = SHmakeShape (dim);

    if (dim > 0) {
        va_start (Argp, dim);

        /* Set Shape*/
        for (i = 0; i < dim; i++) {
            res_shape = SHsetExtent (res_shape, i, va_arg (Argp, int));
        }

        res_elems_num = SHgetUnrLen (res_shape);

        /* if #elems > 0 copy these elements into memory*/
        if (res_elems_num > 0) {
            res_elems = COINTcopyCVVaListToMem (type, res_elems_num, Argp);
        }
        va_end (Argp);
    }

    /* Contruct constant*/
    res = COINTmakeConstant (T_int, res_shape, res_elems, res_elems_num);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *COmakeConstantFromArray(simpletype type, int dim, int *shp,
 *                                       void *elems)
 *
 *    @brief creates a constant out of a given array of elems.
 *
 *    @param simpletype type type of the elemens
 *    @param int dim dimensionality of the constant
 *    @param int *shp shape of the constant
 *    @param void *elems elems of the constant
 *
 *    @return the freshly created constant
 ******************************************************************************/

constant *
COmakeConstantFromArray (simpletype type, int dim, int *shp, void *elems)
{
    DBUG_ENTER ();

    /* constant parts*/
    shape *res_shape = NULL;
    void *res_elems = NULL;
    constant *res = NULL;

    /* constant facilities*/
    size_t res_elems_num = 0;

    /* counters*/
    int i = 0;

    /* construct shape*/
    res_shape = SHmakeShape (dim);
    if (dim > 0) {

        /* set shape*/
        for (i = 0; i < dim; i++) {
            res_shape = SHsetExtent (res_shape, i, shp[i]);
        }

        res_elems_num = SHgetUnrLen (res_shape);

        /* if #elems > 0 copy elems to mem*/
        if (res_elems_num > 0) {
            res_elems = COINTcopyCVToMem (type, res_elems_num, elems);
        }
    }

    /* construct entire constant*/
    res = COINTmakeConstant (type, res_shape, res_elems, res_elems_num);

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
    DBUG_ENTER ();

    DBUG_RETURN (CONSTANT_TYPE (a));
}

int
COgetDim (constant *a)
{
    DBUG_ENTER ();

    DBUG_RETURN (SHgetDim (CONSTANT_SHAPE (a)));
}

shape *
COgetShape (constant *a)
{
    DBUG_ENTER ();

    DBUG_RETURN (CONSTANT_SHAPE (a));
}

int
COgetExtent (constant *a, int i)
{
    DBUG_ENTER ();

    DBUG_ASSERT (i < CONSTANT_DIM (a), "COgetExtent called with illegal dim spec");

    DBUG_RETURN (SHgetExtent (CONSTANT_SHAPE (a), i));
}

void *
COgetDataVec (constant *a)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
COconstantData2String (size_t max_char, constant *a)
{
    char *res;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    if (buf == NULL) {
        buf = SBUFcreate (64);
    }
    tmp_str = SHshape2String (0, CONSTANT_SHAPE (a));
    tmp2_str = COconstantData2String (10000, a);
    buf = SBUFprintf (buf, "reshape( %s, [%s])", tmp_str, tmp2_str);
    tmp_str = MEMfree (tmp_str);
    tmp2_str = MEMfree (tmp2_str);

    res = SBUF2str (buf);
    SBUFflush (buf);

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    fprintf (file, "constant at " F_PTR ": %s ", (void *)a,
             global.mdb_type[CONSTANT_TYPE (a)]);
    SHprintShape (file, CONSTANT_SHAPE (a));
    tmp_str = COconstantData2String (10000, a);
    fprintf (file, " [%s]\n", tmp_str);
    tmp_str = MEMfree (tmp_str);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "Constant is NULL!");

    CONSTANT_SHAPE (a) = SHfreeShape (CONSTANT_SHAPE (a));
    CONSTANT_ELEMS (a) = MEMfree (CONSTANT_ELEMS (a));
    a = MEMfree (a);

    DBUG_RETURN (a);
}

/******************************************************************************
 *
 * function:
 *    void COtouchConstant( constant *a, info *arg_info)
 *
 * description:
 *    touch a including all of its sub-structures.
 *
 ******************************************************************************/

void
COtouchConstant (constant *a, info *arg_info)
{
    DBUG_ENTER ();

    SHtouchShape (CONSTANT_SHAPE (a), arg_info);
    CHKMtouch (CONSTANT_ELEMS (a), arg_info);
    CHKMtouch (a, arg_info);

    DBUG_RETURN ();
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
    int dim;
    size_t i;

    DBUG_ENTER ();

    dim = COgetDim (a);
    if (dim == 0) {
        res = global.cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0);
    } else {
        /* First, we build the exprs! */
        exprs = NULL;
        /* 
         * decrement after check for > 0, safe method for reverse loop ending on 0
         * i : (CONSTANT_VLEN - 1) to 0 
         */
        for (i = CONSTANT_VLEN (a) ; i-- > 0; ) {
            exprs
              = TBmakeExprs (global.cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), i),
                             exprs);
        }
        /* Finally, the N_array node is created! */
        res = TBmakeArray (TYmakeAKS (TYmakeSimpleType (CONSTANT_TYPE (a)),
                                      SHmakeShape (0)),
                           SHcopyShape (COgetShape (a)), exprs);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COaST2Constant( node *n)
 *
 * description:
 *    converts a given node from the AST to a constant.
 *    return NULL if n is not a simple constant.
 *
 *    this function can deal with scalar types like
 *      N_num, N_double, N_float, N_bool
 *    arrays (N_array)
 *    avis ( N_avis with akv type)
 *    and identifier (N_id with akv type)
 *
 ******************************************************************************/

constant *
COaST2Constant (node *n)
{
    constant *new_co;
    void *element;
    ntype *atype;

    DBUG_ENTER ();

    if ((n != NULL) && (COisConstant (n))) {
        /* convert the given constant node */

        switch (NODE_TYPE (n)) {
        case N_numbyte:
            element = (char *)MEMmalloc (sizeof (char));
            *((char *)element) = NUMBYTE_VAL (n);
            new_co = COmakeConstant (T_byte, SHmakeShape (0), element);
            break;

        case N_numshort:
            element = (short *)MEMmalloc (sizeof (short));
            *((short *)element) = NUMSHORT_VAL (n);
            new_co = COmakeConstant (T_short, SHmakeShape (0), element);
            break;

        case N_numint:
            element = (int *)MEMmalloc (sizeof (int));
            *((int *)element) = NUMINT_VAL (n);
            new_co = COmakeConstant (T_int, SHmakeShape (0), element);
            break;

        case N_numlong:
            element = (long *)MEMmalloc (sizeof (long));
            *((long *)element) = NUMLONG_VAL (n);
            new_co = COmakeConstant (T_long, SHmakeShape (0), element);
            break;

        case N_numlonglong:
            element = (long long *)MEMmalloc (sizeof (long long));
            *((long long *)element) = NUMLONGLONG_VAL (n);
            new_co = COmakeConstant (T_longlong, SHmakeShape (0), element);
            break;

        case N_numubyte:
            element = (unsigned char *)MEMmalloc (sizeof (unsigned char));
            *((unsigned char *)element) = NUMUBYTE_VAL (n);
            new_co = COmakeConstant (T_ubyte, SHmakeShape (0), element);
            break;

        case N_numushort:
            element = (unsigned short *)MEMmalloc (sizeof (unsigned short));
            *((unsigned short *)element) = NUMUSHORT_VAL (n);
            new_co = COmakeConstant (T_ushort, SHmakeShape (0), element);
            break;

        case N_numuint:
            element = (unsigned int *)MEMmalloc (sizeof (unsigned int));
            *((unsigned int *)element) = NUMUINT_VAL (n);
            new_co = COmakeConstant (T_uint, SHmakeShape (0), element);
            break;

        case N_numulong:
            element = (unsigned long *)MEMmalloc (sizeof (unsigned long));
            *((unsigned long *)element) = NUMULONG_VAL (n);
            new_co = COmakeConstant (T_ulong, SHmakeShape (0), element);
            break;

        case N_numulonglong:
            element = (unsigned long long *)MEMmalloc (sizeof (unsigned long long));
            *((unsigned long long *)element) = NUMULONGLONG_VAL (n);
            new_co = COmakeConstant (T_ulonglong, SHmakeShape (0), element);
            break;

        case N_num:
            element = (int *)MEMmalloc (sizeof (int));
            *((int *)element) = NUM_VAL (n);
            new_co = COmakeConstant (T_int, SHmakeShape (0), element);
            break;

        case N_double:
            element = (double *)MEMmalloc (sizeof (double));
            *((double *)element) = DOUBLE_VAL (n);
            new_co = COmakeConstant (T_double, SHmakeShape (0), element);
            break;

        case N_float:
            element = (float *)MEMmalloc (sizeof (float));
            *((float *)element) = FLOAT_VAL (n);
            new_co = COmakeConstant (T_float, SHmakeShape (0), element);
            break;

        case N_bool:
            element = (bool *)MEMmalloc (sizeof (bool));
            *((bool *)element) = BOOL_VAL (n);
            new_co = COmakeConstant (T_bool, SHmakeShape (0), element);
            break;

        case N_char:
            element = (char *)MEMmalloc (sizeof (char));
            *((char *)element) = CHAR_VAL (n);
            new_co = COmakeConstant (T_char, SHmakeShape (0), element);
            break;

        case N_array:
            atype = NTCnewTypeCheck_Expr (n);
            if (TYisAKV (atype)) {
                new_co = COcopyConstant (TYgetValue (atype));
            } else {
                new_co = NULL;
            }
            atype = TYfreeType (atype);
            break;

        case N_id:
            n = ID_AVIS (n);
            /* Falls through. */

        case N_avis:
            if (TYisAKV (AVIS_TYPE (n))) {
                new_co = COcopyConstant (TYgetValue (AVIS_TYPE (n)));
            } else {
                new_co = NULL;
            }
            break;

        default:
            DBUG_UNREACHABLE ("missing implementation for given nodetype");
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
 *    this function can deal with scalar types like
 *      N_num, N_double, N_float, N_bool, N_char
 *    arrays (N_array)
 *    avis nodes ( N_avis)
 *    and identifier (N_id with akv type)
 *
 ******************************************************************************/

bool
COisConstant (node *n)
{
    ntype *atype;
    bool res;

    DBUG_ENTER ();

    if (n != NULL) {
        switch (NODE_TYPE (n)) {
        case N_numbyte:
        case N_numshort:
        case N_numint:
        case N_numlong:
        case N_numlonglong:
        case N_numubyte:
        case N_numushort:
        case N_numuint:
        case N_numulong:
        case N_numulonglong:
        case N_num:
        case N_double:
        case N_float:
        case N_bool:
        case N_char:
            res = TRUE;
            break;

        case N_array:
            atype = NTCnewTypeCheck_Expr (n);
            res = TYisAKV (atype);
            atype = TYfreeType (atype);
            break;

        case N_id:
            n = ID_AVIS (n);
            /* Falls through. */

        case N_avis:
            res = (TYisAKV (AVIS_TYPE (n)));
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
 *   constant *COmakeNegativeOne( simpletype type, shape *shp)
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
COmakeNegativeOne (simpletype type, shape *shp)
{
    DBUG_ENTER ();

    DBUG_RETURN (global.basecv_negativeone[type](shp));
}

constant *
COmakeZero (simpletype type, shape *shp)
{
    DBUG_ENTER ();

    DBUG_RETURN (global.basecv_zero[type](shp));
}

constant *
COmakeOne (simpletype type, shape *shp)
{
    DBUG_ENTER ();

    DBUG_RETURN (global.basecv_one[type](shp));
}

constant *
COmakeTrue (shape *shp)
{
    DBUG_ENTER ();

    DBUG_RETURN (global.basecv_one[T_bool](shp));
}

constant *
COmakeFalse (shape *shp)
{
    DBUG_ENTER ();

    DBUG_RETURN (global.basecv_zero[T_bool](shp));
}

/******************************************************************************
 *
 * function:
 *   bool COisZero  ( constant *a, bool all)
 *   bool COisNonNeg( constant *a, bool all)
 *   bool COisNeg   ( constant *a, bool all)
 *   bool COisPos   ( constant *a, bool all)
 *   bool COisOne   ( constant *a, bool all)
 *   bool COisTrue  ( constant *a, bool all)
 *   bool COisFalse ( constant *a, bool all)
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
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "COisZero called with NULL pointer");

    /* create a zero constant with one element */
    zero = COmakeZero (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* compare constants (elements must be equal) */
        eq = COeq (a, zero, NULL);

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
COisNonNeg (constant *a, bool all)
{
    bool result;
    constant *zero;
    constant *eq;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "COisNonNeg called with NULL pointer");

    /* create a "zero" constant with one element */
    zero = COmakeZero (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* vector-compare constants */
        eq = COge (a, zero, NULL);

        /* compute result dependent on flag "all" */
        if (all) {
            result = TRUE; /* and-reduce */
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        } else {
            result = FALSE; /* or-reduce */
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
COisNeg (constant *a, bool all)
{
    bool result;
    constant *zero;
    constant *eq;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "COisNeg called with NULL pointer");

    /* create a "zero" constant with one element */
    zero = COmakeZero (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* vector-compare constants */
        eq = COlt (a, zero, NULL);

        /* compute result dependent on flag "all" */
        if (all) {
            result = TRUE; /* and-reduce */
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        } else {
            result = FALSE; /* or-reduce */
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
COisPos (constant *a, bool all)
{
    bool result;
    constant *zero;
    constant *eq;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "COisPos called with NULL pointer");

    /* create a "zero" constant with one element */
    zero = COmakeZero (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (zero != NULL) {
        /* vector-compare constants */
        eq = COgt (a, zero, NULL);

        /* compute result dependent on flag "all" */
        if (all) {
            result = TRUE; /* and-reduce */
            for (i = 0; i < CONSTANT_VLEN (eq); i++) {
                result = result && ((bool *)(CONSTANT_ELEMS (eq)))[i];
            }
        } else {
            result = FALSE; /* or-reduce */
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
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (a != NULL, "COisOne called with NULL pointer");

    /* create a "one" constant with one element */
    one = COmakeOne (COgetType (a), SHmakeShape (0));

    /* check for correct constant */
    if (one != NULL) {
        /* compare constants */
        eq = COeq (a, one, NULL);

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
    DBUG_ENTER ();

    DBUG_RETURN (COisOne (a, all));
}

bool
COisFalse (constant *a, bool all)
{
    DBUG_ENTER ();

    DBUG_RETURN (COisZero (a, all));
}

bool
COisEmptyVect (constant *a)
{
    bool result;
    DBUG_ENTER ();
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
    size_t i;

    DBUG_ENTER ();

    result = FALSE;

    /* compare structural data */
    if ((COgetType (c1) == COgetType (c1)) && (COgetDim (c1) == COgetDim (c1))
        && (SHcompareShapes (COgetShape (c1), COgetShape (c2)))) {

        /* compare constant elements */
        eq = COeq (c1, c2, NULL);

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

/** <!--********************************************************************-->
 *
 * @fn: void *COcreateAllIndicesAndFold(
 *              shape *shp,
 *              void *(*foldfun)( constant *idx, void *, void *),
 *              void *accu,
 *              void* attr,
 *              bool  scalaridx)
 *
 * @brief: Bodo will write this.
 *
 * @params:     scalaridx: if this is FALSE, the indices are generated
 *              as 1-element vectors. If TRUE, they are generated as scalars.
 *
 ******************************************************************************/

void *
COcreateAllIndicesAndFold (shape *shp, void *(*foldfun) (constant *idx, void *, void *),
                           void *accu, void *attr, bool scalaridx)
{
    constant *idx;
    int *datav;
    int max_d, d, len;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    if (scalaridx) {
        idx = COmakeZero (T_int, SHcreateShape (0, 0));
    } else {
        idx = COmakeZero (T_int, SHcreateShape (1, SHgetDim (shp)));
    }

    datav = (int *)COgetDataVec (idx);
    max_d = SHgetDim (shp) - 1;
    len = SHgetUnrLen (shp);

    if (max_d == -1) {
        accu = foldfun (idx, accu, attr);
    } else if (len > 0) {
        do {
            accu = foldfun (idx, accu, attr);

            DBUG_EXECUTE (tmp_str = COconstant2String (idx));
            DBUG_PRINT ("idx: %s", tmp_str);
            DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

            d = max_d;
            datav[d]++;
            while ((d > 0) && (datav[d] == SHgetExtent (shp, d))) {
                datav[d] = 0;
                d--;
                datav[d]++;
            };
        } while (datav[d] < SHgetExtent (shp, d));
    }

    idx = COfreeConstant (idx);

    DBUG_RETURN (accu);
}

/** <!--********************************************************************-->
 *
 * @fn: int COconst2Int( constant *c)
 *
 * @brief: Create integer from scalar constant c
 *
 ******************************************************************************/

int
COconst2Int (constant *c)
{
    int res;

    DBUG_ENTER ();
    res = ((int *)CONSTANT_ELEMS (c))[0];

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
