/*
 *
 * $Log$
 * Revision 1.1  2005/07/25 16:43:06  sbs
 * Initial revision
 *
 *
 */

/**
 *
 * @file
 *
 * This file provides a simple heap facility.
 * Whereas most of the heap managament in the compiler should be done
 * using ILIBmalloc and ILIBfree, these sometimes do not suffice.
 *
 * The problem that sometimes occurs is that the nature of the algorithm
 * does not allow to precisely determine when an individual data structure
 * can be freed. However, there is usually a stage in the compilation
 * process where ALL data structures of a certain kind can be freed.
 *
 * Examples for this kind of situation are: type variables (ssi.c),
 * signature dependencies( sig_deps.c) or namespaces (namespaces.c).
 *
 * The idea of this module is to provide some general support for these
 * situations. Essentially, on creates an abstract heap by calling
 * PHPcreateHeap. This heap can only be used for data structes of ONE
 * kind. All allocations are made by calling PHPmalloc.
 * When NONE of the data structures is needed anymore, ALL can be
 * freed in one sweep by calling PHPfreeHeap.
 *
 * In order to provide a searching facility in the heap, we also
 * have a function PHPfindElem. It goes through all elements in
 * a given heap and applies a given comparison function (and one
 * reference element) to them. It returns the pointer to the first
 * matching element or - in case no element matches - NULL.
 *
 */

#include "private_heap.h"
#include "dbug.h"
#include "internal_lib.h"

struct HEAP {
    size_t elem_size;
    int chunk_size;
    int num_elems;
    char *data;
    struct HEAP *next;
};

#define HEAP_ELEM_SIZE(n) (n->elem_size)
#define HEAP_CHUNK_SIZE(n) (n->chunk_size)
#define HEAP_NUM_ELEMS(n) (n->num_elems)
#define HEAP_DATA(n) (n->data)
#define HEAP_NEXT(n) (n->next)

heap *
PHPcreateHeap (size_t elem_size, int chunk_size)
{
    char *data;
    heap *res;

    DBUG_ENTER ("PHPcreateHeap");

    DBUG_ASSERT (sizeof (char) == 1, "sizeof char is not 1 on this platform");

    data = (char *)ILIBmalloc (elem_size * chunk_size);
    res = (heap *)ILIBmalloc (sizeof (heap));
    HEAP_ELEM_SIZE (res) = elem_size;
    HEAP_CHUNK_SIZE (res) = chunk_size;
    HEAP_NUM_ELEMS (res) = 0;
    HEAP_DATA (res) = data;
    HEAP_NEXT (res) = NULL;

    DBUG_RETURN (res);
}

void *
PHPmalloc (heap *private_heap)
{
    void *res;

    DBUG_ENTER ("PHPmalloc");

    while (HEAP_NUM_ELEMS (private_heap) == HEAP_CHUNK_SIZE (private_heap)) {
        private_heap = HEAP_NEXT (private_heap);
    }
    res = (void *)(HEAP_DATA (private_heap)
                   + HEAP_ELEM_SIZE (private_heap) * HEAP_NUM_ELEMS (private_heap));
    HEAP_NUM_ELEMS (private_heap)++;
    if (HEAP_NUM_ELEMS (private_heap) == HEAP_CHUNK_SIZE (private_heap)) {
        HEAP_NEXT (private_heap)
          = PHPcreateHeap (HEAP_ELEM_SIZE (private_heap), HEAP_CHUNK_SIZE (private_heap));
    }
    DBUG_RETURN (res);
}

void *
PHPfindElem (heap *private_heap, php_cmp_fun fun, void *elem)
{
    bool found = FALSE;
    void *this;
    int i;

    DBUG_ENTER ("PHPfindElem");

    while ((i < HEAP_NUM_ELEMS (private_heap)) && !found) {
        this = (void *)(HEAP_DATA (private_heap) + HEAP_ELEM_SIZE (private_heap) * i);
        found = fun (this, elem);
    }
    if (!found) {
        if (HEAP_NEXT (private_heap) == NULL) {
            this = NULL;
        } else {
            this = PHPfindElem (HEAP_NEXT (private_heap), fun, elem);
        }
    }

    DBUG_RETURN (this);
}

heap *
PHPfreeHeap (heap *private_heap)
{
    DBUG_ENTER ("PHPfreeHeap");

    if (HEAP_NEXT (private_heap) != NULL) {
        HEAP_NEXT (private_heap) = PHPfreeHeap (HEAP_NEXT (private_heap));
    }
    HEAP_DATA (private_heap) = ILIBfree (HEAP_DATA (private_heap));
    private_heap = ILIBfree (private_heap);

    DBUG_RETURN (private_heap);
}
