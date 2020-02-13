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
 * For each dependency graph, we maintain 4 data structures:
 *
 * 1. The euler tour of the graph: This holds information about the order in
 * which the vertices are visited in the euler tour of the DAG. We use the
 * "dynarray" data structure for this purpose. Therein, "elem" structure holds
 * depth of each vertex in the spanning tree as well as the preorder number of
 * the vertex.
 *
 * 2. Intra-block matrices: Each euler tour is sub-divided into intra blocks
 * (refer to the literature on LCA computation in trees). For each intra-block,
 * the intra-block matrices are precomputed for answering LCA queries on
 * vertices contained in the intra-block. The matrix entries in this case are
 * just numbers which enable us to index the euler tour array and obtain the
 * pre-order number of the result.
 *
 * 3. Intra-block minimum array: While we can now answer inta-block queries, we
 * need a mechanism to answer queries where the two vertices do not belong to
 * the same block. This is achieved in a two step process. In the first step, we
 * store the depth and pre-order numbers of the vertex with the lowest depth in
 * any given intra-block.
 *
 * 4. Inter-block matrix: In the second step, we process the inta-block minimum
 * array to pre-compute queries spanning multiple blocks. How this is done
 * exactly can be found in the literature. In particulars, the inquisitive minds
 * should read papers on LCA by Berkman and Vishkin, Michael Bender and Johannes
 * Fischer.
 *
 * In to compute the LCA of two vertices, we need to know the position of their
 * first occurence in the euler tour and then apply range minimum query
 * techniques to get the pre-order number of the LCA.
 */

#include <math.h>
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "TFLUB"
#include "debug.h"

#include "math_utils.h"
#include "memory.h"
#include "tree_compound.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphutils.h"
#include "graphtypes.h"
#include "tfprintutils.h"
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

    DBUG_ENTER ();

    int i, j, prevdepth, currdepth, blockid = 0;

    for (i = 0; i < DYNARRAY_TOTALELEMS (eulertour); i = i + blocksize) {

        prevdepth = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, i));

        for (j = i + 1; j < i + blocksize; j++) {

            if (j < DYNARRAY_TOTALELEMS (eulertour)) {

                currdepth = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, j));

                /*
                 * Since, the adjacent depth values differ by either +1 (if the depth
                 * increases) or -1 (if the depth decreases), a block can be represented
                 * by a bit vector where a 1 denotes an increase in depth and a 0
                 * denotes a decrease in depth. The blockid is just the decimal
                 * representation of this bit vector.
                 */

                if (prevdepth > currdepth) {
                    blockid += MATHipow (2,blocksize - 2 - (j - (i + 1)));
                }

                prevdepth = currdepth;

            } else {

                blockid *= 2;
            }
        }

        for (j = i; j < i + blocksize; j++) {

            if (j < DYNARRAY_TOTALELEMS (eulertour)) {
                ((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (eulertour, j)))[1] = blockid;
            }
        }

        blockid = 0;
    }

    DBUG_RETURN ();
}

matrix *
LUBcomputeIntraTable (dynarray *eulertour, int start, int end)
{

    DBUG_ENTER ();

    DBUG_ASSERT ((start <= end && eulertour != NULL),
                 "Incompatible arguments passed to LUBcomputeIntraTable");

    int i, j, minvalue, minindex, currdepth;
    matrix *result = (matrix *)MEMmalloc (sizeof (matrix));

    for (i = 0; i <= end - start + 1; i++) {

        /*
         * Pick an element from the array
         */

        if (start + i < DYNARRAY_TOTALELEMS (eulertour)) {
            minvalue = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, start + i));
            minindex = start + i;

            for (j = i; j <= end - start; j++) {

                /*
                 * Get the result of RMQ( x, y) where x is the element that was picked
                 * in the previous step and y belongs to the elements in the euler tour
                 * from the picked element to the last element.
                 */

                if (start + j < DYNARRAY_TOTALELEMS (eulertour)) {

                    currdepth = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, start + j));

                    if (minvalue > currdepth) {
                        minvalue = currdepth;
                        minindex = start + j;
                    }
                }

                setMatrixValue (result, i, j, minindex - start);
                setMatrixValue (result, j, i, minindex - start);
            }
        }
    }

    DBUG_RETURN (result);
}

dynarray *
LUBcomputePerBlockMin (dynarray *eulertour, int blocksize)
{

    DBUG_ENTER ();

    DBUG_ASSERT ((blocksize > 0 && eulertour != NULL),
                 "Incompatible arguments passed to LUBcomputePerBlockMin");

    dynarray *result;
    int mindepth, currdepth, minindex, i, j;
    elem *e;

    result = (dynarray *)MEMmalloc (sizeof (dynarray));

    for (i = 0; i < DYNARRAY_TOTALELEMS (eulertour); i = i + blocksize) {

        mindepth = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, i));
        minindex = i;

        for (j = i + 1; j < i + blocksize; j++) {

            if (j < DYNARRAY_TOTALELEMS (eulertour)) {

                currdepth = ELEM_IDX (DYNARRAY_ELEMS_POS (eulertour, j));

                if (mindepth > currdepth) {
                    mindepth = currdepth;
                    minindex = j;
                }
            }
        }

        e = (elem *)MEMmalloc (sizeof (elem));
        ELEM_IDX (e) = mindepth;
        ELEM_DATA (e) = MEMmalloc (sizeof (int));

        *(int *)ELEM_DATA (e) = minindex;

        addToArray (result, e);
    }

    DBUG_RETURN (result);
}

