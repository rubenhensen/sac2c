/*
 *
 * $Log$
 * Revision 1.13  2004/11/26 14:35:31  sbs
 * compiles
 *
 * Revision 1.12  2004/07/29 15:36:41  khf
 * IdxSel: free on shape removed (caused pointer sharing)
 *
 * Revision 1.11  2004/05/30 13:03:43  khf
 * COIdxSel added
 *
 * Revision 1.10  2003/12/12 08:06:37  sbs
 * Idx2Offset now asserts index compatibility!
 *
 * Revision 1.9  2003/06/11 22:05:36  ktr
 * Added support for multidimensional arrays
 *
 * Revision 1.8  2003/04/07 14:27:36  sbs
 * COCat added.
 * drop and take extended for accepting scalar arguments also 8-)
 *
 * Revision 1.7  2002/06/21 12:28:18  dkr
 * no changes done
 *
 * Revision 1.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 1.5  2001/05/17 14:16:21  nmw
 * MALLOC/FREE replaced by ILIBmalloc/Free, using result of ILIBfree()
 *
 * Revision 1.4  2001/05/07 07:40:00  nmw
 * dbug output corrected
 *
 * Revision 1.3  2001/05/03 16:54:49  nmw
 * COModarray implemented
 *
 * Revision 1.2  2001/03/23 12:49:53  nmw
 * CODim/COShape implemented
 *
 * Revision 1.1  2001/03/02 14:32:58  sbs
 * Initial revision
 *
 */

#include <stdlib.h>
#include "internal_lib.h"
#include "dbug.h"

#include "constants.h"
#include "constants_internal.h"

/******************************************************************************
 ***
 *** local helper functions:
 ***
 ******************************************************************************/

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
    lenidx = SHgetExtent (CONSTANT_SHAPE (idx), 0);

    shp = CONSTANT_SHAPE (a);
    lenshp = SHgetDim (shp);

    DBUG_ASSERT ((lenshp >= lenidx), "Idx2Offset called with longer idx than array dim");

    if (lenidx > 0) {
        DBUG_ASSERT (cvidx[0] < SHgetExtent (shp, 0),
                     "Idx2Offset called with idx out of range");
        offset = cvidx[0];
    } else {
        offset = 0;
    }
    for (i = 1; i < lenidx; i++) {
        DBUG_ASSERT (cvidx[i] < SHgetExtent (shp, i),
                     "Idx2Offset called with idx out of range");
        offset = offset * SHgetExtent (shp, i) + cvidx[i];
    }
    for (; i < lenshp; i++) {
        offset *= SHgetExtent (shp, i);
    }

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *    constant *IncrementIndex( constant *min, constant *idx, constant *max)
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

    dim = CONSTANT_VLEN (idx) - 1;
    if (dim >= 0) {
        /*
         * 'idx' is non-empty
         */
        while (
          (dim > 0)
          && (((int *)CONSTANT_ELEMS (idx))[dim] == ((int *)CONSTANT_ELEMS (max))[dim])) {
            ((int *)CONSTANT_ELEMS (idx))[dim] = ((int *)CONSTANT_ELEMS (min))[dim];
            dim--;
        }
        if (((int *)CONSTANT_ELEMS (idx))[dim] == ((int *)CONSTANT_ELEMS (max))[dim]) {
            idx = COfreeConstant (idx);
        } else {
            (((int *)CONSTANT_ELEMS (idx))[dim])++;
        }
    } else {
        /*
         * 'idx' is empty
         */
        idx = COfreeConstant (idx);
    }

    DBUG_RETURN (idx);
}

/******************************************************************************
 *
 * function:
 *    constant *TileFromArray( constant *idx, shape *res_shp, constant *a)
 *
 * description:
 *    internal function for creating an array of shape "res_shp" whose elems
 *    are copied from "a" starting at offset "idx". Note here, that len(idx)
 *    may be smaller than dim(a) !!! If so, the following assumption is made:
 *    Let m = len(idx) n = dim(a), then
 *       res_shp[m-1] <= shape(a)[m-1]  &&
 *       forall k in {m, ..., n-1} : res_shp[k] == shape(a)[k]
 *    This allows us to copy chunks of  res_shp[m-1] * ... * res_shp[n-1] elems!
 *
 ******************************************************************************/

