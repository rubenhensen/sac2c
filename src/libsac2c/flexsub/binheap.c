#include "ctinfo.h"
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
#include "globals.h"
#include "graphutils.h"
#include "dynelem.h"
#include "dynarray.h"
#include "binheap.h"
#include "tfprintutils.h"

/*
 * A priority queue is implemented as a dynarray here.
 */

void
PQinsertElem (elem *e, dynarray *q)
{

    DBUG_ENTER ();

    int i, mid;

    addToArray (q, e);
    i = DYNARRAY_TOTALELEMS (q) - 1;

    while (1) {

        mid = i / 2;

        if (ELEM_IDX (DYNARRAY_ELEMS_POS (q, mid)) <= ELEM_IDX (e) || i == mid) {
            break;
        }

        e = DYNARRAY_ELEMS_POS (q, i);
        DYNARRAY_ELEMS_POS (q, i) = DYNARRAY_ELEMS_POS (q, mid);
        DYNARRAY_ELEMS_POS (q, mid) = e;

        i = mid;
    }

    DYNARRAY_ELEMS_POS (q, i) = e;

    DBUG_RETURN ();
}

void
PQinsert (int x, dynarray *q)
{

    DBUG_ENTER ();

    elem *e;

    e = (elem *)MEMmalloc (sizeof (elem));
    ELEM_DATA (e) = NULL;
    ELEM_IDX (e) = x;

    PQinsertElem (e, q);

    DBUG_RETURN ();
}

void
PQdeleteMin (dynarray *q)
{

    DBUG_ENTER ();

    int i, child;
    elem *last;

    DBUG_ASSERT (DYNARRAY_TOTALELEMS (q) > 0, "Priority queue is empty");

    last = DYNARRAY_ELEMS_POS (q, DYNARRAY_TOTALELEMS (q) - 1);

    for (i = 0; i * 2 < DYNARRAY_TOTALELEMS (q) - 2; i = child) {

        child = i * 2 + 1;

        if (ELEM_IDX (DYNARRAY_ELEMS_POS (q, child + 1))
            < ELEM_IDX (DYNARRAY_ELEMS_POS (q, child))) {
            child++;
        }

        if (ELEM_IDX (last) > ELEM_IDX (DYNARRAY_ELEMS_POS (q, child))) {

            if (i == 0) {
                freeElem (DYNARRAY_ELEMS_POS (q, i));
            }

            DYNARRAY_ELEMS_POS (q, i) = DYNARRAY_ELEMS_POS (q, child);

        } else {

            break;
        }
    }

    DYNARRAY_ELEMS_POS (q, i) = last;
    DYNARRAY_ELEMS_POS (q, --DYNARRAY_TOTALELEMS (q)) = NULL;

    DBUG_RETURN ();
}

elem *
PQgetMinElem (dynarray *q)
{

    DBUG_ENTER ();

    DBUG_ASSERT (DYNARRAY_TOTALELEMS (q) > 0, "Priority queue is empty");

    elem *result;
    result = DYNARRAY_ELEMS_POS (q, 0);

    DBUG_RETURN (result);
}

int
PQgetMin (dynarray *q)
{

    DBUG_ENTER ();

    DBUG_ASSERT (DYNARRAY_TOTALELEMS (q) > 0, "Priority queue is empty");

    int result;
    result = ELEM_IDX (PQgetMinElem (q));

    DBUG_RETURN (result);
}

void
PQprint (dynarray *q)
{

    DBUG_ENTER ();

    printDynarray (q);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
