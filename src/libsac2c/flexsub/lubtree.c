/** <!--********************************************************************-->
 *
 * @file lubtree.c
 *
 * prefix: LUB
 *
 * description: This file contains functions to compute Least Upper Bound (LUB)
 * a.k.a Lowest Common Ancestor (LCA) of two types in the spanning tree of the
 * type dependency graph.
 *
 *****************************************************************************/

#include <math.h>
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "lubtree.h"

/** <!--********************************************************************-->
 *
 * @fn
 *
 *   @brief
 *
 *   @param
 *   @return
 *
 *****************************************************************************/

void
LUBsetBlockIds (dynarray *eulertour, int blocksize)
{

    DBUG_ENTER ("LUBsetBlockId");

    int i, j, prevdepth, currdepth, blockid = 0;

    for (i = 0; i < DYNARRAY_TOTALELEMS (eulertour); i = i + blocksize) {

        prevdepth = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, i));

        for (j = i + 1; j < i + blocksize; j++) {

            if (j < DYNARRAY_TOTALELEMS (eulertour)) {

                currdepth = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, j));

                /*
                 * Since, the adjacent depth values differ by either +1 (if the depth
                 * increases) or -1 (if the depth decreases), a block can be represented
                 * by a bit vector where a 1 denotes an increase in depth and a 0
                 * denotes a decrease in depth. The blockid is just the decimal
                 * representation of this bit vector.
                 */

                if (prevdepth > currdepth) {
                    blockid += pow (2, (blocksize - 2 - (j - (i + 1))));
                }

                prevdepth = currdepth;

            } else {

                blockid *= 2;
            }
        }

        for (j = i; j < i + blocksize; j++) {

            if (j < DYNARRAY_TOTALELEMS (eulertour)) {
                ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, j)) = blockid;
            }
        }

        blockid = 0;
    }

    DBUG_VOID_RETURN;
}

matrix *
LUBcomputeIntraTable (dynarray *eulertour, int start, int end)
{

    DBUG_ENTER ("LUBcomputeIntraTable");

    if (start > end || eulertour == NULL) {
        CTIabort ("Incompatible arguments passed to LUBcomputeIntraTable");
    }

    int i, j, minvalue, minindex, currdepth;
    matrix *result = MEMmalloc (sizeof (matrix));

    for (i = start; i <= end; i++) {

        /*
         * Pick an element from the array
         */

        minvalue = 0;
        minindex = i;

        for (j = i; j <= end; j++) {

            /*
             * Get the result of RMQ( x, y) where x is the element that was picked in
             * the previous step and y belongs to the elements in the euler tour from
             * the picked element to the last element.
             */

            currdepth = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, j));

            if (minvalue > currdepth) {
                minvalue = currdepth;
                minindex = j;
            }

            setMatrixValue (result, i, j, minindex);
            setMatrixValue (result, j, i, minindex);
        }
    }

    DBUG_RETURN (result);
}

dynarray *
LUBcomputePerBlockMin (dynarray *eulertour, int blocksize)
{

    DBUG_ENTER ("LUBcomputePerBlockMin");

    if (blocksize <= 0 || eulertour == NULL) {
        CTIabort ("Incompatible arguments passed to LUBcomputePerBlockMin");
    }

    dynarray *result;
    int mindepth, currdepth, minindex, i, j;
    elem *e;

    result = MEMmalloc (sizeof (dynarray));

    for (i = 0; i < DYNARRAY_TOTALELEMS (eulertour); i = i + blocksize) {

        mindepth = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, i));
        minindex = i;

        for (j = i + 1; j < i + blocksize; j++) {

            currdepth = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, j));

            if (mindepth < currdepth) {
                mindepth = currdepth;
                minindex = j;
            }
        }

        e = MEMmalloc (sizeof (elem));
        ELEM_IDX (e) = minindex;

        addToArray (result, e);
    }

    DBUG_RETURN (result);
}

matrix *
LUBprocessBlockMinArray (dynarray *a)
{

    DBUG_ENTER ("LUBprocessBlockMinArray");

    DBUG_ASSERT ((a == NULL), "Incompatible arguments passed to LUBprocessBlockMinArray");

    DBUG_ASSERT (DYNARRAY_TOTALELEMS (a) < 2, "Array should have more than one element");

    int i, j;
    matrix *m;

    for (i = 0; i < DYNARRAY_TOTALELEMS (a); i++) {

        for (j = 0; j < ceil (log2 (DYNARRAY_TOTALELEMS (a))); j++) {

            if (j == 0) {

                if (ELEM_IDX (DYNARRAY_ELEMS_POS (a, i))
                    < ELEM_IDX (DYNARRAY_ELEMS_POS (a, i + 1))) {

                    setMatrixValue (m, i, 0, i);

                } else {

                    setMatrixValue (m, i, 0, i + 1);
                }

            } else {

                if (ELEM_IDX (DYNARRAY_ELEMS_POS (a, getMatrixValue (m, i, j - 1)))
                    < ELEM_IDX (
                        DYNARRAY_ELEMS_POS (a, getMatrixValue (m, i + (int)pow (2, j - 1),
                                                               j - 1)))) {

                    setMatrixValue (m, i, j, getMatrixValue (m, i, j - 1));

                } else {

                    setMatrixValue (m, i, j,
                                    getMatrixValue (m, i + (int)pow (2, j - 1), j - 1));
                }
            }
        }
    }

    DBUG_RETURN (m);
}

matrix **
LUBcreatePartitions (dynarray *eulertour)
{

    DBUG_ENTER ("LUBcreatePartitions");

    int i, totalelems, blocksize;
    matrix **rmqmatrices = NULL;

    totalelems = DYNARRAY_TOTALELEMS (eulertour);
    blocksize = log2 (totalelems) / 2.0;

    DBUG_PRINT ("TFLUB",
                ("Size of block for LCA query on spanning tree is %d", blocksize));

    if (blocksize != 0)
        LUBsetBlockIds (eulertour, blocksize);

    for (i = 0; i < totalelems; i += blocksize) {
        LUBcomputeIntraTable (eulertour, i, i + blocksize - 1);
    }

    DBUG_RETURN (rmqmatrices);
}