constant *
TileFromArray (constant *idx, shape *res_shp, constant *a)
{
    int res_vlen, res_off, chunk_size, off_size, off_len, i;
    void *res_elems, *off_elems;
    shape *off_shp;
    constant *min, *max, *off, *res;

    DBUG_ENTER ("TileFromArray");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "TileFromArray applied to non-int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "TileFromArray applied to non-vector!");
    DBUG_ASSERT ((CONSTANT_VLEN (idx) >= 1), "TileFromArray applied to empty vector!");

    /*
     * First, we allocate the CV for the result:
     */
    res_vlen = SHgetUnrLen (res_shp);
    res_elems = AllocCV (CONSTANT_TYPE (a), res_vlen);

    /*
     * Now, we create an offset-vector 'off' = drop( -1, idx)
     * (This usually guarantees pretty big chunks to be copied at once)
     */
    off_shp = SHmakeShape (1);
    off_len = CONSTANT_VLEN (idx) - 1;
    SHsetExtent (off_shp, 0, off_len);
    off_elems = AllocCV (T_int, off_len);
    for (i = 0; i < off_len; i++) {
        ((int *)off_elems)[i] = ((int *)CONSTANT_ELEMS (idx))[i];
    }
    off = MakeConstant (T_int, off_shp, off_elems, off_len);

    /*
     * Now, we compute the minimum and maximum index
     */
    min = COcopyConstant (off);
    max = COcopyConstant (off);
    for (i = 0; i < CONSTANT_VLEN (min); i++) {
        ((int *)CONSTANT_ELEMS (max))[i] += SHgetExtent (res_shp, i) - 1;
    }

    /*
     * calculate the chunk size ('chunk_size') and the number of elements
     * we have omited in 'off' ('off_size')
     * (remember that we are using  off = drop( -1, idx)  instead of  off = idx !!)
     */
    chunk_size = 1;
    for (i = CONSTANT_VLEN (off) + 1; i < CONSTANT_DIM (a); i++) {
        chunk_size *= SHgetExtent (res_shp, i);
    }
    off_size = chunk_size * (((int *)CONSTANT_ELEMS (idx))[CONSTANT_VLEN (idx) - 1]);
    chunk_size *= SHgetExtent (res_shp, CONSTANT_VLEN (off));

    /*
     * Now, we copy the desired values from 'a' to the new CV:
     */
    res_off = 0;
    do {
        CopyElemsFromCVToCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a),
                             Idx2Offset (off, a) + off_size, chunk_size, res_elems,
                             res_off);
        res_off += chunk_size;
        off
          = IncrementIndex (min, off, max); /* This function eventually frees 'off' !!! */
    } while (off != NULL);
    min = COfreeConstant (min);
    max = COfreeConstant (max);

    /*
     * Finally, the resulting constant node is created:
     */
    res = MakeConstant (CONSTANT_TYPE (a), res_shp, res_elems, res_vlen);

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
 ***   - sel: element/subarray selection
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
    res_shp = SHmakeShape (CONSTANT_VLEN (new_shp));
    for (i = 0; i < CONSTANT_VLEN (new_shp); i++) {
        curr_ext_res = ((int *)CONSTANT_ELEMS (new_shp))[i];
        res_shp = SHsetExtent (res_shp, i, curr_ext_res);
    }

    res_vlen = SHgetUnrLen (res_shp);

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

    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COReshape", new_shp, a, res););

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COSel( constant *idx, constant *a)
 *
 * description:
 *    selects single elements or entire sub-arrays from a.
 *
 ******************************************************************************/

