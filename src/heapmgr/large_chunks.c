/*
 *
 * $Log$
 * Revision 1.2  2000/01/17 16:25:58  cg
 * Added multi-threading capabilities to the heap manager.
 *
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:  large_chunks.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains the core allocation and de-allocation routines
 *   for large chunk arenas.
 *
 *   For a detailed description of the arena concept see file heapmgr.h.
 *
 *****************************************************************************/

#include <stdlib.h>

#include "heapmgr.h"
#include "sac_message.h"

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocLargeChunk( SAC_HM_size_unit_t units,
 *                                  SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function allocates a chunk of memory with the size given in units.
 *   The memory is allocated from the given arena of large chunks .
 *
 *   If no suffificent space can be found in the free list but adjacent
 *   free chunks of memory exist, such chunks are coalasced in order to
 *   create larger free chunks and satisfy the request for memory without
 *   extending the arena.
 *
 *   general allocation strategy:
 *
 *    1. Check free list for an entry that is large enough with respect to
 *       the memory request, but does not exceed this by more than a certain
 *       threshold.
 *    2. Split the requested amount of memory from a large enough entry of
 *       the free list.
 *    3. Split the requested amount of memory from the wilderness chunk.
 *    4. Coalasce adjacent chunks of memory in the free list.
 *    5. Coalasce the wilderness chunk with an adjacent free chunk from the
 *       free list.
 *    6. Extend the wilderness chunk by either requesting more memory from the
 *       operating system (top arena) or by allocating a new bin (other arenas).
 *
 ******************************************************************************/

void *
SAC_HM_MallocLargeChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *bestp, *lastp, *wilderness, *new_wilderness;
    SAC_HM_header_t *prevp;

    SAC_HM_size_unit_t split_threshold;

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;
    bestp = NULL;

    /*
     * Search for sufficiently large chunk of memory in the free list.
     */

    lastp = arena->freelist;
    freep = SAC_HM_LARGECHUNK_NEXTFREE (lastp);

    while (freep != NULL) {
        DIAG_CHECK_FREEPATTERN_LARGECHUNK (freep, arena->num);

        if (SAC_HM_LARGECHUNK_SIZE (freep) < units) {
            /*
             *  The current chunk of memory is too small, so continue with the next one.
             */
            lastp = freep;
            freep = SAC_HM_LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        if (SAC_HM_LARGECHUNK_SIZE (freep) >= split_threshold) {
            /*
             * The current chunk of memory is larger than required, so remember it for
             * potential later use and continue with the next one.
             */
            bestp = freep;
            lastp = freep;
            freep = SAC_HM_LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /*
         * The current chunk of memory more or less fits exactly, so remove it
         * from the free list and return it to the calling context.
         */

        DIAG_INC (arena->cnt_after_freelist);

    exact_fit:
        SAC_HM_LARGECHUNK_NEXTFREE (lastp) = SAC_HM_LARGECHUNK_NEXTFREE (freep);

        SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep)) = -1;
        DIAG_SET_ALLOCPATTERN_LARGECHUNK (freep);

        return ((void *)(freep + 2));
    }

    /*
     * No exactly fitting space found in free list, so we
     * try to split from a larger chunk.
     */

    if (bestp != NULL) {
        /*
         * During the traversal of the free list we have found a sufficiently
         * large chunk of memory that will be splitted now. The chunk to be
         * returned is taken from the top.
         */
        DIAG_INC (arena->cnt_after_splitting);

    split:
        SAC_HM_LARGECHUNK_SIZE (bestp) -= units;
        freep = bestp + SAC_HM_LARGECHUNK_SIZE (bestp);
        SAC_HM_LARGECHUNK_SIZE (freep) = units;
        SAC_HM_LARGECHUNK_ARENA (freep) = arena;
        SAC_HM_LARGECHUNK_PREVSIZE (freep) = SAC_HM_LARGECHUNK_SIZE (bestp);
        SAC_HM_LARGECHUNK_PREVSIZE (freep + units) = -1;

        DIAG_SET_ALLOCPATTERN_LARGECHUNK (freep);

        return ((void *)(freep + 2));
    }

    /*
     * We haven't found any chunk of sufficient size, so we now try to split
     * from the wilderness chunk. The wilderness chunk only exists in the top
     * arena, otherwise arena->wilderness points to the first (dummy) entry of
     * the free list. The size of this entry is always 0. In large chunk arenas,
     * this step of the allocation process is automatically skipped.
     */

    wilderness = arena->wilderness;
    DIAG_CHECK_FREEPATTERN_LARGECHUNK (wilderness, arena->num);

    if (SAC_HM_LARGECHUNK_SIZE (wilderness) >= units + 3) {
        /*
         * The wilderness chunk is sufficiently large, so split the requested
         * amount of memory from the bottom of the wilderness chunk.
         * This technique is slightly less efficient than splitting from the top.
         * However, in the case of the top arena´s wilderness chunk this allows
         * to extend the wilderness subsequently without unnecessary fragmentation.
         */

        DIAG_INC (arena->cnt_after_wilderness);

    split_wilderness:
        new_wilderness = wilderness + units;
        SAC_HM_LARGECHUNK_SIZE (new_wilderness)
          = SAC_HM_LARGECHUNK_SIZE (wilderness) - units;
        SAC_HM_LARGECHUNK_PREVSIZE (new_wilderness) = -1;
        DIAG_SET_FREEPATTERN_LARGECHUNK (new_wilderness);
        arena->wilderness = new_wilderness;

        SAC_HM_LARGECHUNK_SIZE (wilderness) = units;
        SAC_HM_LARGECHUNK_ARENA (wilderness) = arena;
        DIAG_SET_ALLOCPATTERN_LARGECHUNK (wilderness);

        return ((void *)(wilderness + 2));
    }

    /*
     * There is no sufficient space in the wilderness chunk, so we now coalasce
     * coalascable chunks until a sufficiently large free chunk has been
     * created or until all chunks are coalasced as far as possible.
     */

    lastp = arena->freelist;
    freep = SAC_HM_LARGECHUNK_NEXTFREE (lastp);

    while (freep != NULL) {
        if (SAC_HM_LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * The previous adjacent chunk to the current chunk is also free,
             * so coalasce the two and remove the current chunk from the free list.
             */
            DIAG_INC (arena->cnt_coalascing);
            prevp = freep - SAC_HM_LARGECHUNK_PREVSIZE (freep);
            SAC_HM_LARGECHUNK_SIZE (prevp) += SAC_HM_LARGECHUNK_SIZE (freep);
            SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep))
              = SAC_HM_LARGECHUNK_SIZE (prevp);
            SAC_HM_LARGECHUNK_NEXTFREE (lastp) = SAC_HM_LARGECHUNK_NEXTFREE (freep);

            if (SAC_HM_LARGECHUNK_SIZE (prevp) >= units) {
                DIAG_INC (arena->cnt_after_coalascing);

                if (SAC_HM_LARGECHUNK_SIZE (freep) >= split_threshold) {
                    /*
                     * The coalasced chunk is larger than the amount of memory requested,
                     * so we split an appropriate chunk from the top of it.
                     */
                    bestp = prevp;
                    goto split;
                } else {
                    /*
                     * The coalasced chunk more or less exactly fits the amount of memory
                     * requested, so it is removed from the free list and returned to the
                     * current context.
                     *
                     * Unfortunately, for this purpose we need to know its predecessor in
                     * the free list, an information that is currently not available. As a
                     * consequence, the entire free list has to be traversed to obtain
                     * this information.
                     *
                     * This of course is expensive with respect to performance, however it
                     * avoids even more expensive doubly linked lists for the
                     * representation of free lists.
                     */
                    freep = prevp;
                    lastp = arena->freelist;
                    while (SAC_HM_LARGECHUNK_NEXTFREE (lastp) != freep) {
                        lastp = SAC_HM_LARGECHUNK_NEXTFREE (lastp);
                    }
                    goto exact_fit;
                }
            } else {
                freep = SAC_HM_LARGECHUNK_NEXTFREE (freep);
                continue;
            }
        } else {
            lastp = freep;
            freep = SAC_HM_LARGECHUNK_NEXTFREE (freep);
        }
    }

    /*
     * We haven't found sufficient space by coalascing chunks in the free list.
     * We now try to coalasce the wilderness with the adjacent free chunks.
     * An actual wilderness chunk only exists in the top arena,
     * otherwise wilderness points to the first (dummy) entry of
     * the free list. The prevsize of this entry is always 0. In large chunk arenas,
     * this step of the allocation process is automatically skipped.
     */

    if (SAC_HM_LARGECHUNK_PREVSIZE (wilderness) > 0) {
        /*
         * The chunk adjacent to the wilderness chunk is currently free.
         */
        DIAG_INC (arena->cnt_coalascing_wilderness);
        new_wilderness = wilderness - SAC_HM_LARGECHUNK_PREVSIZE (wilderness);
        SAC_HM_LARGECHUNK_SIZE (new_wilderness) += SAC_HM_LARGECHUNK_SIZE (wilderness);

        /*
         * Next, the coalasced free chunk adjacent to the wilderness chunk must
         * be removed from the free list. Unfortunately, this requires a retraversal
         * the free list.
         */
        lastp = arena->freelist;
        while (SAC_HM_LARGECHUNK_NEXTFREE (lastp) != new_wilderness) {
            lastp = SAC_HM_LARGECHUNK_NEXTFREE (lastp);
#ifdef DIAG
            if (lastp == NULL) {
                atexit (SAC_HM_ShowDiagnostics);
                SAC_RuntimeError (
                  "Corrupted free list encountered upon coalascing wilderness "
                  "chunk in arena %d",
                  arena->num);
            }
#endif
        }
        SAC_HM_LARGECHUNK_NEXTFREE (lastp) = SAC_HM_LARGECHUNK_NEXTFREE (new_wilderness);

        wilderness = new_wilderness;
        arena->wilderness = wilderness;

        if (SAC_HM_LARGECHUNK_SIZE (wilderness) >= units + 3) {
            /*
             * Now, the wilderness is large enough, so we directly split the requested
             * amount of memory from it.
             */
            DIAG_INC (arena->cnt_after_coalascing_wilderness);
            goto split_wilderness;
        }
    }

    /*
     * We haven't found any chunk of sufficient size, so we either allocate a new
     * bin from the arena of arenas or we extend the wilderness chunk if the given
     * arena is the top arena.
     */

    DIAG_INC (arena->cnt_after_extension);

    if (arena->num == SAC_HM_TOP_ARENA) {
        /*
         * The given arena is the top arena, so we try to get new memory from the
         * operating system and extend the wilderness chunk.
         */

        wilderness = SAC_HM_ExtendTopArenaWilderness (units);
        goto split_wilderness;
    } else {
        /*
         * The given arena is not the top arena but a usual large chunk arena.
         * So, we allocate a new bin in the arena of arenas, initialize it as
         * one relatively large chunk of memory and insert it on top of the free
         * list.
         */

        bestp = SAC_HM_AllocateNewBinInArenaOfArenas (arena->binsize, arena - arena->num);

        DIAG_INC (arena->cnt_bins);
        DIAG_ADD (arena->size, arena->binsize * SAC_HM_UNIT_SIZE);
        DIAG_SET_FREEPATTERN_LARGECHUNK (bestp);

        SAC_HM_LARGECHUNK_SIZE (bestp) = arena->binsize - 1;
        /*
         * We cannot use the full amount of arena->binsize because 1 unit is used
         * for administrative purposes within the arena of arenas.
         */

        SAC_HM_LARGECHUNK_PREVSIZE (bestp) = -1;
        SAC_HM_LARGECHUNK_ARENA (bestp) = arena;

        SAC_HM_LARGECHUNK_NEXTFREE (bestp) = SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist);
        SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist) = bestp;

        goto split;
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_FreeLargeChunk(SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function de-allocates memory chunks in arenas of large chunks
 *   including the top arena. Coalascing of adjacent free chunks is deferred
 *   until new memory requests can no longer be satisfied without.
 *
 *   The memory chunk to be freed is simply added to the arena's free list.
 *
 *
 *
 ******************************************************************************/

void
SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep;

    freep = addr - 2;

    DIAG_CHECK_ALLOCPATTERN_LARGECHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep))
      = SAC_HM_LARGECHUNK_SIZE (freep);

    SAC_HM_LARGECHUNK_NEXTFREE (freep) = SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist);
    SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist) = freep;
}

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocLargeChunkNoCoalasce( size_unit_t units,
 *                                            SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function allocates a chunk of memory with the size given in units.
 *   The memory is allocated from the given arena of large chunks .
 *
 *   This allocation routine does NOT do any coalascing. Since the de-allocation
 *   routine for large chunks also does not do coalascing, the usage of this
 *   function leads to increasing fragmentation of the memory and an increased
 *   overall memory consumption.
 *
 *
 *
 ******************************************************************************/

#if 0
/* Allocation without coalascing is currently deactivated. */

void *SAC_HM_MallocLargeChunkNoCoalasce( size_unit_t units, 
                                         SAC_HM_arena_t *arena)
{
  SAC_HM_header_t *freep, *lastp, *bestp;
  size_unit_t split_threshold;

  DIAG_INC( arena->cnt_alloc);
   
  split_threshold = units + arena->min_chunk_size;
  bestp = NULL;

  /*
   * Search for sufficiently large chunk of memory in the free list.
   */

  lastp = arena->freelist;
  freep = SAC_HM_LARGECHUNK_NEXTFREE(lastp);
  
  while (freep != NULL) {
    DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena->num);
    
    if (SAC_HM_LARGECHUNK_SIZE(freep) < units) {
      /*
       *  The current chunk of memory is too small, so continue with the next one.
       */
      lastp = freep;
      freep = SAC_HM_LARGECHUNK_NEXTFREE(freep);
      continue;
    }
    
    if (SAC_HM_LARGECHUNK_SIZE(freep) >= split_threshold) {
      /*
       * The current chunk of memory is larger than required, so remember it for
       * potential later use and continue with the next one.
       */
      bestp = freep;
      lastp = freep;
      freep = SAC_HM_LARGECHUNK_NEXTFREE(freep);
      continue;
    }
    
    /*
     * The current chunk of memory more or less fits exactly, so remove it
     * from the free list, mark it as being allocated, and return it to the 
     * calling context.
     */
    SAC_HM_LARGECHUNK_NEXTFREE( lastp) = SAC_HM_LARGECHUNK_NEXTFREE( freep);

    SAC_HM_LARGECHUNK_PREVSIZE(freep + SAC_HM_LARGECHUNK_SIZE(freep)) = -1;
    DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep);

    return((void *)(freep+2));
  }


  /*
   * No exactly fitting space found in freelist, so, we 
   * try to split from best fitting chunk found so far.
   */

  if (bestp == NULL) {
    /*
     * We haven't found any chunk of sufficient size, so we allocate a new
     * bin from the arena of arenas.
     */

    bestp = AllocateNewBinInArenaOfArenas( arena->arena_size);
  
    DIAG_INC( arena->bins);
    DIAG_ADD( arena->size, arena->arena_size * UNIT_SIZE);
    DIAG_SET_FREEPATTERN_LARGECHUNK(bestp);

    SAC_HM_LARGECHUNK_SIZE(bestp) = arena->arena_size - 2;
    SAC_HM_LARGECHUNK_PREVSIZE(bestp) = -1;
    SAC_HM_LARGECHUNK_ARENA(bestp) = arena;
  
    SAC_HM_LARGECHUNK_NEXTFREE(bestp) = SAC_HM_LARGECHUNK_NEXTFREE(arena->freelist);
    SAC_HM_LARGECHUNK_NEXTFREE(arena->freelist) = bestp;
  }
  

  /*
   * Now, we do have a sufficiently large amount of memory, so we simply split
   * the required chunk size from the top of it.
   */

  DIAG_INC( arena->cnt_split);
  SAC_HM_LARGECHUNK_SIZE(bestp) -= units;
  freep = bestp + SAC_HM_LARGECHUNK_SIZE(bestp);
  SAC_HM_LARGECHUNK_SIZE(freep) = units;
  SAC_HM_LARGECHUNK_ARENA(freep) = arena;
  SAC_HM_LARGECHUNK_PREVSIZE(freep) = SAC_HM_LARGECHUNK_SIZE(bestp);
  SAC_HM_LARGECHUNK_PREVSIZE(freep + units) = -1;
  DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep);

  return((void *)(freep+2));
}
#endif