matrix *
LUBprocessBlockMinArray (dynarray *a)
{

    DBUG_ENTER ();

    DBUG_ASSERT ((a != NULL && DYNARRAY_TOTALELEMS (a) > 0),
                 "Incompatible arguments passed to LUBprocessBlockMinArray");

    int i, j, halfstep, fullstep, totalelems;
    matrix *m = (matrix *)MEMmalloc (sizeof (matrix));

    totalelems = DYNARRAY_TOTALELEMS (a);

    if (totalelems == 1) {
        setMatrixValue (m, 0, 0, 0);
        return m;
    }

    for (j = 0; j < ceil (log2 (totalelems)); j++) {

        setMatrixValue (m, totalelems - 1, j, totalelems - 1);
    }

    for (j = 0; j < ceil (log2 (totalelems)); j++) {

        for (i = 0; i < totalelems - 1; i++) {

            if (j == 0) {

                if (ELEM_IDX (DYNARRAY_ELEMS_POS (a, i))
                    < ELEM_IDX (DYNARRAY_ELEMS_POS (a, i + 1))) {

                    setMatrixValue (m, i, 0, i);

                } else {

                    setMatrixValue (m, i, 0, i + 1);
                }

            } else {

                halfstep = getMatrixValue (m, i, j - 1);

                if (i + (1 << (j - 1)) < totalelems) {

                    fullstep = getMatrixValue (m, i + (1 << (j - 1)), j - 1);

                } else {

                    fullstep = getMatrixValue (m, totalelems - 1, j - 1);
                }

                if (ELEM_IDX (DYNARRAY_ELEMS_POS (a, halfstep))
                    < ELEM_IDX (DYNARRAY_ELEMS_POS (a, fullstep))) {

                    setMatrixValue (m, i, j, halfstep);

                } else {

                    setMatrixValue (m, i, j, fullstep);
                }
            }
        }
    }

    DBUG_RETURN (m);
}

int
LUBgetBlockId (dynarray *eulertour, int index)
{

    DBUG_ENTER ();

    elem *e = DYNARRAY_ELEMS_POS (eulertour, index);

    DBUG_RETURN (((int *)ELEM_DATA (e))[1]);
}

lubinfo *
LUBcreatePartitions (dynarray *eulertour)
{

    DBUG_ENTER ();

    int i, j, totalelems, blocksize, oldsize, index;
    lubinfo *lub = (lubinfo *)MEMmalloc (sizeof (lubinfo));

    totalelems = DYNARRAY_TOTALELEMS (eulertour);

    /*
     * We need to ensure that the blocksize is at least 1.
     */

    if (totalelems == 1) {
        blocksize = 1;
    } else {
        blocksize = (int)(log2 (totalelems) / 2.0);
    }

    LUBINFO_BLOCKSIZE (lub) = blocksize;

    DBUG_PRINT ("Size of block for LCA query on spanning tree is %d", blocksize);

    LUBsetBlockIds (eulertour, blocksize);

    for (i = 0; i < totalelems; i += blocksize) {

        oldsize = LUBINFO_NUMINTRA (lub);

        index = LUBgetBlockId (eulertour, i);

        /*
         * check if the index falls within the currently allocated size. If not,
         * then reallocate memory to include the index.
         */

        if (index > oldsize - 1) {

            void *_temp
              = MEMrealloc (LUBINFO_INTRAMATS (lub), (index + 1) * sizeof (matrix *));
            if (!_temp) {
                CTIabort ("LUBcreatePartitions couldn't realloc memory!\n");
            }

            MEMfree (LUBINFO_INTRAMATS (lub));
            LUBINFO_INTRAMATS (lub) = (matrix **)_temp;
            LUBINFO_NUMINTRA (lub) = index + 1;

            for (j = oldsize - 1; j < LUBINFO_NUMINTRA (lub); j++) {
                LUBINFO_INTRAMATS_POS (lub, j) = NULL;
            }
        }

        if (LUBINFO_INTRAMATS_POS (lub, index) == NULL) {
            LUBINFO_INTRAMATS_POS (lub, index)
              = LUBcomputeIntraTable (eulertour, i, i + blocksize - 1);
        }
    }

    LUBINFO_BLOCKMIN (lub) = LUBcomputePerBlockMin (eulertour, blocksize);

    LUBINFO_INTERMAT (lub) = LUBprocessBlockMinArray (LUBINFO_BLOCKMIN (lub));

    DBUG_RETURN (lub);
}

