#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "tree_compound.h"
#include "globals.h"
#include "graphutils.h"
#include "dynelem.h"
#include "dynarray.h"

void
initDynarray (dynarray *arrayd)
{

    DYNARRAY_ELEMS (arrayd) = NULL;
    DYNARRAY_TOTALELEMS (arrayd) = 0;
    DYNARRAY_ALLOCELEMS (arrayd) = 0;
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

int
addToArray (dynarray *arrayd, elem *item)
{

    int pos;

    if (DYNARRAY_TOTALELEMS (arrayd) == DYNARRAY_ALLOCELEMS (arrayd)) {

        DYNARRAY_ALLOCELEMS (arrayd) += 3;

        void *_temp = MEMrealloc (DYNARRAY_ELEMS (arrayd),
                                  DYNARRAY_ALLOCELEMS (arrayd) * sizeof (elem *));
        if (!_temp) {
            CTIabort (EMPTY_LOC, "addToArray couldn't realloc memory!\n");
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

        DYNARRAY_ALLOCELEMS (arrayd) = pos + 1;

        void *_temp = MEMrealloc (DYNARRAY_ELEMS (arrayd),
                                  DYNARRAY_ALLOCELEMS (arrayd) * sizeof (elem *));
        if (!_temp) {
            CTIabort (EMPTY_LOC, "addToArrayAtPos couldn't realloc memory!\n");
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
    result = (elem **)MEMmalloc ((ll + lr) * sizeof (elem *));

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
            left++;
            ll--;

        } else {

            result[total++] = *right;
            right++;
            lr--;
        }
    }

    if (ll > 0) {

        while (ll > 0) {
            result[total++] = *left;
            left++;
            ll--;
        }

    } else {

        while (lr > 0) {
            result[total++] = *right;
            right++;
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

    MEMfree (result);
}

void
sortArray (elem **elems, int lower, int upper, int desc)
{

    if (elems == NULL) {
        CTIabort (EMPTY_LOC, "Typechecker trying to sort DYNARRAY with null elements");
    }

    if (upper - lower > 0) {

        int mid = (upper + lower) / 2;
        sortArray (elems, lower, mid, desc);
        sortArray (elems, mid + 1, upper, desc);
        merge (elems, lower, upper, desc);
    }
}
