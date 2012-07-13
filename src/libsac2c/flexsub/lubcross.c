/** <!--********************************************************************-->
 *
 * @file lubcross.c
 *
 * prefix: LUB
 *
 * description:
 */

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
#include "lubcross.h"
#include "binheap.h"
#include "query.h"

typedef struct POSTINFO {
    int iscsrc;
    int colidx;
    node *vertex;
} postinfo;

#define POSTINFO_ISCSRC(n) ((n)->iscsrc)
#define POSTINFO_COLIDX(n) ((n)->colidx)
#define POSTINFO_VERTEX(n) ((n)->vertex)

typedef struct TOPOINFO {
    int colidx;
    node *vertex;
} topoinfo;

#define TOPOINFO_COLIDX(n) ((n)->colidx)
#define TOPOINFO_VERTEX(n) ((n)->vertex)

typedef struct PCPCINFO {
    dynarray *csrc;
    matrix *csrcmat;
    dynarray *noncsrc;
    matrix *noncsrcmat;
} pcpcinfo;

#define PCPCINFO_CSRC(n) ((n)->csrc)
#define PCPCINFO_CSRCMAT(n) ((n)->csrcmat)
#define PCPCINFO_NONCSRC(n) ((n)->noncsrc)
#define PCPCINFO_NONCSRCMAT(n) ((n)->noncsrcmat)

matrix *
LUBcreateReachMat (compinfo *ci)
{

    DBUG_ENTER ();

    dynarray *csrc, *ctar, *prearr;
    int i, j;
    matrix *result;
    node *srcvert, *tarvert;
    elem *e;

    result = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (result);

    csrc = COMPINFO_CSRC (ci);
    ctar = COMPINFO_CTAR (ci);
    prearr = COMPINFO_PREARR (ci);

    for (i = 0; i < DYNARRAY_TOTALELEMS (csrc); i++) {

        e = DYNARRAY_ELEMS_POS (prearr, ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, i)) - 1);
        srcvert = (node *)ELEM_DATA (e);

        for (j = 0; j < DYNARRAY_TOTALELEMS (ctar); j++) {

            e = DYNARRAY_ELEMS_POS (prearr, ELEM_IDX (DYNARRAY_ELEMS_POS (ctar, j)) - 1);
            tarvert = (node *)ELEM_DATA (e);

            if (GINisReachable (srcvert, tarvert, ci)) {
                setMatrixValue (result, j, i, 1);
            } else {
                setMatrixValue (result, j, i, 0);
            }
        }
    }

    DBUG_RETURN (result);
}

matrix *
LUBcreatePCPTMat (matrix *reachmat, compinfo *ci)
{

    DBUG_ENTER ();

    matrix *pcptmat;
    elemstack *stk;
    elem *e;
    dynarray *csrc, *ctar;

    csrc = COMPINFO_CSRC (ci);
    ctar = COMPINFO_CTAR (ci);

    int i, j;
    int prev_lower = -1;

    stk = (elemstack *)MEMmalloc (sizeof (elemstack));
    initElemstack (stk);

    pcptmat = (matrix *)MEMmalloc (sizeof (matrix));

    for (i = 0; i < DYNARRAY_TOTALELEMS (ctar); i++) {

        for (j = 0; j < DYNARRAY_TOTALELEMS (csrc); j++) {

            if (getMatrixValue (reachmat, i, j) == 1) {

                while (!isElemstackEmpty (stk)) {

                    e = popElemstack (&stk);
                    /*
                     * Store the preorder number of a cross edge source reaching a
                     * particular cross edge target here. The property of this cross edge
                     * source is that its pre-order number should be less than or equal to
                     * the current cross edge source under examination (indexed by j).
                     */

                    ((int *)ELEM_DATA (e))[1] = ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, j));
                    setMatrixElem (pcptmat, i, ELEM_IDX (e), e);
                }

                prev_lower = ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, j));
            }

            e = (elem *)MEMmalloc (sizeof (elem));
            ELEM_IDX (e) = j;
            ELEM_DATA (e) = MEMmalloc (2 * sizeof (int));
            ((int *)ELEM_DATA (e))[0] = prev_lower;

            pushElemstack (&stk, e);
        }

        /*
         * We have examined all cross edge sources now for a given cross edge
         * target. It may so happen that for the last few cross edge sources, there
         * may not be any cross edge source which has a higher preorder number and
         * reached that cross edge target currently under consideration!
         *
         * Pop rest of the elements off the stack now. Its time to store a null (
         * signified by -1) vertex preoder number as the cross edge source reaching
         * the cross edge target currently under consideration (indexed by i).
         */

        while (!isElemstackEmpty (stk)) {

            e = popElemstack (&stk);
            ((int *)ELEM_DATA (e))[1] = -1;
            setMatrixElem (pcptmat, i, ELEM_IDX (e), e);
        }
    }

    DBUG_RETURN (pcptmat);
}