int
LUBgetLowestFromCandidates (dynarray *d, int indices[4])
{

    DBUG_ENTER ();

    int i, result;
    int mindepth;

    mindepth = ELEM_IDX (DYNARRAY_ELEMS_POS (d, indices[0]));
    result = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (d, indices[0]));

    for (i = 1; i < 4; i++) {
        if (mindepth > ELEM_IDX (DYNARRAY_ELEMS_POS (d, indices[i]))) {
            mindepth = ELEM_IDX (DYNARRAY_ELEMS_POS (d, indices[i]));
            result = *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (d, indices[i]));
        }
    }

    DBUG_RETURN (result);
}

node *
LUBtreeLCAfromNodes (node *n1, node *n2, compinfo *ci)
{

    DBUG_ENTER ();

    DBUG_ASSERT ((n1 != NULL && n2 != NULL && ci != NULL),
                 "Incompatible arguments passed to LUBtreeLCAfromNodes");

    node *result;
    int lblockid, lmatrow, lmatcol;
    int ublockid, umatrow, umatcol;
    int lowerid, upperid;
    int blocksize;
    int etindices[4] = {0, 0, 0, 0};
    int base, jump;
    int indexlower, indexupper;
    int resultidx;
    matrix *intermat;
    matrix **intramats;
    dynarray *blockmin;
    elem *e;

    lubinfo *lub = COMPINFO_LUB (ci);

    DBUG_ASSERT (lub != NULL, "The type component graph lacks LCA info");

    intramats = LUBINFO_INTRAMATS (lub);

    DBUG_ASSERT (intramats != NULL, "No intra matrices found");

    blocksize = LUBINFO_BLOCKSIZE (lub);

    DBUG_ASSERT (blocksize > 0, "Blocksize should be a positive integer");

    if (TFVERTEX_EULERID (n1) < TFVERTEX_EULERID (n2)) {
        lowerid = TFVERTEX_EULERID (n1);
        upperid = TFVERTEX_EULERID (n2);
    } else {
        lowerid = TFVERTEX_EULERID (n2);
        upperid = TFVERTEX_EULERID (n1);
    }

    /*
     * Check if the vertices belong to the same intra-block
     */

    lblockid = LUBgetBlockId (COMPINFO_EULERTOUR (ci), lowerid);
    ublockid = LUBgetBlockId (COMPINFO_EULERTOUR (ci), upperid);

    if (upperid / blocksize == lowerid / blocksize) {
        lmatrow = lowerid % blocksize;
        lmatcol = upperid % blocksize;

        indexlower = (lowerid / blocksize) * blocksize
                     + getMatrixValue (intramats[lblockid], lmatrow, lmatcol);

        e = DYNARRAY_ELEMS_POS (COMPINFO_EULERTOUR (ci), indexlower);

        etindices[0] = indexlower;
        etindices[1] = indexlower;
        etindices[2] = indexlower;
        etindices[3] = indexlower;

    } else {
        /*
         * The two vertices do not belong to the same intra-block
         */

        lmatrow = lowerid % blocksize;
        lmatcol = blocksize - 1;

        indexlower = (lowerid / blocksize) * blocksize
                     + getMatrixValue (intramats[lblockid], lmatrow, lmatcol);

        etindices[0] = indexlower;

        umatrow = 0;
        umatcol = upperid % blocksize;

        indexupper = (upperid / blocksize) * blocksize
                     + getMatrixValue (intramats[ublockid], umatrow, umatcol);

        etindices[3] = indexupper;

        intermat = LUBINFO_INTERMAT (lub);
        DBUG_ASSERT (intermat != NULL, "No inter-block query matrix found");

        blockmin = LUBINFO_BLOCKMIN (lub);
        DBUG_ASSERT (blockmin != NULL, "No block minimum array found");

        if (upperid / blocksize > lowerid / blocksize + 1) {
            //FIXME (grzegorz) There is integer log function so have to cast for now
            jump = (int)+floor (log2 (upperid / blocksize - lowerid / blocksize - 2));

            base = lowerid / blocksize + 1;
            e = DYNARRAY_ELEMS_POS (blockmin, getMatrixValue (intermat, base, jump));
            etindices[1] = *(int *)ELEM_DATA (e);

            base = upperid / blocksize - 1 - MATHipow (2, jump);
            e = DYNARRAY_ELEMS_POS (blockmin, getMatrixValue (intermat, base, jump));
            etindices[2] = *(int *)ELEM_DATA (e);

        } else {

            etindices[1] = etindices[0];
            etindices[2] = etindices[0];
        }
    }

    resultidx = LUBgetLowestFromCandidates (COMPINFO_EULERTOUR (ci), etindices);

    e = DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), resultidx - 1);

    result = (node *)ELEM_DATA (e);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
