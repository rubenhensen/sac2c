/*
 *
 * $Log$
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   internal.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains global data and functions internally used by the
 *   SAC private heap manager.
 *
 *
 *****************************************************************************/

#include "heapmgr.h"
#include "sac_message.h"

#include <unistd.h>

/******************************************************************************
 *
 * function:
 *   void SAC_HM_OutOfMemory( size_byte_t request)
 *
 * description:
 *
 *   This function displays an error message and aborts program execution
 *   whenever a request for memory cannot be satisfied by the heap manager.
 *
 ******************************************************************************/

void
SAC_HM_OutOfMemory (size_byte_t request)
{
    SAC_RuntimeError ("SAC heap manager failed to obtain %lu Bytes of memory "
                      "from operating system !",
                      request);
}

/******************************************************************************
 *
 * function:
 *   SAC_HM_header_t *SAC_HM_AllocateNewBinInArenaOfArenas(size_unit_t units)
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

SAC_HM_header_t *
SAC_HM_AllocateNewBinInArenaOfArenas (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *lastp;
    size_unit_t split_threshold;

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;

    /*
     * Search for sufficiently large chunk of memory in the free list.
     */

    lastp = arena->freelist;
    freep = SAC_HM_SMALLCHUNK_NEXTFREE (lastp);

    while (freep != NULL) {

        if (SAC_HM_SMALLCHUNK_SIZE (freep) >= split_threshold) {
            /*
             * The current chunk of memory is larger than required, so split
             * the amount of memory needed from the top of the chunk.
             */
            DIAG_INC (arena->cnt_split);
            SAC_HM_SMALLCHUNK_SIZE (freep) -= units;
            return (freep + SAC_HM_SMALLCHUNK_SIZE (freep));
        }

        if (SAC_HM_SMALLCHUNK_SIZE (freep) >= units) {
            /*
             * The current chunk of memory more or less fits exactly, so remove it
             * from free list and return it to the calling context.
             */
            SAC_HM_SMALLCHUNK_NEXTFREE (lastp) = SAC_HM_SMALLCHUNK_NEXTFREE (freep);
            return (freep);
        }

        /*
         *  The current chunk of memory is too small, so continue with the next one.
         */
        lastp = freep;
        freep = SAC_HM_SMALLCHUNK_NEXTFREE (freep);
    }

    /*
     * No sufficient space found in free list, so, we have to allocate
     * an additional bin for the arena of arenas within the top arena.
     * We will only return to this point if the additional allocation succeeds.
     */

#ifdef MT
    if (SAC_MT_not_yet_parallel) {
        freep
          = (SAC_HM_header_t *)SAC_HM_MallocLargeChunk (arena->binsize + 2,
                                                        &(SAC_HM_arenas[0][TOP_ARENA]));
    } else {
        pthread_mutex_lock (&SAC_HM_top_arena_lock);
        freep
          = (SAC_HM_header_t *)SAC_HM_MallocLargeChunk (arena->binsize + 2,
                                                        &(SAC_HM_arenas[0][TOP_ARENA]));
        pthread_mutex_unlock (&SAC_HM_top_arena_lock);
    }
#else  /* MT */
    freep = (SAC_HM_header_t *)SAC_HM_MallocLargeChunk (arena->binsize + 2,
                                                        &(SAC_HM_arenas[0][TOP_ARENA]));
#endif /* MT */

    DIAG_ADD (arena->size, arena->binsize * UNIT_SIZE);
    DIAG_INC (arena->bins);

    SAC_HM_SMALLCHUNK_SIZE (freep) = arena->binsize - units;

    SAC_HM_SMALLCHUNK_NEXTFREE (freep) = SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist);
    SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist) = freep;

    return (freep + SAC_HM_SMALLCHUNK_SIZE (freep));
}

/******************************************************************************
 *
 * function:
 *   SAC_HM_header_t *SAC_HM_ExtendTopArenaWilderness( size_unit_t units)
 *
 * description:
 *
 *   This function allows to extend the top arena´s wilderness chunk by
 *   "fresh" memory obtained from the operating system.
 *
 *
 *
 *
 *
 ******************************************************************************/

