/*
 *
 * $Log$
 * Revision 1.1  1999/07/08 12:28:56  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#include "heapmgr.h"

void *
calloc (size_byte_t nelem, size_byte_t elsize)
{
    void *res;

    res = malloc (nelem * elsize);

    res = memset (res, 0, nelem * elsize);

    return (res);
}

void *
realloc (void *ptr, size_byte_t size)
{
    void *mem;
    size_unit_t old_size_units;
    SAC_HM_arena_t *arena;

    if (ptr == NULL) {
        return (malloc (size));
    }

    if (size == 0) {
        free (ptr);
        return (NULL);
    }

    arena = SAC_HM_ADDR_2_ARENA (ptr);

    if (arena->min_chunk_size
        <= SAC_HM_arenas[NUM_SMALLCHUNK_ARENAS - 1].min_chunk_size) {
        old_size_units = arena->min_chunk_size;
        if (size <= old_size_units) {
            return (ptr);
        }
    } else {
        old_size_units = LARGECHUNK_SIZE (((SAC_HM_header_t *)ptr) - 2);
    }

    mem = malloc (size);

    mem = memcpy (mem, ptr, MIN (old_size_units * UNIT_SIZE, size));

    free (ptr);

    return (mem);
}