/** <!--********************************************************************-->
 *
 * @fn dynarray * LUBsortInPostorder( compinfo *ci)
 *
 *   @brief
 *   This function uses the list of vertices in preorder sequence and the
 *   list of cross edge sources as inputs. It then sorts the cross edge
 *   sources in post order sequence, stores this information in a new dynamic
 *   array and returns the array. In this array, we additionally store the
 *   preorder numbers of the cross edge sources.
 *
 *   @param ci
 *
 *   @return result
 *
 *****************************************************************************/
dynarray *
LUBsortInPostorder (compinfo *ci)
{

    DBUG_ENTER ();

    dynarray *result, *prearr, *csrc;
    int i;
    elem *e;
    postinfo *data;
    node *vertex;
    int prenum;

    prearr = COMPINFO_PREARR (ci);
    csrc = COMPINFO_CSRC (ci);

    DBUG_ASSERT ((prearr != NULL && csrc != NULL),
                 "Incompatible arguments passed to LUBsortInPostorder");

    result = (dynarray *)MEMmalloc (sizeof (dynarray));
    initDynarray (result);

    for (i = 0; i < DYNARRAY_TOTALELEMS (csrc); i++) {

        e = (elem *)MEMmalloc (sizeof (elem));

        prenum = ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, i));
        vertex = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (prearr, prenum - 1));

        ELEM_IDX (e) = TFVERTEX_POST (vertex);
        ELEM_DATA (e) = MEMmalloc (sizeof (postinfo));

        data = (postinfo *)(ELEM_DATA (e));

        POSTINFO_ISCSRC (data) = 1;
        POSTINFO_COLIDX (data) = i;
        POSTINFO_VERTEX (data) = vertex;

        addToArray (result, e);
    }

    sortArray (DYNARRAY_ELEMS (result), 0, DYNARRAY_TOTALELEMS (result) - 1, 0);

    DBUG_RETURN (result);
}

void
LUBorColumnsAndUpdate (matrix *m1, int colidx1, matrix *m2, int colidx2, matrix *result,
                       int rescolidx)
{
    DBUG_ENTER ();

    DBUG_ASSERT (MATRIX_TOTALROWS (m1) == MATRIX_TOTALROWS (m2),
                 "The two matrices in LUBorColumnsAndAppend do \
	       not have the same row count");

    DBUG_ASSERT (result != NULL, "Result matrix cannot be empty");

    int i, value;

    for (i = 0; i < MATRIX_TOTALROWS (m1); i++) {

        /*
         * At the moment we should refrain from using a bitwise OR operation below
         * because the matrix cells are by default initialized to -1.
         */

        if (getMatrixValue (m1, i, colidx1) == 1
            || getMatrixValue (m2, i, colidx2) == 1) {
            value = 1;
        } else {
            value = 0;
        }

        setMatrixValue (result, i, rescolidx, value);
    }

    DBUG_RETURN ();
}

