/*****************************************************************************
 *
 * file:  small_chunks.c
 *
 * prefix: SAC_HM
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

static inline void do_free_small_unused_chunks (SAC_HM_arena_t *arena);

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocSmallChunk( SAC_HM_size_unit_t units, SAC_HM_arena_t *arena)
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
SAC_HM_MallocSmallChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *wilderness;

    /* SAC_DO_HM_XTHR_FREE: free all the unused chunks */
    do_free_small_unused_chunks (arena);

    assert (units >= arena->min_chunk_size);
    DIAG_INC (arena->cnt_alloc);

    /*
     * Search for chunk of memory in the free list.
     */

    freep = SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist);

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

        SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist) = SAC_HM_SMALLCHUNK_NEXTFREE (freep);

        return ((void *)(freep + 1));
    }

    /*
     * There has been no entry in the free list,
     * so try to split memory from the arena's wilderness chunk.
     */

    wilderness = arena->wilderness;

    if (SAC_HM_SMALLCHUNK_SIZE (wilderness) > units) {
        /*
         * The wilderness chunk is sufficiently large to satisfy the needs,
         * so split a small chunk from its top and return it to the calling
         * context.
         */
        SAC_HM_SMALLCHUNK_SIZE (wilderness) -= units;
        freep = wilderness + SAC_HM_SMALLCHUNK_SIZE (wilderness);
        SAC_HM_SMALLCHUNK_ARENA (freep) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);
        DIAG_INC (arena->cnt_after_wilderness);

        return ((void *)(freep + 1));
    }

    if (SAC_HM_SMALLCHUNK_SIZE (wilderness) == units) {
        /*
         * The wilderness chunk exactly satisfies the needs,
         * so return it to the calling context and disable wilderness
         * chunk in arena representation.
         */
        SAC_HM_SMALLCHUNK_ARENA (wilderness) = arena;

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
      = SAC_HM_AllocateNewBinInArenaOfArenas (arena->binsize, arena - arena->num);

    DIAG_INC (arena->cnt_bins);
    DIAG_ADD (arena->size, arena->binsize * SAC_HM_UNIT_SIZE);
    DIAG_INC (arena->cnt_after_extension);

    SAC_HM_SMALLCHUNK_SIZE (wilderness) = arena->binsize - units;
    arena->wilderness = wilderness;
    freep = wilderness + SAC_HM_SMALLCHUNK_SIZE (wilderness);
    SAC_HM_SMALLCHUNK_ARENA (freep) = arena;

    DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

    return ((void *)(freep + 1));
}

#if SAC_DO_HM_XTHR_FREE
/* Internal helper function */
/* Atomically prepend the freep header at the beginning of the unused list
 * in the arena. */
static inline void
push_smallchunk_to_arena_unused_list (SAC_HM_header_t *freep, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *their_list;
    do {
        /* get the current head of the list */
        their_list = (SAC_HM_header_t *)arena->unused_list;
        /* freep becomes the new first element of the list */
        SAC_HM_SMALLCHUNK_NEXTFREE (freep) = their_list;
        /* atomically swap the old list head (their_list) with the new head (freep).
         * The __sync function returns false if the swap has been unsuccessful,
         * indicating an interference from other thread, hence re-try in the case. */
    } while (!__sync_bool_compare_and_swap (&arena->unused_list, their_list, freep));
}

#endif

/******************************************************************************
 *
 * function:
 *   void do_free_small_chunk(SAC_HM_header_t *freep, SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function de-allocates memory chunks in arenas of small chunks.
 *   Free chunks are simply inserted into the free list. There is no coalascing of
 *   adjacent free chunks because such arenas contain only chunks of equal
 *   size.
 *
 *
 *
 ******************************************************************************/

/* precondition: current thread == arena thread */
static inline void
do_free_small_chunk (SAC_HM_header_t *freep, SAC_HM_arena_t *arena)
{
    DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_SMALLCHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SAC_HM_SMALLCHUNK_NEXTFREE (freep) = SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist);
    SAC_HM_SMALLCHUNK_NEXTFREE (arena->freelist) = freep;
}

void
SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    /* addr points at the first user byte of the chunk ("returned pointer").
     * freep points at the proper header start of the small chunks.
     * nextfree is a field already in the user part, hence we may overwrite/reuse
     * it here at will. */
    SAC_HM_header_t *freep = addr - 1;

#if 0
  /* write 0xdeadbeef over the chunk data */
  memset_words(addr, 0xdeadbeef,
              (SAC_HM_SMALLCHUNK_ARENA( freep)->min_chunk_size - 1) * (SAC_HM_UNIT_SIZE/sizeof(unsigned)));
#endif

#if SAC_DO_HM_XTHR_FREE
    /* note: current thread is not necessarily the arena thread */
    /* push the chunk to the unused list */
    push_smallchunk_to_arena_unused_list (freep, arena);
#else  /* SAC_DO_HM_XTHR_FREE */
    /* precondition: current thread == arena thread */
    /* directly free the chunk */
    do_free_small_chunk (freep, arena);
#endif /* SAC_DO_HM_XTHR_FREE */
}

/******************************************************************************
 *
 * function:
 *   void do_free_small_unused_chunks(SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   Free all chunks in the unused list of the arena.
 *
 ******************************************************************************/

#if SAC_DO_HM_XTHR_FREE