constant *
COSel (constant *idx, constant *a)
{
    void *elems;
    int res_dim, res_vlen, curr_ext_a, i;
    shape *res_shp;
    constant *res;

    DBUG_ENTER ("COSel");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COSel not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to COSel not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in COSel!");

    /*
     * First, we create the shape of the result:
     *
     *   res_shp = drop( len(idx), shape(a)!
     */
    res_dim = CONSTANT_DIM (a) - CONSTANT_VLEN (idx); /* correct since dim(idx)==1! */
    res_shp = SHmakeShape (res_dim);
    for (i = 0; i < res_dim; i++) {
        curr_ext_a = SHgetExtent (CONSTANT_SHAPE (a), i + CONSTANT_VLEN (idx));
        res_shp = SHsetExtent (res_shp, i, curr_ext_a);
    }

    /*
     * Now we pick the desired elems from a:
     */
    res_vlen = SHgetUnrLen (res_shp);
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
 *    constant *COIdxSel( constant *idx, constant *a)
 *
 * description:
 *    selects single elements from a.
 *
 ******************************************************************************/

constant *
COIdxSel (constant *idx, constant *a)
{
    void *elem;
    int index;
    shape *res_shp;
    constant *res;

    DBUG_ENTER ("COIdxSel");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COIdxSel not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 0), "idx to COIdxSel not scalar!");
    res_shp = COgetShape (a);
    index = ((int *)COgetDataVec (idx))[0];
    DBUG_ASSERT ((SHgetUnrLen (res_shp)) > index,
                 "idx-scalar exceeds number of elements of array in COIdxSel!");

    /*
     * Pick the desired element from a:
     */

    elem = PickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), index, 1);
    /*
     * Finally, the result node is created:
     */
    res = MakeConstant (CONSTANT_TYPE (a), SHmakeShape (0), elem, 1);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COTake( constant *idx, constant *a)
 *
 * description:
 *    picks elements from a! The shape of the result equals
 *       |idx| ++ drop( len(idx), shape(a)).
 *    In case of positive idx_i, the elements in axis i are picked from the
 *    "beginning", i.e., starting at position 0 upto idx_i - 1.
 *    Otherwise (negative idx_i), the elements in axis i are taken from the
 *    "end", i.e., from shape(a)_i + idx_i upto shape(a)_i - 1.
 *
 ******************************************************************************/