int
LUBisNodeCsrc (node *n, dynarray *csrc)
{

    DBUG_ENTER ();

    int i, result = 0;

    for (i = 0; i < DYNARRAY_TOTALELEMS (csrc); i++) {

        if (TFVERTEX_PRE (n) == ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, i))) {
            result = 1;
            break;
        }
    }

    DBUG_RETURN (result);
}

dynarray *
LUBrearrangeCsrcOnTopo (dynarray *csrc, dynarray *prearr)
{

    DBUG_ENTER ();

    dynarray *result;
    node *vertex;
    int i;
    elem *e, *currpre, *currcsrc;

    result = (dynarray *)MEMmalloc (sizeof (dynarray));
    initDynarray (result);

    for (i = 0; i < DYNARRAY_TOTALELEMS (csrc); i++) {

        currcsrc = DYNARRAY_ELEMS_POS (csrc, i);
        currpre = DYNARRAY_ELEMS_POS (prearr, ELEM_IDX (currcsrc) - 1);
        vertex = (node *)ELEM_DATA (currpre);

        e = (elem *)MEMmalloc (sizeof (elem));
        ELEM_IDX (e) = TFVERTEX_TOPO (vertex);
        ELEM_DATA (e) = MEMmalloc (sizeof (topoinfo));

        TOPOINFO_COLIDX ((topoinfo *)ELEM_DATA (e)) = i;
        TOPOINFO_VERTEX ((topoinfo *)ELEM_DATA (e)) = vertex;

        addToArray (result, e);
    }

    sortArray (DYNARRAY_ELEMS (result), 0, DYNARRAY_TOTALELEMS (result) - 1, 0);

    DBUG_RETURN (result);
}

dynarray *
LUBrearrangeNoncsrcOnTopo (dynarray *noncsrc)
{

    DBUG_ENTER ();

    dynarray *result;
    int i;
    elem *e1, *e2;
    node *vertex;

    result = (dynarray *)MEMmalloc (sizeof (dynarray));
    initDynarray (result);

    for (i = 0; i < DYNARRAY_TOTALELEMS (noncsrc); i++) {

        e1 = DYNARRAY_ELEMS_POS (noncsrc, i);
        vertex = POSTINFO_VERTEX ((postinfo *)ELEM_DATA (e1));

        e2 = (elem *)MEMmalloc (sizeof (elem));
        ELEM_IDX (e2) = TFVERTEX_TOPO (vertex);
        ELEM_DATA (e2) = MEMmalloc (sizeof (topoinfo));

        TOPOINFO_COLIDX ((topoinfo *)ELEM_DATA (e2))
          = POSTINFO_COLIDX ((postinfo *)ELEM_DATA (e1));
        TOPOINFO_VERTEX ((topoinfo *)ELEM_DATA (e2)) = vertex;

        addToArray (result, e2);
    }

    sortArray (DYNARRAY_ELEMS (result), 0, DYNARRAY_TOTALELEMS (result) - 1, 0);

    DBUG_RETURN (result);
}