/* precondition: current thread == arena thread */
static inline void
do_free_small_unused_chunks (SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *unused = grab_arena_unused_list (arena);

    while (unused) {
        /* remember the next in the list as the pointer will be overwriten in the freeing
         * process */
        SAC_HM_header_t *next = SAC_HM_SMALLCHUNK_NEXTFREE (unused);
        /* free the unused chunk */
        do_free_small_chunk (unused, arena);
        /* move on */
        unused = next;
    }
}

#else

static inline void
do_free_small_unused_chunks (SAC_HM_arena_t *arena)
{
    /* nothing */
    assert (!arena->unused_list
            && "arena->unused_list shouldn't be used. "
               "(Have you tried recompiling the stdlib and your code?)");
}

#endif

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocSmallChunkPresplit( size_unit_t units,
 *                                          SAC_HM_arena_t *arena,
 *                                          int presplit)
 *
 * description:
 *
 *   This function allocates a chunk of memory with the size given in units.
 *   The memory is allocated from the given arena of small chunks.
 *
 *   In contrast to the previous function, a so-called pre-splitting is
 *   performed, i.e., instead of splitting one new chunk of memory from the
 *   wilderness chunk, a certain number of chunks is split immediately and
 *   put into the free list.
 *
 *   The number of chunks split from the wilderness chunk at once are specified
 *   by an additional parameter. The minimum value for this parameter is 2 !
 *   This results in one additional chunk of memory to be split from the
 *   wilderness chunk and to be put into the free list for subsequent
 *   allocation.
 *
 ******************************************************************************/

#if 0
/* Presplitting is currently deactivated. */

void *SAC_HM_MallocSmallChunkPresplit( size_unit_t units, 
                                       SAC_HM_arena_t *arena,
                                       int presplit)
{
   SAC_HM_header_t *freep, *wilderness, *lastfreep, *firstfreep;
   
   DIAG_INC( arena->cnt_alloc);
   
  /*
   * Search for chunk of memory in the free list.
   */

   freep = SAC_HM_SMALLCHUNK_NEXTFREE( arena->freelist);

   if (freep != NULL) {
     /*
      * Any entry in the free list will exactly fit our needs, so we may simply
      * take the first one provided that the free list is not empty.
      * The first entry is simply removed from the free list and returned to the
      * calling context.
      */
     DIAG_CHECK_FREEPATTERN_SMALLCHUNK(freep, arena->num);
     DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep);
     
     SAC_HM_SMALLCHUNK_NEXTFREE( arena->freelist) = SAC_HM_SMALLCHUNK_NEXTFREE(freep);

     return((void*) (freep+1));
   }
   

  /* 
   * There has been no entry in the free list, 
   * so try to split memory from the arena's wilderness chunk.
   */

  wilderness = arena->wilderness;
  
  if (SAC_HM_SMALLCHUNK_SIZE(wilderness) < units) {
    /*
     * The wilderness is empty, so allocate new wilderness from arena of arenas.
     */
    wilderness = AllocateNewBinInArenaOfArenas( arena->arena_size);

    DIAG_INC( arena->bins);
    DIAG_ADD( arena->size, arena->arena_size * UNIT_SIZE);

    SAC_HM_SMALLCHUNK_SIZE(wilderness) = arena->arena_size;
    arena->wilderness = wilderness;
  }
  

  /* 
   * Now, there is sufficient space in the wilderness chunk, 
   * so split a prespecified number of equally sized chunks of memory 
   * from theE top of the wilderness chunk.
   *
   * The size of the wilderness chunk is decreased by <presplit> units.
   * The first <presplit>-1 chunks are initialized and put into the free
   * list while the last chunk is returned to the calling context.
   */
    
  DIAG_INC( arena->cnt_split);
    
  lastfreep = wilderness + SAC_HM_SMALLCHUNK_SIZE(wilderness) - (2 * units);
  SAC_HM_SMALLCHUNK_SIZE(wilderness) -= units * presplit;
  firstfreep = wilderness + SAC_HM_SMALLCHUNK_SIZE(wilderness);

  SAC_HM_SMALLCHUNK_NEXTFREE(arena->freelist) = firstfreep;

  for (freep = firstfreep;
       freep != lastfreep;
       freep += units) {
    SAC_HM_SMALLCHUNK_NEXTFREE(freep) = freep + units;
    SAC_HM_SMALLCHUNK_ARENA(freep) = arena;
    DIAG_SET_FREEPATTERN_SMALLCHUNK(freep);
  }
    
  SAC_HM_SMALLCHUNK_NEXTFREE(lastfreep) = NULL;
  SAC_HM_SMALLCHUNK_ARENA(lastfreep) = arena;
  DIAG_SET_FREEPATTERN_SMALLCHUNK(lastfreep);


  /*
   * Maybe the wilderness chunk is empty after pre-splitting; in this
   * case, it is removed from the arena's representation.
   */

  if (firstfreep == wilderness) {
    arena->wilderness = arena->freelist;
  }


  /*
   * Eventually, the topmost pre-splitted chunk of memory is initalized
   * and returned to the calling context.
   */
    
  freep = lastfreep + units;
  
  SAC_HM_SMALLCHUNK_ARENA(freep) = arena;
  DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep);

  return((void*) (freep+1));
}

#endif /* 0 */