SAC_HM_header_t *
SAC_HM_ExtendTopArenaWilderness (size_unit_t units)
{
    size_unit_t new_mem;
    char *mem;
    SAC_HM_header_t *wilderness;
    SAC_HM_arena_t *arena = &(SAC_HM_arenas[0][TOP_ARENA]);

    wilderness = arena->wilderness;

    /*
     * First, the amount of additionally required memory is computed, i.e.
     * the requested amount of memory decreased by the amount of memory still
     * available in the wilderness chunk. The result then is rounded up to
     * the next multiple of SBRK_CHUNK. This guarantees that memory is requested
     * from the operating system in sufficiently large chunks since calls to
     * the system call sbrk() are usually expensive.
     */
    new_mem = ((units - SAC_HM_LARGECHUNK_SIZE (wilderness) + 3) * UNIT_SIZE);
    new_mem = (new_mem + SBRK_CHUNK) & (~(SBRK_CHUNK - 1));

    DIAG_INC (SAC_HM_call_sbrk);

    /*
     * Actually, request memory from operating system.
     */
    mem = (char *)sbrk (new_mem);
    if (mem == (char *)-1) {
        /*
         * The operating system has denied the request for additional memory,
         * so we give up right now.
         */
        SAC_HM_OutOfMemory (new_mem);
    }

    DIAG_ADD (SAC_HM_heapsize, new_mem);
    DIAG_ADD (arena->size, new_mem);
    DIAG_INC (arena->bins);

    if ((SAC_HM_header_t *)mem == wilderness + SAC_HM_LARGECHUNK_SIZE (wilderness)) {
        /*
         * The new memory and the old wilderness chunk are contiguous.
         * This should always be the case for single-threaded operation or as
         * long as only one thread has control over the top arena.
         * As a consequence the old wilderness chunk may simply be extended.
         */
        SAC_HM_LARGECHUNK_SIZE (wilderness) += new_mem / UNIT_SIZE;
        return (wilderness);
    } else {
        /*
         * New memory and old wilderness chunk are NOT contiguous.
         * The old wilderness becomes regular free chunk; the request will be
         * satisfied from freshly allocated memory alone.
         */
        SAC_HM_LARGECHUNK_NEXTFREE (wilderness)
          = SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist);
        SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist) = wilderness;

        if (new_mem >= units * UNIT_SIZE) {
            /*
             * The freshly allocated memory suffices to satisfy request,
             * so make it the new wilderness chunk.
             */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            SAC_HM_LARGECHUNK_SIZE (wilderness) = new_mem / UNIT_SIZE;
            SAC_HM_LARGECHUNK_PREVSIZE (wilderness) = -1;

            DIAG_SET_FREEPATTERN_LARGECHUNK (wilderness);

            return (wilderness);
        } else {
            /*
             * The freshly allocated memory does NOT suffice to satisfy request,
             * so obtain additional memory from operating system.
             */
            size_unit_t more_mem;
            char *mem2;

            /*
             * Compute amount of additional memory required to satisfy the request.
             */
            more_mem = units * UNIT_SIZE - new_mem;
            more_mem = (more_mem + SBRK_CHUNK) & (~(SBRK_CHUNK - 1));

            DIAG_INC (SAC_HM_call_sbrk);

            /*
             * Request additional memory from operating system.
             */
            mem2 = (char *)sbrk (more_mem);
            if ((mem2 == (char *)-1) || (mem + new_mem != mem2)) {
                /*
                 * The operating system has denied the request for additional memory,
                 * so we give up right now.
                 */
                SAC_HM_OutOfMemory (more_mem);
            }

            DIAG_ADD (SAC_HM_heapsize, more_mem);
            DIAG_ADD (arena->size, more_mem);
            DIAG_INC (arena->bins);

            if (((SAC_HM_header_t *)mem) + new_mem * UNIT_SIZE
                != ((SAC_HM_header_t *)mem2)) {
                /*
                 * The additionally allocated memory and the originally allocated memory
                 * are not contiguous. In this case, we give up immediately, however, this
                 * usually should not occur.
                 */
                SAC_RuntimeError ("Heap manager failed to obtain contiguous memory from "
                                  "operating system");
            }

            /*
             * Now mem and mem2 are guaranteed to refer to contiguous sections of memory.
             * So, initialize and return the new wilderness chunk.
             */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            SAC_HM_LARGECHUNK_SIZE (wilderness) = (new_mem + more_mem) / UNIT_SIZE;
            SAC_HM_LARGECHUNK_PREVSIZE (wilderness) = -1;
            DIAG_SET_FREEPATTERN_LARGECHUNK (wilderness);

            return (wilderness);
        }
    }
}