matrix *
LUBcomputeMaximalWitness (pcpcinfo *ppi)
{

    DBUG_ENTER ();

    matrix *result;
    matrix *csrcmax, *noncsrcmax;

    dynarray *csrc, *noncsrc;
    matrix *csrcmat, *noncsrcmat;
    int i, j, k, max = 0;
    node *vertex_csrc, *vertex_noncsrc;

    csrc = PCPCINFO_CSRC (ppi);
    csrcmat = PCPCINFO_CSRCMAT (ppi);
    csrcmax = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (csrcmax);

    for (i = 0; i < MATRIX_TOTALROWS (csrcmat); i++) {

        for (j = 0; j < MATRIX_TOTALROWS (csrcmat); j++) {

            for (k = 0; k < MATRIX_TOTALCOLS (csrcmat); k++) {

                if (getMatrixValue (csrcmat, i, k) && getMatrixValue (csrcmat, j, k)) {
                    max = k;
                }
            }

            setMatrixValue (csrcmax, i, j, max);
            max = 0;
        }
    }

    noncsrc = PCPCINFO_NONCSRC (ppi);
    noncsrcmat = PCPCINFO_NONCSRCMAT (ppi);
    noncsrcmax = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (noncsrcmax);

    for (i = 0; i < MATRIX_TOTALROWS (noncsrcmat); i++) {

        for (j = 0; j < MATRIX_TOTALROWS (noncsrcmat); j++) {

            for (k = 0; k < MATRIX_TOTALCOLS (noncsrcmat); k++) {

                if (getMatrixValue (noncsrcmat, i, k)
                    && getMatrixValue (noncsrcmat, j, k)) {
                    max = k;
                }
            }

            setMatrixValue (noncsrcmax, i, j, max);
            max = 0;
        }
    }

    /*
     * We have two matrices now. Each cell in each matrix contains an index to the
     * csrc and noncsrc arrays. These arrays hold vertices sorted in topological
     * sequence. Now we compare the two matrices cell-wise and store the pre-order
     * number of the vertex which has a higher topological number between the two
     * cell entries.
     */

    DBUG_ASSERT ((MATRIX_TOTALROWS (csrcmax) == MATRIX_TOTALROWS (noncsrcmax)
                  && MATRIX_TOTALCOLS (csrcmax) == MATRIX_TOTALCOLS (noncsrcmax)),
                 "Matrix shape mismatch while bulding PC-PC matrix.");

    result = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (result);

    for (i = 0; i < MATRIX_TOTALROWS (csrcmax); i++) {

        for (j = 0; j < MATRIX_TOTALCOLS (csrcmax); j++) {

            vertex_csrc = TOPOINFO_VERTEX ((topoinfo *)ELEM_DATA (
              DYNARRAY_ELEMS_POS (csrc, getMatrixValue (csrcmax, i, j))));

            vertex_noncsrc = TOPOINFO_VERTEX ((topoinfo *)ELEM_DATA (
              DYNARRAY_ELEMS_POS (noncsrc, getMatrixValue (noncsrcmax, i, j))));

            if (TFVERTEX_TOPO (vertex_csrc) > TFVERTEX_TOPO (vertex_noncsrc)) {
                setMatrixValue (result, i, j, TFVERTEX_PRE (vertex_csrc));
            } else {
                setMatrixValue (result, i, j, TFVERTEX_PRE (vertex_noncsrc));
            }
        }
    }

    freeMatrix (csrcmax);
    freeMatrix (noncsrcmax);

    DBUG_RETURN (result);
}

matrix *
LUBrearrangeMatOnTopo (dynarray *topoarr, matrix *mat)
{

    DBUG_ENTER ();

    matrix *result;
    topoinfo *ti;
    int i, j, value;

    result = (matrix *)MEMmalloc (sizeof (matrix));

    for (i = 0; i < DYNARRAY_TOTALELEMS (topoarr); i++) {

        ti = (topoinfo *)ELEM_DATA (DYNARRAY_ELEMS_POS (topoarr, i));

        for (j = 0; j < MATRIX_TOTALROWS (mat); j++) {
            value = getMatrixValue (mat, j, TOPOINFO_COLIDX (ti));
            setMatrixValue (result, j, i, value);
        }
    }

    DBUG_RETURN (result);
}

