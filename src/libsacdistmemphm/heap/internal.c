/*****************************************************************************
 *
 * file:   internal.c
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description:
 *
 *   This file contains global data and functions internally used by the
 *   SAC distributed memory private heap manager.
 *
 *
 *****************************************************************************/

#include "config.h"
#include <unistd.h>
#include <stdint.h>

#include "heapmgr.h"

/******************************************************************************
 *
 * function:
 *   SAC_DISTMEM_HM_header_t
 **SAC_DISTMEM_HM_AllocateNewBinInArenaOfArenas(SAC_DISTMEM_HM_size_unit_t units)
 *
 * description:
 *
 *   This function allocates a new bin for a specific arena within the
 *   arena of arenas. The size of the new bin is specified in units.
 *
 *   The diagnostic check pattern mechanism cannot be used in the arena of arenas
 *   because here we use small chunks, but definitely need the sizes of all chunks
 *   since these are NOT uniform. Small chunk administration can, nevertheless,
 *   be used since bins in the arena of arenas are never de-allocated.
 *   Consequently, there is no chance of coalascing de-allocated chunks.
 *
 *
 ******************************************************************************/

SAC_DISTMEM_HM_header_t *
SAC_DISTMEM_HM_AllocateNewBinInArenaOfArenas (SAC_DISTMEM_HM_size_unit_t units,
                                              SAC_DISTMEM_HM_arena_t *arena)
{
    SAC_DISTMEM_HM_header_t *freep, *lastp;
    SAC_DISTMEM_HM_size_unit_t split_threshold;

#ifdef DIAG
    if (arena->num != SAC_DISTMEM_HM_ARENA_OF_ARENAS) {
        SAC_RuntimeError ("Arena should be arena of arenas but is No. %d", arena->num);
    }
#endif

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;

    /*
     * Search for sufficiently large chunk of memory in the free list.
     */

    lastp = arena->freelist;
    freep = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (lastp);

    while (freep != NULL) {

        if (SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep) >= split_threshold) {
            /*
             * The current chunk of memory is larger than required, so split
             * the amount of memory needed from the top of the chunk.
             */
            DIAG_INC (arena->cnt_after_splitting);
            SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep) -= units;
            return (freep + SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep));
        }

        if (SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep) >= units) {
            /*
             * The current chunk of memory more or less fits exactly, so remove it
             * from free list and return it to the calling context.
             */
            DIAG_INC (arena->cnt_after_freelist);
            SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (lastp)
              = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep);
            return (freep);
        }

        /*
         *  The current chunk of memory is too small, so continue with the next one.
         */
        lastp = freep;
        freep = SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep);
    }

    /*
     * No sufficient space found in free list, so, we have to allocate
     * an additional bin for the arena of arenas within the top arena.
     * We will only return to this point if the additional allocation succeeds.
     */
    freep = (SAC_DISTMEM_HM_header_t *)
      SAC_DISTMEM_HM_MallocLargeChunk (arena->binsize + 2,
                                       &(SAC_DISTMEM_HM_arenas
                                           [SAC_DISTMEM_HM_TOP_ARENA]));

    /* Increase size of arena. */
    DIAG_ADD (arena->size, arena->binsize * SAC_DISTMEM_HM_UNIT_SIZE);
    /* Subtract added size from top arena so that the total size remains correct.
     * We can safely do this because we never return memory to the top arena. */
    DIAG_ADD (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA].size,
              -((arena->binsize + 2) * SAC_DISTMEM_HM_UNIT_SIZE));

    DIAG_INC (arena->cnt_bins);
    DIAG_INC (arena->cnt_after_extension);

    SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep) = arena->binsize - units;

    /*
     * The new bin is added at the end of the free list.
     */
    SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
    SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (lastp) = freep;

    return (freep + SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep));
}
