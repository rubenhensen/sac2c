/*
 * $Log$
 * Revision 1.2  1999/10/22 14:12:24  sbs
 * inserted comments and added reshape, take, drop, and psi with non
 * scalar results.
 * ..
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
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
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a constant is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 *   There are EXACTLY ONE CLASSE OF FUNCTIONS that is an EXEPTION OF
 *   THIS RULE: - the GETxyz - functions for extracting components of constants
 * - The only function for freeing a shape structure is COFreeConstant!
 * - If the result is a shape structure, it has been freshly allocated!
 *
 */

#include "globals.h"
#include "free.h"
#include "dbug.h"
#include "shape.h"
#include "cv2scalar.h"
#include "cv2cv.h"
#include "tree_compound.h"

typedef struct CONSTANT {
    simpletype type;
    shape *shape;
    void *elems;
    int vlen;
} constant;

/*
 * Now, we include the own interface! The reason fot this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 * The only problem this technique has is that we either have to "export"
 * the definition of the the type "constant" (which we want to avoid for
 * software engeneering reasons) or we have to include all but the
 *   typedef void constant;
 * We achieve the later by using the compilation flag "SELF"
 */

#define SELF
#include "constants.h"

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define CONSTANT_TYPE(c) (c->type)
#define CONSTANT_SHAPE(c) (c->shape)
#define CONSTANT_ELEMS(c) (c->elems)
#define CONSTANT_VLEN(c) (c->vlen)

#define CONSTANT_DIM(c) (SHGetDim (CONSTANT_SHAPE (c)))

/******************************************************************************
 ***
 *** local helper functions:
 ***
 ******************************************************************************/
/******************************************************************************
 *
 * function:
 *    constant *MakeConstant( simpletype type, shape * shp, void *elems, int vlen)
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
 *
 * function:
 *    void *AllocCV( simpletype type, int len)
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
    res = (void *)MALLOC (simpletype_size[type] * length);
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
 *    int Idx2Offset( constant *idx, constant *a)
 *
 * description:
 *    internal function for computing the offset of that element in the unrolling
 *    of a that is referred to by the integer vector idx.
 *    In case of an empty array as idx, 0 is returned!
 *
 ******************************************************************************/

int
Idx2Offset (constant *idx, constant *a)
{
    int offset;
    int *cvidx;
    int lenidx;
    shape *shp;
    int lenshp;
    int i;

    DBUG_ENTER ("Idx2Offset");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "Idx2Offset called with non-int index");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "Idx2Offset called with non-vector index");

    cvidx = (int *)CONSTANT_ELEMS (idx);
    lenidx = SHGetExtent (CONSTANT_SHAPE (idx), 0);

    shp = CONSTANT_SHAPE (a);
    lenshp = SHGetDim (shp);

    DBUG_ASSERT ((lenshp >= lenidx), "Idx2Offset called with longer idx than array dim");

    if (lenidx > 0) {
        offset = cvidx[0];
    } else {
        offset = 0;
    }
    for (i = 1; i < lenidx; i++) {
        offset = offset * SHGetExtent (shp, i) + cvidx[i];
    }
    for (; i < lenshp; i++) {
        offset *= SHGetExtent (shp, i);
    }

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *    constant *IncrementIndex( constant * min, constant *idx, constant *max)
 *
 * description:
 *    internal function for generating new indices idx between min and max!
 *
 *    ATTENTION: this function does not obey the normal rules concerning memory
 *    management of arguments!
 *    This function actually MODIFIES its FIRST ARGUMENT "idx" and returns it
 *    as result!
 *    Furthermore, once idx equals max, "idx" is FREED and a NULL pointer is
 *    returned!
 *
 ******************************************************************************/

constant *
IncrementIndex (constant *min, constant *idx, constant *max)
{
    int dim;

    DBUG_ENTER ("IncrementIndex");

    dim = ((int *)CONSTANT_ELEMS (idx))[CONSTANT_VLEN (idx) - 1];
    while ((((int *)CONSTANT_ELEMS (idx))[dim] == ((int *)CONSTANT_ELEMS (max))[dim])
           && (dim > 0)) {
        ((int *)CONSTANT_ELEMS (idx))[dim] = ((int *)CONSTANT_ELEMS (min))[dim];
        dim--;
    }
    if (((int *)CONSTANT_ELEMS (idx))[dim] == ((int *)CONSTANT_ELEMS (max))[dim]) {
        COFreeConstant (idx);
        idx = NULL;
    } else {
        ((int *)CONSTANT_ELEMS (idx))[dim] += 1;
    }

    DBUG_RETURN (idx);
}

/******************************************************************************
 *
 * function:
 *    constant *TileFromArray( constant *min, shape *res_shp, constant *a)
 *
 * description:
 *    internal function for creating an array of shape "res_shp" whose elems
 *    are copied from "a" starting at offset "min". Note here, that len(idx)
 *    may be smaller than dim( a) !!! If so, the following assumption is made:
 *    Let m = len(idx) n=dim(a), then res_shp[m] <= shape(a)[m]
 *    && forall k in {m+1, ..., n-1} : res_shp[k] == shape(a)[k]!!
 *
 *    This allows us to copy chunks of  res_shp[m] *... * res_shp[n-1] elems!
 *
 ******************************************************************************/

