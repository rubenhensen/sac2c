#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "graphtypes.h"
#include "graphutils.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "reachhelper.h"
#include "tfprintutils.h"

void
buildTransitiveLinkTable (dynarray *arrayd)
{

    /*
     * Typical elements of arrayd here are of the form x->[y,z)
     * We store x as the idx of elem and y,z as the associated data.
     *
     * Refer struct ELEM for a better understanding
     */

    int i, j, k, lower, upper, source;
    elem *e;
    matrix *adjmat;

    adjmat = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (adjmat);

    for (i = 0; i < DYNARRAY_TOTALELEMS (arrayd); i++) {

        for (j = 0; j < DYNARRAY_TOTALELEMS (arrayd); j++) {

            for (k = 0; k < DYNARRAY_TOTALELEMS (arrayd); k++) {

                if (ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i))
                      == ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, k))
                    && *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, j)))
                         == *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, k)))) {

                    setMatrixValue (adjmat, i, j, 1);
                }
            }
        }
    }

    /*
     * We have the adjacency matrix now. Update it to get the transitive closure.
     */

    for (i = 0; i < MATRIX_TOTALROWS (adjmat); i++) {

        for (j = 0; j < DYNARRAY_TOTALELEMS (MATRIX_ARRAY2D (adjmat)[i]); j++) {

            if (getMatrixValue (adjmat, i, j) == 1) {

                lower = *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, j)));
                upper = *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, j)) + 1);

                for (k = 0; k < DYNARRAY_TOTALELEMS (arrayd); k++) {

                    source = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, k));

                    if (lower <= source && source < upper) {
                        setMatrixValue (adjmat, i, k, 1);
                    }
                }
            }
        }
    }

    for (i = 0; i < MATRIX_TOTALROWS (adjmat); i++) {

        for (j = 0; j < DYNARRAY_TOTALELEMS (MATRIX_ARRAY2D (adjmat)[i]); j++) {

            if (i != j && getMatrixValue (adjmat, i, j) == 1) {
                e = (elem *)MEMmalloc (sizeof (elem));
                ELEM_IDX (e) = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i));
                ELEM_DATA (e) = MEMrealloc (ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, j)),
                                            2 * sizeof (int), 2 * sizeof (int));
                addToArray (arrayd, e);
            }
        }
    }

    freeMatrix (adjmat);
}

void
setSrcTarArrays (dynarray *arrayd, dynarray **arrX, dynarray **arrY)
{

    int a;
    elem *e;
    dynarray *arraydX, *arraydY;

    arraydX = (dynarray *)MEMmalloc (sizeof (dynarray));
    initDynarray (arraydX);
    arraydY = (dynarray *)MEMmalloc (sizeof (dynarray));
    initDynarray (arraydY);

    for (a = 0; a < DYNARRAY_TOTALELEMS (arrayd); a++) {

        if (!indexExistsInArray (arraydX, ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a)))) {
            e = (elem *)MEMmalloc (sizeof (elem));
            ELEM_IDX (e) = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a));
            addToArray (arraydX, e);
        }

        if (!indexExistsInArray (arraydY,
                                 *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, a))))) {
            e = (elem *)MEMmalloc (sizeof (elem));
            ELEM_IDX (e) = *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, a)));
            ELEM_DATA (e) = MEMmalloc (sizeof (int));
            *((int *)ELEM_DATA (e)) = 0;
            addToArray (arraydY, e);
        }
    }

    sortArray (DYNARRAY_ELEMS (arraydX), 0, DYNARRAY_TOTALELEMS (arraydX) - 1, 0);
    sortArray (DYNARRAY_ELEMS (arraydY), 0, DYNARRAY_TOTALELEMS (arraydY) - 1, 0);

    *arrX = arraydX;
    *arrY = arraydY;
}

matrix *
computeTLCMatrix (dynarray *arrayd, dynarray *arrX, dynarray *arrY)
{

    dynarray *arraydX, *arraydY;
    int xc = 0, index_xc, y, j, k;
    matrix *tlc;
    int a, b;

    tlc = (matrix *)MEMmalloc (sizeof (matrix));
    initMatrix (tlc);

    arraydX = arrX;
    arraydY = arrY;

    sortArray (DYNARRAY_ELEMS (arrayd), 0, DYNARRAY_TOTALELEMS (arrayd) - 1, 1);
    index_xc = DYNARRAY_TOTALELEMS (arraydX) - 1;
    xc = ELEM_IDX (DYNARRAY_ELEMS_POS (arraydX, index_xc));

    for (a = 0; a < DYNARRAY_TOTALELEMS (arrayd); a++) {

        if (ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a)) < xc) {

            for (b = 0; b < DYNARRAY_TOTALELEMS (arraydY); b++) {
                setMatrixValue (tlc, index_xc, b,
                                *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arraydY, b))));
            }

            xc = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a));
            index_xc--;
        }

        j = *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, a)));
        k = *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, a)) + 1);

        for (b = 0; b < DYNARRAY_TOTALELEMS (arraydY); b++) {

            y = ELEM_IDX (DYNARRAY_ELEMS_POS (arraydY, b));

            if (y >= j && y < k) {
                (*((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arraydY, b))))++;
            }
        }
    }

    for (b = 0; b < DYNARRAY_TOTALELEMS (arraydY); b++) {
        setMatrixValue (tlc, index_xc, b,
                        *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arraydY, b))));
    }

    return tlc;
}
