#include <string.h>
#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tf_preprocess_graph.h"
#include "tf_structures.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "globals.h"

/* This file describes a generic array which can grow dynamically as
 * well as a generic stack.
 *
 * These data structure is used to compute the transitive link matrix
 * and the non-tree labels in tf_preprocess_graph.c
 */

void
initElem (elem *e)
{
    ELEM_IDX (e) = 0;
    ELEM_DATA (e) = NULL;
}

void
initDynarray (dynarray *arrayd)
{
    DYNARRAY_ELEMS (arrayd) = NULL;
    DYNARRAY_TOTALELEMS (arrayd) = 0;
    DYNARRAY_ALLOCELEMS (arrayd) = 0;
}

void
initMatrix (matrix *m)
{
    MATRIX_ARRAY2D (m) = NULL;
    MATRIX_TOTALROWS (m) = 0;
    MATRIX_TOTALCOLS (m) = 0;
}

void
initElemstack (elemstack *s)
{
    ELEMSTACK_CURR (s) = NULL;
    ELEMSTACK_NEXT (s) = NULL;
}

void
freeElem (elem *e)
{
    if (e != NULL) {
        if (ELEM_DATA (e) != NULL) {
            MEMfree (ELEM_DATA (e));
            ELEM_DATA (e) = NULL;
        }
        MEMfree (e);
        e = NULL;
    }
}

void
freeElemArray (elem **e, int count)
{
    int i;
    if (e != NULL) {
        for (i = 0; i < count; i++) {
            if (e[i] != NULL) {
                freeElem (e[i]);
                e[i] = NULL;
            }
        }
        MEMfree (e);
        e = NULL;
    }
}

void
freeDynarray (dynarray *arrayd)
{
    if (arrayd != NULL) {
        int i;
        for (i = 0; i < DYNARRAY_ALLOCELEMS (arrayd); i++) {
            if (DYNARRAY_ELEMS (arrayd)[i] != NULL) {
                freeElem (DYNARRAY_ELEMS (arrayd)[i]);
                DYNARRAY_ELEMS (arrayd)[i] = NULL;
            }
        }
        MEMfree (arrayd);
        arrayd = NULL;
    }
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

void *
MEMrealloc (void *src, int allocsize, int oldsize)
{
    void *p = MEMmalloc (allocsize);
    memset (p, 0, allocsize);
    if (src != NULL)
        memcpy (p, src, oldsize);
    return p;
}

void
pushElemstack (elemstack **s, elem *e)
{
    elemstack *top = MEMmalloc (sizeof (elemstack));
    ELEMSTACK_CURR (top) = e;
    ELEMSTACK_NEXT (top) = *s;
    *s = top;
}

elemstack *
popElemstack (elemstack **s)
{
    elemstack *top = NULL;
    if (*s == NULL) {
        CTIabort ("Trying to pop from empty elemstack\n");
    } else {
        top = *s;
        *s = ELEMSTACK_NEXT (top);
    }
    return top;
}

int
addToArray (dynarray *arrayd, elem *item)
{
    int pos, oldsize;
    if (DYNARRAY_TOTALELEMS (arrayd) == DYNARRAY_ALLOCELEMS (arrayd)) {
        oldsize = DYNARRAY_ALLOCELEMS (arrayd);
        DYNARRAY_ALLOCELEMS (arrayd) += 3;
        void *_temp = MEMrealloc (DYNARRAY_ELEMS (arrayd),
                                  (DYNARRAY_ALLOCELEMS (arrayd) * sizeof (elem *)),
                                  oldsize * sizeof (elem *));
        if (!_temp) {
            CTIabort ("addToArray couldn't realloc memory!\n");
        }
        MEMfree (DYNARRAY_ELEMS (arrayd));
        DYNARRAY_ELEMS (arrayd) = (elem **)_temp;
    }
    pos = DYNARRAY_TOTALELEMS (arrayd);
    DYNARRAY_TOTALELEMS (arrayd)++;
    DYNARRAY_ELEMS_POS (arrayd, pos) = item;
    return DYNARRAY_TOTALELEMS (arrayd);
}

int
indexExistsInArray (dynarray *arrayd, int idx)
{
    int i;
    for (i = 0; i < DYNARRAY_TOTALELEMS (arrayd); i++) {
        if (idx == ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i))) {
            return 1;
        }
    }
    return 0;
}