constant *
COTake (constant *idx, constant *a)
{
    shape *res_shp;
    int i, curr_val_idx;
    int idx_i, shp_i;
    constant *offset, *res;
    constant *new_idx = NULL;

    DBUG_ENTER ("COTake");

    if (CONSTANT_DIM (idx) == 0) {
        new_idx = COcopyScalar2OneElementVector (idx);
        idx = new_idx;
    }
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to COTake not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in COTake!");

    if (CONSTANT_VLEN (idx) > 0) {
        /* 'idx' is a non-empty array */

        DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COTake not int!");

        /*
         * First, we create the result shape:
         *
         * res_shp = |idx| ++ drop( len(idx), shape(a))!
         */
        res_shp = SHcopyShape (CONSTANT_SHAPE (a));
        for (i = 0; i < CONSTANT_VLEN (idx); i++) {
            curr_val_idx = abs (((int *)CONSTANT_ELEMS (idx))[i]);
            res_shp = SHsetExtent (res_shp, i, curr_val_idx);
        }

        /*
         * Now, we create an offset-vector with length len(idx):
         *
         * offset_i = ( idx_i >= 0 ? 0 : shape(a)_i + idx_i )
         */
        offset = COcopyConstant (idx);
        for (i = 0; i < CONSTANT_VLEN (offset); i++) {
            shp_i = SHgetExtent (CONSTANT_SHAPE (a), i);
            idx_i = ((int *)CONSTANT_ELEMS (offset))[i];
            ((int *)CONSTANT_ELEMS (offset))[i] = (idx_i >= 0 ? 0 : shp_i + idx_i);
        }

        /*
         * Finally, we pick the tile of shape 'res_shp' from a starting at position
         * 'offset'
         */
        res = TileFromArray (offset, res_shp, a);

        offset = COfreeConstant (offset);
    } else {
        /* 'idx' is an empty array  ->  res = a */

#if 0
    /*
     * For the time being the TC infers for  '[]'  in  'drop( [], [1,2])'
     * the type  'T_nothing' !!
     */
    DBUG_ASSERT( (CONSTANT_TYPE( idx) == T_int), "idx to CODrop not int!");
#endif

        res = COcopyConstant (a);
    }
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COTake", idx, a, res););

    if (new_idx != NULL) {
        new_idx = COfreeConstant (new_idx);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CODrop( constant *idx, constant *a)
 *
 * description:
 *    picks elements from a! The shape of the result equals
 *           (take( len(idx), shape(a)) - |idx| ) ++ drop( len(idx), shape(a)).
 *    In case of positive idx_i, the elements in axis i are picked from the
 *    "end", i.e., starting at position idx_i upto shape(a)_i - 1.
 *    Otherwise (negative idx_i), the elements in axis i are taken from the
 *    "start", i.e., from 0 upto shape(a)_i + idx_i - 1.
 *
 ******************************************************************************/

constant *
CODrop (constant *idx, constant *a)
{
    shape *res_shp;
    int i, curr_val_idx;
    int idx_i;
    constant *offset;
    constant *res;
    constant *new_idx = NULL;

    DBUG_ENTER ("CODrop");

    if (CONSTANT_DIM (idx) == 0) {
        new_idx = COcopyScalar2OneElementVector (idx);
        idx = new_idx;
    }
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to CODrop not vector!");
    DBUG_ASSERT ((CONSTANT_DIM (a)) >= CONSTANT_VLEN (idx),
                 "idx-vector exceeds dim of array in CODrop!");

    if (CONSTANT_VLEN (idx) > 0) {
        /* 'idx' is a non-empty array */

        DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to CODrop not int!");

        /*
         * First, we create the result shape:
         *
         * res_shp = (take( len(idx), shape(a)) - |idx| ) ++ drop( len(idx), shape(a))!
         */
        res_shp = SHcopyShape (CONSTANT_SHAPE (a));
        for (i = 0; i < CONSTANT_VLEN (idx); i++) {
            curr_val_idx
              = SHgetExtent (res_shp, i) - abs (((int *)CONSTANT_ELEMS (idx))[i]);
            res_shp = SHsetExtent (res_shp, i, curr_val_idx);
        }

        /*
         * Now, we compute the offset of the tile from a to be selected:
         *
         * offset_i = ( idx_i < 0 ? 0 : idx_i )
         */
        offset = COcopyConstant (idx);
        for (i = 0; i < CONSTANT_VLEN (offset); i++) {
            idx_i = ((int *)CONSTANT_ELEMS (offset))[i];
            ((int *)CONSTANT_ELEMS (offset))[i] = (idx_i < 0 ? 0 : idx_i);
        }

        /*
         * Now, we pick the tile of shape 'res_shp' from a starting at position 'offset'
         */
        res = TileFromArray (offset, res_shp, a);
    } else {
        /* 'idx' is an empty array  ->  res = a */

#if 0
    /*
     * For the time being the TC infers for  '[]'  in  'drop( [], [1,2])'
     * the type  'T_nothing' !!
     */
    DBUG_ASSERT( (CONSTANT_TYPE( idx) == T_int), "idx to CODrop not int!");
#endif

        res = COcopyConstant (a);
    }

    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("CODrop", idx, a, res););

    if (new_idx != NULL) {
        new_idx = COfreeConstant (new_idx);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COCat( constant *a, constant *b)
 *
 *   @brief  concatenates a and b wrt the leftmost axis.
 *           Scalars are converted in one element vectors first!
 *
 *   @param a   first array to be concatenated
 *   @param b   second array to be concatenated
 *   @return    the concatenation of a and b
 *
 ******************************************************************************/

constant *
COCat (constant *a, constant *b)
{
    int dim, vlen;
    int i;
    simpletype type;
    shape *shp;
    constant *res, *new_a = NULL, *new_b = NULL;
    void *cv;

    DBUG_ENTER ("COCat");

    if (CONSTANT_DIM (a) == 0) {
        new_a = COcopyScalar2OneElementVector (a);
        a = new_a;
    }
    if (CONSTANT_DIM (b) == 0) {
        new_b = COcopyScalar2OneElementVector (b);
        b = new_b;
    }

    DBUG_ASSERT (CONSTANT_TYPE (a) == CONSTANT_TYPE (b),
                 "COCat applied to arrays of different element type!");
    DBUG_ASSERT (CONSTANT_DIM (a) == CONSTANT_DIM (b),
                 "COCat applied to arrays of different dimensionality!");

    dim = CONSTANT_DIM (a);
    type = CONSTANT_TYPE (a);

    /**
     * First, we compute the resulting shape. Instead of using SHcopyShape,
     * we copy the shape manually in order to allow for consistency checking
     * in the DBUG version.
     */
    shp = SHmakeShape (dim);
    SHsetExtent (shp, 0,
                 SHgetExtent (CONSTANT_SHAPE (a), 0)
                   + SHgetExtent (CONSTANT_SHAPE (b), 0));
    for (i = 1; i < dim; i++) {
        DBUG_ASSERT ((SHgetExtent (CONSTANT_SHAPE (a), i)
                      == SHgetExtent (CONSTANT_SHAPE (b), i)),
                     "COCat applied to arrays with non identical extents in the trailing "
                     "axes!");
        SHsetExtent (shp, i, SHgetExtent (CONSTANT_SHAPE (a), i));
    }

    /**
     * Then, we concatenate the data vectors:
     */
    vlen = CONSTANT_VLEN (a) + CONSTANT_VLEN (b);
    cv = AllocCV (type, vlen);

    CopyElemsFromCVToCV (type, CONSTANT_ELEMS (a), 0, CONSTANT_VLEN (a), cv, 0);
    CopyElemsFromCVToCV (type, CONSTANT_ELEMS (b), 0, CONSTANT_VLEN (b), cv,
                         CONSTANT_VLEN (a));

    /**
     * Finally, we create the result:
     */
    res = MakeConstant (type, shp, cv, vlen);

    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COCat", a, b, res););

    if (new_a != NULL) {
        new_a = COfreeConstant (new_a);
    }
    if (new_b != NULL) {
        new_b = COfreeConstant (new_b);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CODim( constant *a)
 *
 * description:
 *    returns the dimension of a (or 0 if a is a scalar)
 *
 ******************************************************************************/
constant *
CODim (constant *a)
{
    constant *res;

    DBUG_ENTER ("CODim");

    res = COmakeConstantFromInt (CONSTANT_DIM (a));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COShape( constant *a)
 *
 * description:
 *    returns the shape of a (int vector with dim elements).
 *    if called for a scalar constant COShape will return NULL.
 *
 ******************************************************************************/
constant *
COShape (constant *a)
{
    int *shape_vec;
    constant *res;
    int i;

    DBUG_ENTER ("COShape");

    if (CONSTANT_DIM (a) > 0) {
        shape_vec = (int *)ILIBmalloc (CONSTANT_DIM (a) * sizeof (int));
        for (i = 0; i < CONSTANT_DIM (a); i++)
            shape_vec[i] = SHgetExtent (CONSTANT_SHAPE (a), i);
        res = COmakeConstant (T_int, SHcreateShape (1, CONSTANT_DIM (a)), shape_vec);
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COModarray( constant *a, constant *idx, constant *elem)
 *
 * description:
 *    returns modified a with value elem at index idx
 *
 ******************************************************************************/
constant *
COModarray (constant *a, constant *idx, constant *elem)
{
    constant *res;

    DBUG_ENTER ("COModarray");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "idx to COSel not int!");
    DBUG_ASSERT ((CONSTANT_DIM (idx) == 1), "idx to COSel not vector!");
    DBUG_ASSERT ((CONSTANT_TYPE (a) == CONSTANT_TYPE (elem)),
                 "mixed types for array and inserted elements");
    DBUG_ASSERT (((CONSTANT_DIM (a)) == (CONSTANT_VLEN (idx) + CONSTANT_DIM (elem))),
                 "idx-vector exceeds dim of array in COModarray!");

    /* first we create the modified target constant as copy of a */
    res = COcopyConstant (a);

    /* now we copy the modified elements into the target constant vector */
    CopyElemsFromCVToCV (CONSTANT_TYPE (res),                 /* basetype */
                         CONSTANT_ELEMS (elem),               /* from */
                         0,                                   /* offset */
                         SHgetUnrLen (CONSTANT_SHAPE (elem)), /* len */
                         CONSTANT_ELEMS (res),                /* to */
                         Idx2Offset (idx, res));              /* offset */

    DBUG_RETURN (res);
}