matrix *
LUBcreatePCPCMat (matrix *reachmat, dynarray *postarr, compinfo *ci)
{

    DBUG_ENTER ();

    node *n1 = NULL, *n2, *treelca;
    matrix *result = NULL;
    matrix *currmat = NULL, *mat1, *mat2;
    postinfo *pi1, *pi2, *pi;
    dynarray *noncsrc = NULL, *q = postarr;
    elem *e;
    int colidx = 0, colidx_pi1;
    ;
    pcpcinfo *ppi = NULL;

    while (DYNARRAY_TOTALELEMS (q) > 0) {

        pi1 = (postinfo *)ELEM_DATA (PQgetMinElem (q));
        n1 = POSTINFO_VERTEX (pi1);
        colidx_pi1 = POSTINFO_COLIDX (pi1);
        PQdeleteMin (q);

        if (DYNARRAY_TOTALELEMS (q) == 0) {
            break;
        } else {
            pi2 = (postinfo *)ELEM_DATA (PQgetMinElem (q));
            n2 = POSTINFO_VERTEX (pi2);
            treelca = LUBtreeLCAfromNodes (n1, n2, ci);
        }

        if (!LUBisNodeCsrc (treelca, COMPINFO_CSRC (ci))) {

            /*
             * Now, we update the reachable clusterheads for n1. Enqueue the newfound
             * node in q here. Also add the newfound node to the array that hold
             * pcpc-plcas that are not cross edge sources.
             */
            if (noncsrc == NULL) {

                noncsrc = (dynarray *)MEMmalloc (sizeof (dynarray));
                initDynarray (noncsrc);
                currmat = (matrix *)MEMmalloc (sizeof (matrix));
                initMatrix (currmat);
            }

            /*
             * Before inserting this vertex, we need to investigate whether it has
             * already been discovered.
             *
             * TODO: The function indexExistsInArray(..) has linear complexity. We can
             * write a function with a logarithmic complexity because q is a sorted
             * array.
             */

            if (!indexExistsInArray (q, TFVERTEX_POST (treelca))) {

                e = (elem *)MEMmalloc (sizeof (elem));
                ELEM_IDX (e) = TFVERTEX_POST (treelca);
                pi = (postinfo *)MEMmalloc (sizeof (postinfo));
                POSTINFO_ISCSRC (pi) = 0;
                POSTINFO_COLIDX (pi) = colidx++;
                POSTINFO_VERTEX (pi) = treelca;
                ELEM_DATA (e) = pi;

                addToArray (noncsrc, e);
                PQinsertElem (e, q);

                if (LUBisNodeCsrc (n1, COMPINFO_CSRC (ci))) {
                    mat1 = reachmat;
                } else {
                    mat1 = currmat;
                }

                if (LUBisNodeCsrc (n2, COMPINFO_CSRC (ci))) {
                    mat2 = reachmat;
                } else {
                    mat2 = currmat;
                }

                LUBorColumnsAndUpdate (mat1, colidx_pi1, mat2, POSTINFO_COLIDX (pi2),
                                       currmat, colidx - 1);
            }
        }
    }

    if (noncsrc != NULL) {
        // printDynarray( noncsrc);
        // printMatrix( currmat);
        ppi = (pcpcinfo *)MEMmalloc (sizeof (pcpcinfo));

        PCPCINFO_CSRC (ppi)
          = LUBrearrangeCsrcOnTopo (COMPINFO_CSRC (ci), COMPINFO_PREARR (ci));
        PCPCINFO_CSRCMAT (ppi) = LUBrearrangeMatOnTopo (PCPCINFO_CSRC (ppi), reachmat);

        PCPCINFO_NONCSRC (ppi) = LUBrearrangeNoncsrcOnTopo (noncsrc);
        PCPCINFO_NONCSRCMAT (ppi)
          = LUBrearrangeMatOnTopo (PCPCINFO_NONCSRC (ppi), currmat);

        result = LUBcomputeMaximalWitness (ppi);
    }

    DBUG_RETURN (result);
}

void
LUBincorporateCrossEdges (compinfo *ci)
{

    DBUG_ENTER ();

    matrix *reachmat;
    dynarray *postarr;

    if (COMPINFO_CSRC (ci) != NULL) {

        reachmat = LUBcreateReachMat (ci);
        postarr = LUBsortInPostorder (ci);

        LUBINFO_PCPTMAT (COMPINFO_LUB (ci)) = LUBcreatePCPTMat (reachmat, ci);
        LUBINFO_PCPCMAT (COMPINFO_LUB (ci)) = LUBcreatePCPCMat (reachmat, postarr, ci);
        printLubInfo (ci);
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