constant *
TileFromArray (constant *min, shape *res_shp, constant *a)
{
    int res_vlen, i, chunk_size, res_off;
    void *res_elems;
    constant *max, *idx, *res;

    DBUG_ENTER ("TileFromArray");
    DBUG_ASSERT ((CONSTANT_TYPE (min) == T_int), "TileFromArray applied to non-int!");
    DBUG_ASSERT ((CONSTANT_DIM (min) == 1), "TileFromArray applied to non-vector!");

    /*
     * First, we allocate the CV for the result:
     */
    res_vlen = SHGetUnrLen (res_shp);
    res_elems = AllocCV (CONSTANT_TYPE (a), res_vlen);

    /*
     * Now, we compute the maximum index!
     */
    max = COCopyConstant (min);
    for (i = 0; i < CONSTANT_VLEN (min); i++) {
        ((int *)CONSTANT_ELEMS (max))[i] += SHGetExtent (res_shp, i) - 1;
    }

    /*
     * Now, we copy the desired values from a to the new CV:
     *
     */
    chunk_size = 1;
    for (i = CONSTANT_VLEN (min); i < CONSTANT_DIM (a); i++) {
        chunk_size *= SHGetExtent (res_shp, i);
    }
    res_off = 0;
    idx = COCopyConstant (min);
    do {
        CopyElemsFromCVToCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), Idx2Offset (idx, a),
                             chunk_size, res_elems, res_off);
        res_off += chunk_size;
        idx = IncrementIndex (min, idx, max); /* This function eventually frees idx!!! */
    } while (idx != NULL);
    COFreeConstant (max);

    /*
     * Finally, the resulting constant node is created:
     */
    res = MakeConstant (CONSTANT_TYPE (a), res_shp, res_elems, res_vlen);

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
 *    constant *COMakeConstantFromArray( node * a)
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
 *    simpletype COGetType( constant * a)
 *    int COGetDim( constant * a)
 *    shape * COGetShape( constant * a)
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

/******************************************************************************
 ***
 *** Functions for handling / converting constants:
 ***
 ******************************************************************************/
/******************************************************************************
 *
 * function:
 *    constant * COCopyConstant( constant *a)
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
 *    void COFreeConstant( constant *a)
 *
 * description:
 *    frees a including all of its sub-structures.
 *
 ******************************************************************************/

void
COFreeConstant (constant *a)
{
    DBUG_ENTER ("COFreeConstant");

    SHFreeShape (CONSTANT_SHAPE (a));
    FREE (CONSTANT_ELEMS (a));
    FREE (a);

    DBUG_VOID_RETURN;
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
        ARRAY_TYPE (res) = MakeType (CONSTANT_TYPE (a), CONSTANT_DIM (a),
                                     SHShape2OldShpseg (CONSTANT_SHAPE (a)), NULL, NULL);
        /*
         * Note here, that in some situation the calling function has to add
         * constvec infos. This is not done here, since it is not yet clear how
         * the representation of constant arrays should be in the long term....
         */
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 *** Operations on constants:
 ***   Here, you will find those functions of the MOA stuff implemented that
 ***   belongs to the intrinsic functions of SAC.
 ***   AT the time being, we have implemented the following operations:
 ***
 ***   - reshape: changing an arrays shape provided the new shp matches
 ***   - psi: element/subarray selection
 ***   - take: picking elements from the upper left corner!
 ***   - drop: picking elements from the lower right corner!
 ***
 ***
 ******************************************************************************/
/******************************************************************************
 *
 * function:
 *    constant *COReshape( constant *new_shp, constant *a)
 *
 * description:
 *    returns a constant with shape new_shp and elemnts from a, provided
 *    that       prod( new_shp) == prod( shape(a))     holds!
 *
 ******************************************************************************/

constant *
COReshape (constant *new_shp, constant *a)
{
    void *elems;
    int res_vlen, curr_ext_res, i;
    shape *res_shp;
    constant *res;

    DBUG_ENTER ("COReshape");
    DBUG_ASSERT ((CONSTANT_TYPE (new_shp) == T_int), "new_shp for COReshape not int!");
    DBUG_ASSERT ((CONSTANT_DIM (new_shp) == 1), "new_shp for COReshape not vector!");

    /*
     * First, we create the result shape:
     *
     * res_shp = new_shp!
     */
    res_shp = SHMakeShape (CONSTANT_VLEN (new_shp));
    for (i = 0; i < CONSTANT_VLEN (new_shp); i++) {
        curr_ext_res = ((int *)CONSTANT_ELEMS (new_shp))[i];
        res_shp = SHSetExtent (res_shp, i, curr_ext_res);
    }

    res_vlen = SHGetUnrLen (res_shp);

    DBUG_ASSERT ((CONSTANT_VLEN (a)) == res_vlen,
                 "new_shp does not match length of the unrolling of a in COReshape!");

    /*
     * Now, we copy the elems CV:
     */
    elems = PickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), 0, res_vlen);
    /*
     * Finally, we create thre result node:
     */
    res = MakeConstant (CONSTANT_TYPE (a), res_shp, elems, res_vlen);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COPsi( constant *idx, constant *a)
 *
 * description:
 *    selects single elements or entire sub-arrays from a.
 *
 ******************************************************************************/