int
addToArrayAtPos (dynarray *arrayd, elem *item, int pos)
{
    if (pos >= DYNARRAY_ALLOCELEMS (arrayd)) {
        int oldsize = DYNARRAY_ALLOCELEMS (arrayd);
        DYNARRAY_ALLOCELEMS (arrayd) = pos + 1;
        void *_temp = MEMrealloc (DYNARRAY_ELEMS (arrayd),
                                  (DYNARRAY_ALLOCELEMS (arrayd) * sizeof (elem *)),
                                  oldsize * sizeof (elem *));
        if (!_temp) {
            CTIabort ("addToArrayAtPos couldn't realloc memory!\n");
        }
        MEMfree (DYNARRAY_ELEMS (arrayd));
        DYNARRAY_ELEMS (arrayd) = (elem **)_temp;
    }
    DYNARRAY_TOTALELEMS (arrayd) = DYNARRAY_ALLOCELEMS (arrayd);
    DYNARRAY_ELEMS_POS (arrayd, pos) = item;
    return DYNARRAY_TOTALELEMS (arrayd);
}

void
merge (elem **elems, int lower, int upper, int desc)
{
    elem **left, **right, **result;
    int mid = (lower + upper) / 2;
    int ll, lr, i, total = 0;
    int cond;
    ll = mid - lower + 1;
    lr = upper - mid;
    left = elems + lower;
    right = elems + mid + 1;
    result = MEMmalloc ((ll + lr) * sizeof (elem *));
    while (ll > 0 && lr > 0) {
        if (ELEM_IDX (left[0]) <= ELEM_IDX (right[0])) {
            if (desc)
                cond = 0;
            else
                cond = 1;
        } else {
            if (desc)
                cond = 1;
            else
                cond = 0;
        }
        if (cond) {
            result[total++] = *left;
            left = left + 1;
            ll--;
        } else {
            result[total++] = *right;
            right = right + 1;
            lr--;
        }
    }
    if (ll > 0) {
        while (ll > 0) {
            result[total++] = *left;
            left = left + 1;
            ll--;
        }
    } else {
        while (lr > 0) {
            result[total++] = *right;
            right = right + 1;
            lr--;
        }
    }
    ll = mid - lower + 1;
    lr = upper - mid;
    left = elems + lower;
    right = elems + mid + 1;
    for (i = 0; i < ll; i++) {
        left[i] = result[i];
    }
    for (i = 0; i < lr; i++) {
        right[i] = result[i + ll];
    }
    // freeElemArray(result,ll+lr);
    MEMfree (result);
}

void
sortArray (elem **elems, int lower, int upper, int desc)
{
    if (elems == NULL) {
        CTIabort ("Typechecker trying to sort DYNARRAY with null elements");
    }
    if (upper - lower > 0) {
        int mid = (upper + lower) / 2;
        sortArray (elems, lower, mid, desc);
        sortArray (elems, mid + 1, upper, desc);
        merge (elems, lower, upper, desc);
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

void
printMatrix (matrix *m)
{
    int i, j;
    dynarray **array2d = MATRIX_ARRAY2D (m);
    dynarray *arrayd;
    printf ("\n");
    for (i = 0; i < MATRIX_TOTALROWS (m); i++) {
        arrayd = array2d[i];
        if (arrayd != NULL) {
            for (j = 0; j < DYNARRAY_TOTALELEMS (arrayd); j++) {
                if (DYNARRAY_ELEMS_POS (arrayd, j) != NULL) {
                    printf ("%d,", ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, j)));
                    fflush (stdout);
                } else {
                    printf ("-,");
                }
            }
            for (j = DYNARRAY_TOTALELEMS (arrayd); j < MATRIX_TOTALCOLS (m); j++) {
                printf ("-,");
            }
        } else {
            for (j = 0; j < MATRIX_TOTALCOLS (m); j++) {
                printf ("-,");
            }
        }
        printf ("\n");
    }
}

