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
memalign (size_byte_t alignment, size_byte_t size)
{
    void *mem;
    size_byte_t misalign;
    SAC_HM_header_t *freep, *prefixp, *prevp, *freelist;
    SAC_HM_arena_t *arena;
    size_unit_t offset_units, units_allocated;

    DIAG_INC (SAC_HM_call_memalign);

    if (alignment <= UNIT_SIZE) {
        /* automatic alignment */
        return (malloc (size));
    }

    /* worst case allocation for a posteriori alignment */
    units_allocated = SAC_HM_UNITS (size + alignment) + 2;

    if (units_allocated < ARENA_7_MINCS) {
        /* Now, it's arena 5 or 6. */
        if (units_allocated < ARENA_6_MINCS) {
            arena = &(SAC_HM_arenas[5]);
            mem = SAC_HM_MallocLargeChunk (units_allocated, arena);
        } else {
            arena = &(SAC_HM_arenas[6]);
            mem = SAC_HM_MallocLargeChunk (units_allocated, arena);
        }
    } else {
        /* Now, it's arena 7 or 8. */
        if (units_allocated < ARENA_8_MINCS) {
            arena = &(SAC_HM_arenas[7]);
            mem = SAC_HM_MallocLargeChunk (units_allocated, arena);
        } else {
            arena = &(SAC_HM_arenas[8]);
            mem = SAC_HM_MallocTopArena (units_allocated, arena);
        }
    }

    misalign = ((unsigned long int)mem) % alignment;

    if (misalign == 0) {
        /* Memory is already correctly aligned. */
        return (mem);
    }

    offset_units = (alignment - misalign) / UNIT_SIZE;

    if (offset_units < 2) {
        /*
         * Offset is too small to host administration info for free prefix.
         */
        offset_units += alignment / UNIT_SIZE;
    }

    prefixp = ((SAC_HM_header_t *)mem) - 2;
    freep = prefixp + offset_units;

    /*
     * Setup pointer location to be returned.
     */

    LARGECHUNK_SIZE (freep) = units_allocated - offset_units;
    LARGECHUNK_ARENA (freep) = arena;

    if (LARGECHUNK_PREVSIZE (prefixp) > 0) {
        /* previous chunk is free, so, coalasce with prefix. */
        prevp = prefixp - LARGECHUNK_PREVSIZE (prefixp);
        LARGECHUNK_SIZE (prevp) += offset_units;
        LARGECHUNK_PREVSIZE (freep) = LARGECHUNK_SIZE (prevp);
    } else {
        /* previous chunk is NOT free, so, keep prefix separately. */
        LARGECHUNK_SIZE (prefixp) = offset_units;
        LARGECHUNK_PREVSIZE (freep) = offset_units;

        if (offset_units >= arena->min_chunk_size) {
            /* Prefix chunk is large enough to be inserted into arena's free list. */
            freelist = arena->freelist;
            LARGECHUNK_NEXTFREE (prefixp) = LARGECHUNK_NEXTFREE (freelist);
            LARGECHUNK_PREVFREE (prefixp) = freelist;
            LARGECHUNK_NEXTFREE (freelist) = prefixp;
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (prefixp)) = prefixp;
        }
    }

    return ((void *)(freep + 2));
}

void *
valloc (size_byte_t size)
{
    DIAG_INC (SAC_HM_call_valloc);

    return (memalign (SAC_HM_pagesize, size));
}
