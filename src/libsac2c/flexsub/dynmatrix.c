#include "ctinfo.h"
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
#include "globals.h"
#include "graphutils.h"
#include "dynelem.h"
#include "dynarray.h"
#include "dynmatrix.h"

/* This file describes a generic array which can grow dynamically as
 * well as a generic stack.
 *
 * These data structure is used to compute the transitive link matrix
 * and the non-tree labels in preprocess_graph.c
 */

void
initMatrix (matrix *m)
{

    MATRIX_ARRAY2D (m) = NULL;
    MATRIX_TOTALROWS (m) = 0;
    MATRIX_TOTALCOLS (m) = 0;
}

void
free2DArray (dynarray **d2, int count)
{

    if (d2 != NULL) {

        int i;

        for (i = 0; i < count; i++) {

            if (d2[i] != NULL) {

                freeDynarray (d2[i]);
                d2[i] = NULL;
            }
        }

        MEMfree (d2);
        d2 = NULL;
    }
}

void
freeMatrix (matrix *m)
{

    if (m != NULL) {

        if (MATRIX_ARRAY2D (m) != NULL) {

            free2DArray (MATRIX_ARRAY2D (m), MATRIX_TOTALROWS (m));
            MATRIX_ARRAY2D (m) = NULL;
        }

        MEMfree (m);
        m = NULL;
    }
}

void
setMatrixValue (matrix *m, int x, int y, int value)
{

    int i, oldlength;

    elem *element = MEMmalloc (sizeof (elem));
    ELEM_IDX (element) = value;
    ELEM_DATA (element) = NULL;
    oldlength = MATRIX_TOTALROWS (m);

    if (MATRIX_TOTALCOLS (m) < y + 1) {
        MATRIX_TOTALCOLS (m) = y + 1;
    }

    if (MATRIX_TOTALROWS (m) < x + 1) {
        MATRIX_TOTALROWS (m) = x + 1;

        void *_temp
          = MEMrealloc (MATRIX_ARRAY2D (m), (MATRIX_TOTALROWS (m) * sizeof (dynarray *)),
                        oldlength * sizeof (dynarray *));

        if (!_temp) {
            CTIabort ("setMatrixValue couldn't realloc memory!\n");
        }

        MEMfree (MATRIX_ARRAY2D (m));
        MATRIX_ARRAY2D (m) = (dynarray **)_temp;
    }

    for (i = oldlength; i < MATRIX_TOTALROWS (m); i++) {
        MATRIX_ARRAY2D (m)[i] = NULL;
    }

    if (MATRIX_ARRAY2D (m)[x] == NULL) {
        MATRIX_ARRAY2D (m)[x] = MEMmalloc (sizeof (dynarray));
        initDynarray (MATRIX_ARRAY2D (m)[x]);
    }

    addToArrayAtPos (MATRIX_ARRAY2D (m)[x], element, y);
}

int
getMatrixValue (matrix *m, int x, int y)
{

    dynarray *arrayd = MATRIX_ARRAY2D (m)[x];
    elem *e = DYNARRAY_ELEMS (arrayd)[y];
    if (e != NULL)
        return ELEM_IDX (e);
    else
        return -1;
}