void
printMatrixInDotFormat (matrix *m)
{
    int i, j;
    static int id = 0;
    dynarray **array2d = MATRIX_ARRAY2D (m);
    dynarray *arrayd;
    fprintf (global.outfile, "struct%d [label=\"", id++);
    for (i = 0; i < MATRIX_TOTALROWS (m); i++) {
        arrayd = array2d[i];
        if (arrayd != NULL) {
            fprintf (global.outfile, "{");
            for (j = 0; j < DYNARRAY_TOTALELEMS (arrayd); j++) {
                if (DYNARRAY_ELEMS_POS (arrayd, j) != NULL) {
                    fprintf (global.outfile, "%d",
                             ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, j)));
                } else {
                    fprintf (global.outfile, "-");
                }
                if (j != DYNARRAY_TOTALELEMS (arrayd) - 1)
                    printf ("|");
            }
            for (j = DYNARRAY_TOTALELEMS (arrayd); j < MATRIX_TOTALCOLS (m); j++) {
                fprintf (global.outfile, "-");
                if (j != MATRIX_TOTALCOLS (m) - 1)
                    fprintf (global.outfile, "|");
            }
        } else {
            for (j = 0; j < MATRIX_TOTALCOLS (m); j++) {
                fprintf (global.outfile, "-");
                if (j != MATRIX_TOTALCOLS (m) - 1)
                    fprintf (global.outfile, "|");
            }
        }
        fprintf (global.outfile, "}");
        if (i != MATRIX_TOTALROWS (m) - 1)
            fprintf (global.outfile, "|");
    }
    fprintf (global.outfile, "\"];\n");
}

void
printTransitiveLinkTable (dynarray *arrayd)
{
    int i;
    for (i = 0; i < DYNARRAY_TOTALELEMS (arrayd); i++) {
        printf ("%d->[%d,%d)\n", ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i)),
                *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, i))),
                *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, i)) + 1));
    }
}

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
    adjmat = MEMmalloc (sizeof (matrix));
    initMatrix (adjmat);
    // MATRIX_ARRAY2D(adjmat)=MEMmalloc(sizeof(dynarray *));
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
    // printMatrix(adjmat);
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
    // printMatrix(adjmat);
    for (i = 0; i < MATRIX_TOTALROWS (adjmat); i++) {
        for (j = 0; j < DYNARRAY_TOTALELEMS (MATRIX_ARRAY2D (adjmat)[i]); j++) {
            if (i != j && getMatrixValue (adjmat, i, j) == 1) {
                e = MEMmalloc (sizeof (elem));
                ELEM_IDX (e) = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i));
                ELEM_DATA (e) = MEMrealloc (ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, j)),
                                            2 * sizeof (int), 2 * sizeof (int));
                // ELEM_DATA(e)=ELEM_DATA(DYNARRAY_ELEMS_POS(arrayd,j));
                addToArray (arrayd, e);
            }
        }
    }
    freeMatrix (adjmat);
}

void
setXYarrays (dynarray *arrayd, dynarray **arrX, dynarray **arrY)
{
    int a;
    elem *e;
    dynarray *arraydX, *arraydY;
    arraydX = MEMmalloc (sizeof (dynarray));
    initDynarray (arraydX);
    arraydY = MEMmalloc (sizeof (dynarray));
    initDynarray (arraydY);
    for (a = 0; a < DYNARRAY_TOTALELEMS (arrayd); a++) {
        if (!indexExistsInArray (arraydX, ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a)))) {
            e = MEMmalloc (sizeof (elem));
            ELEM_IDX (e) = ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, a));
            addToArray (arraydX, e);
        }
        if (!indexExistsInArray (arraydY,
                                 *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, a))))) {
            e = MEMmalloc (sizeof (elem));
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
    tlc = MEMmalloc (sizeof (matrix));
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
