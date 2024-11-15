/**
 *
 * @file
 *
 * This file provides a simple heap facility.
 * Whereas most of the heap managament in the compiler should be done
 * using MEMmalloc and MEMfree, these sometimes do not suffice.
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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"

struct HEAP {
    size_t elem_size;
    size_t chunk_size;
    size_t num_elems;
    char *data;
    struct HEAP *next;
};

#define HEAP_ELEM_SIZE(n) (n->elem_size)
#define HEAP_CHUNK_SIZE(n) (n->chunk_size)
#define HEAP_NUM_ELEMS(n) (n->num_elems)
#define HEAP_DATA(n) (n->data)
#define HEAP_NEXT(n) (n->next)

/** <!--********************************************************************-->
 *
 * @fn  heap *PHPcreateHeap( size_t elem_size, int chunk_size)
 *
 *   @brief  allocates the heap. NB: In fact only the top most heap frame
 *           is allocated. However, this is invisible to the "user".
 *   @param  elem_size  size of each heap entry (in bytes).
 *   @param  chunk_size size of the heap frames (in elements).
 *   @return pointer to the heap created.
 *
 ******************************************************************************/

heap *
PHPcreateHeap (size_t elem_size, size_t chunk_size)
{
    char *data;
    heap *res;

    DBUG_ENTER ();

    DBUG_ASSERT (sizeof (char) == 1, "sizeof char is not 1 on this platform");

    data = (char *)MEMmalloc (elem_size * chunk_size);
    res = (heap *)MEMmalloc (sizeof (heap));
    HEAP_ELEM_SIZE (res) = elem_size;
    HEAP_CHUNK_SIZE (res) = chunk_size;
    HEAP_NUM_ELEMS (res) = 0;
    HEAP_DATA (res) = data;
    HEAP_NEXT (res) = NULL;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  void *PHPmalloc( heap *private_heap)
 *
 *   @brief  allocates one element (of the prespecified size) within the
 *           given heap.
 *   @param  private_heap   the heap the element is to be drawen from.
 *   @return pointer to the "allocated element".
 *
 ******************************************************************************/

void *
PHPmalloc (heap *private_heap)
{
    void *res;

    DBUG_ENTER ();

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

/** <!--********************************************************************-->
 *
 * @fn  void *PHPfindElem( heap *private_heap, php_cmp_fun fun, void *elem)
 *
 *   @brief  search for a given element in the heap using the comparison
 *           function provided.
 *           given heap.
 *   @param  private_heap   the heap containing the elements to be searched.
 *   @param  fun  comparison function to be used.
 *   @param  elem element to be compared against.
 *   @return pointer to the element we are looking for. in case the element is
 *           not found, NULL is returned.
 *
 ******************************************************************************/

void *
PHPfindElem (heap *private_heap, php_cmp_fun fun, void *elem)
{
    bool found = FALSE;
    void *current = NULL;
    size_t i = 0;

    DBUG_ENTER ();

    while ((i < HEAP_NUM_ELEMS (private_heap)) && !found) {
        current = (void *)(HEAP_DATA (private_heap) + HEAP_ELEM_SIZE (private_heap) * i);
        found = fun (current, elem);
        i++;
    }
    if (!found) {
        if (HEAP_NEXT (private_heap) == NULL) {
            current = NULL;
        } else {
            current = PHPfindElem (HEAP_NEXT (private_heap), fun, elem);
        }
    }

    DBUG_RETURN (current);
}

/** <!--********************************************************************-->
 *
 * @fn  heap *PHPfreeHeap( heap *private_heap)
 *
 *   @brief  frees the entire heap provided
 *   @param  private_heap   the heap to be freed.
 *   @return NULL pointer.
 *
 ******************************************************************************/

heap *
PHPfreeHeap (heap *private_heap)
{
    DBUG_ENTER ();

    if (private_heap != NULL) {
        if (HEAP_NEXT (private_heap) != NULL) {
            HEAP_NEXT (private_heap) = PHPfreeHeap (HEAP_NEXT (private_heap));
        }
        HEAP_DATA (private_heap) = MEMfree (HEAP_DATA (private_heap));
        private_heap = MEMfree (private_heap);
    }

    DBUG_RETURN (private_heap);
}

#undef DBUG_PREFIX
