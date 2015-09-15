/*****************************************************************************
 *
 * file:  small_chunks.c
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description:
 *
 *   This file contains the core allocation and de-allocation routines
 *   for small chunk arenas.
 *
 *   For a detailed description of the arena concept see file heapmgr.h.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <assert.h>

#include "heapmgr.h"

/******************************************************************************
 *
 * function:
 *   void *SAC_DISTMEM_HM_MallocSmallChunk( SAC_DISTMEM_HM_size_unit_t units,
 *SAC_DISTMEM_HM_arena_t *arena)
 *
 * description:
 *
 *   This function allocates a chunk of memory with the size given in units.
 *   The memory is allocated from the given arena of small chunks .
 *
 *   allocation strategy:
 *
 *    1. Check free list for entry.
 *    2. Split from top of wilderness chunk.
 *    3. Take entire wilderness chunk.
 *    4. Allocate new bin and split from new wilderness chunk.
 *
 ******************************************************************************/

void *
SAC_DISTMEM_HM_MallocSmallChunk (SAC_DISTMEM_HM_size_unit_t units,
                                 SAC_DISTMEM_HM_arena_t *arena)
{
    SAC_DISTMEM_HM_header_t *freep, *wilderness;

    assert (units >= arena->min_chunk_size);
    DIAG_INC (arena->cnt_alloc);

    /*
     * Search for chunk of memory in the free list.
     */

    freep = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (arena->freelist);

    if (freep != NULL) {
        /*
         * Any entry in the free list will exactly fit our needs, so we may simply
         * take the first one provided that the free list is not empty.
         * The first entry is simply removed from the free list and returned to the
         * calling context.
         */
        DIAG_CHECK_FREEPATTERN_SMALLCHUNK (freep, arena->num);
        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);
        DIAG_INC (arena->cnt_after_freelist);

        SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (arena->freelist)
          = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep);

        return ((void *)(freep + 1));
    }

    /*
     * There has been no entry in the free list,
     * so try to split memory from the arena's wilderness chunk.
     */

    wilderness = arena->wilderness;

    if (SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness) > units) {
        /*
         * The wilderness chunk is sufficiently large to satisfy the needs,
         * so split a small chunk from its top and return it to the calling
         * context.
         */
        SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness) -= units;
        freep = wilderness + SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness);
        SAC_DISTMEM_HM_SMALLCHUNK_ARENA (freep) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);
        DIAG_INC (arena->cnt_after_wilderness);

        return ((void *)(freep + 1));
    }

    if (SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness) == units) {
        /*
         * The wilderness chunk exactly satisfies the needs,
         * so return it to the calling context and disable wilderness
         * chunk in arena representation.
         */
        SAC_DISTMEM_HM_SMALLCHUNK_ARENA (wilderness) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (wilderness);
        DIAG_INC (arena->cnt_after_wilderness);

        arena->wilderness = arena->freelist;

        return ((void *)(wilderness + 1));
    }

    /*
     * There has also been not enough space in the wilderness chunk,
     * so we have to allocate a new bin for the given arena and use this as
     * the arena's new wilderness chunk.
     * Afterwards, the reqired chunk of memory is cut from the top of the
     * new wilderness chunk and returned to the calling context.
     */

    wilderness
      = SAC_DISTMEM_HM_AllocateNewBinInArenaOfArenas (arena->binsize, arena - arena->num);

    DIAG_INC (arena->cnt_bins);
    DIAG_ADD (arena->size, arena->binsize * SAC_DISTMEM_HM_UNIT_SIZE);
    DIAG_INC (arena->cnt_after_extension);

    SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness) = arena->binsize - units;
    arena->wilderness = wilderness;
    freep = wilderness + SAC_DISTMEM_HM_SMALLCHUNK_SIZE (wilderness);
    SAC_DISTMEM_HM_SMALLCHUNK_ARENA (freep) = arena;

    DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

    return ((void *)(freep + 1));
}

/******************************************************************************
 *
 * function:
 *   void do_free_small_chunk(SAC_DISTMEM_HM_header_t *freep, SAC_DISTMEM_HM_arena_t
 **arena)
 *
 * description:
 *
 *   This function de-allocates memory chunks in arenas of small chunks.
 *   Free chunks are simply inserted into the free list. There is no coalascing of
 *   adjacent free chunks because such arenas contain only chunks of equal
 *   size.
 *
 ******************************************************************************/

static inline void
do_free_small_chunk (SAC_DISTMEM_HM_header_t *freep, SAC_DISTMEM_HM_arena_t *arena)
{
    DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_SMALLCHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep)
      = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (arena->freelist);
    SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (arena->freelist) = freep;
}

void
SAC_DISTMEM_HM_FreeSmallChunk (SAC_DISTMEM_HM_header_t *addr,
                               SAC_DISTMEM_HM_arena_t *arena)
{
    /* addr points at the first user byte of the chunk ("returned pointer").
     * freep points at the proper header start of the small chunks.
     * nextfree is a field already in the user part, hence we may overwrite/reuse
     * it here at will. */
    SAC_DISTMEM_HM_header_t *freep = addr - 1;

    /* directly free the chunk */
    do_free_small_chunk (freep, arena);
}