constant *
COPsi (constant *idx, constant *a)
{
    void *elems;
    int res_dim, res_vlen, curr_ext_a, i;
    shape *res_shp;
    constant *res;

    DBUG_ENTER ("COPsi");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COPsi not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to COPsi not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in COPsi!");

    /*
     * First, we create the shape of the result:
     *
     *   res_shp = drop( len(idx), shape(a)!
     */
    res_dim = CONSTANT_DIM (a) - CONSTANT_VLEN (idx); /* correct since dim(idx)==1! */
    res_shp = SHMakeShape (res_dim);
    for (i = 0; i < res_dim; i++) {
        curr_ext_a = SHGetExtent (CONSTANT_SHAPE (a), i + CONSTANT_VLEN (idx));
        res_shp = SHSetExtent (res_shp, i, curr_ext_a);
    }

    /*
     * Now we pick the desired elems from a:
     */
    res_vlen = SHGetUnrLen (res_shp);
    elems = PickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), Idx2Offset (idx, a),
                              res_vlen);
    /*
     * Finally, the result node is created:
     */
    res = MakeConstant (CONSTANT_TYPE (a), res_shp, elems, res_vlen);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COTake( constant *idx, constant *a)
 *
 * description:
 *    takes elements from the upper left corner of a. The shape of the result
 *    is idx ++ drop( len(idx), shape(a))!
 *    ATTENTION: expects all components of idx to be positive!
 *
 ******************************************************************************/

constant *
COTake (constant *idx, constant *a)
{
    shape *res_shp;
    int i, curr_val_idx;
    int off_len;
    shape *off_shp;
    void *off_elems;
    constant *off, *res;

    DBUG_ENTER ("COTake");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COTake not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to COTake not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in COTake!");

    /*
     * First, we create the result shape:
     *
     * res_shp = idx ++ drop( len(idx), shape(a))!
     */
    res_shp = SHCopyShape (CONSTANT_SHAPE (a));

    for (i = 0; i < CONSTANT_VLEN (idx); i++) {
        curr_val_idx = ((int *)CONSTANT_ELEMS (idx))[i];
        res_shp = SHSetExtent (res_shp, i, curr_val_idx);
    }

    /*
     * Now, we create an offset-vector with length len(idx)-1 and values 0!
     * ( This usually guarantees pretty big chunks to be copied at once)
     */
    off_shp = SHMakeShape (1);
    off_len = CONSTANT_VLEN (idx) - 1;
    SHSetExtent (off_shp, 0, off_len);

    off_elems = AllocCV (T_int, off_len);
    for (i = 0; i < off_len; i++) {
        ((int *)off_elems)[i] = 0;
    }
    off = MakeConstant (T_int, off_shp, off_elems, off_len);

    /*
     * Finally, we pick the tile of shp res_shp from a starting at pos off!
     */
    res = TileFromArray (off, res_shp, a);
    COFreeConstant (off);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CODrop( constant *idx, constant *a)
 *
 * description:
 *    picks elements from the lower right corner of a! The shape of the result
 *    equals (take( len(idx), shape(a)) - idx ) ++ drop( len(idx), shape(a)).
 *    ATTENTION: expects all components of idx to be positive!
 *
 ******************************************************************************/

constant *
CODrop (constant *idx, constant *a)
{
    shape *res_shp;
    int i, curr_val_idx;
    int off_len;
    shape *off_shp;
    void *off_elems;
    constant *off, *res;

    DBUG_ENTER ("CODrop");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to CODrop not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to CODrop not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in CODrop!");

    /*
     * First, we create the result shape:
     *
     * res_shp = (take( len(idx), shape(a)) - idx ) ++ drop( len(idx), shape(a))!
     */
    res_shp = SHCopyShape (CONSTANT_SHAPE (a));

    for (i = 0; i < CONSTANT_VLEN (idx); i++) {
        curr_val_idx = SHGetExtent (res_shp, i) - ((int *)CONSTANT_ELEMS (idx))[i];
        res_shp = SHSetExtent (res_shp, i, curr_val_idx);
    }

    /*
     * Now, we create an offset-vector off = drop( -1 , idx)!
     * ( This usually guarantees pretty big chunks to be copied at once)
     */
    off_shp = SHMakeShape (1);
    off_len = CONSTANT_VLEN (idx) - 1;
    SHSetExtent (off_shp, 0, off_len);

    off_elems = AllocCV (T_int, off_len);
    for (i = 0; i < off_len; i++) {
        ((int *)off_elems)[i] = ((int *)CONSTANT_ELEMS (idx))[i];
    }
    off = MakeConstant (T_int, off_shp, off_elems, off_len);

    /*
     * Finally, we pick the tile of shp res_shp from a starting at pos off!
     */
    res = TileFromArray (off, res_shp, a);
    COFreeConstant (off);

    DBUG_RETURN (res);
}
