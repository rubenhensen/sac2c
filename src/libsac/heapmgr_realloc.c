/*
 *
 * $Log$
 * Revision 1.2  1999/07/16 09:41:16  cg
 * Added facilities for heap management diagnostics.
 *
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

    DIAG_INC (SAC_HM_call_calloc);
    DIAG_DEC (SAC_HM_call_malloc);

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

    DIAG_INC (SAC_HM_call_realloc);

    if (ptr == NULL) {
        DIAG_DEC (SAC_HM_call_malloc);
        return (malloc (size));
    }

    if (size == 0) {
        DIAG_DEC (SAC_HM_call_free);
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

    DIAG_DEC (SAC_HM_call_malloc);
    mem = malloc (size);

    mem = memcpy (mem, ptr, MIN (old_size_units * UNIT_SIZE, size));

    DIAG_DEC (SAC_HM_call_free);
    free (ptr);

    return (mem);
}
