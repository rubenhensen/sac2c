/*
 *
 * $Log$
 * Revision 1.6  1999/12/17 14:05:47  cg
 * Added substantial documentation about how the heap manager works.
 * Fixed a bug in MallocLargeChunk(), the free list is now correctly
 * coalasced.
 *
 * Revision 1.5  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
 * Revision 1.4  1999/07/29 07:35:41  cg
 * Two new performance related features added to SAC private heap
 * management:
 *   - pre-splitting for arenas with fixed size chunks.
 *   - deferred coalascing for arenas with variable chunk sizes.
 *
 * Revision 1.3  1999/07/16 09:41:16  cg
 * Added facilities for heap management diagnostics.
 *
 * Revision 1.2  1999/07/09 07:35:08  cg
 * Some bugs fixed.
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
 *   This file contains the basic routines and data structures of a SAC
 *   specific heap manager.
 *
 *
 * arenas:
 *
 *   The heap is organized in so-called arenas. An arena is an organizational
 *   entity for requested heap chunks of a certain size range. Each arena is
 *   associated with a minimum and maximum chunk size, a bin size, an allocation
 *   and de-allocation function, and a free list. A characteristic of this free
 *   list is that it is never empty; it always has a first dummy entry with size 0.
 *   This allows for a more efficient implementation of the free list handling.
 *
 *   There are 4 different variations of arenas:
 *    - small chunk arenas,
 *    - large chunk arenas,
 *    - top arena,
 *    - arena of arenas.
 *
 *   In a small chunk arena, each chunk of memory has the same size which is
 *   characteristic for the arena. This allows fast allocation/de-allocation
 *   strategies. However, it is only feasible for rather small memory requests
 *   because it implies significant internal memory fragmentation.
 *
 *   In large chunk arenas, each allocated chunk of memory is associated with
 *   its size, i.e. different chunk sizes within the range of the given
 *   arena may occur.
 *
 *   The top arena basically is a large chunk arena. It satisfies the
 *   class of largest memory requests. The top arena, however, is the only
 *   arena which may arbitrarily be extended by requesting more memory from the
 *   operating system.
 *
 *   Large and small chunk arenas are organized as subsequently allocated bins of
 *   contiguous memory. These bins are allocated in the so-called arena of arenas.
 *   The bins of the arena of arenas itself are allocated within the top arena.
 *   Note that bins are never de-allocated. The reason is that it is too costly
 *   to identify completely empty bins.
 *
 *
 * bins:
 *
 *   Except for the top arena, arenas are organized in bins of contiguous
 *   memory. The first time, a memory request for a certain arena occurs,
 *   a bin is allocated within the arena of arenas. Each time a new request
 *   cannot be satisfied within the already allocated bins of the arena,
 *   a new bin is allocated.
 *
 *
 * initialization:
 *
 *   When the heap manager is initialized upon program startup, a certain
 *   configurable amount of memory is requested from the operating system
 *   and the following memory layout is established:
 *
 *     |                  |
 *     +------------------+
 *     |                  |
 *     |  arena of arenas |
 *     |                  |
 *     +------------------+
 *     |                  |
 *     |  top arena       |
 *     |                  |
 *     +------------------+
 *     |                  |
 *
 *   The default sizes are 1MB each for the first bin of the arena of arenas
 *   and the top arena.
 *
 *
 * administration data layout:
 *
 *   Memory is always allocated in so-called units (currently sizeof(double)).
 *   This assures proper alignment as required by malloc().
 *
 *   Each separately indentifiable chunk of memory (allocated or de-allocated)
 *   is associated with some administrative information:
 *
 *   Small chunks:
 *
 *     |                  |
 *     +==================+
 *     | size/diag        |
 *     +------------------+
 *     | arena_ptr        |
 *     +==================+
 *     | nextfree_ptr     |     <-- returned pointer
 *     +------------------+
 *     |                  |
 *     +==================+
 *     |                  |
 *
 *   The size field conatains the size of the memory chunk in units. This
 *   information does exclusively exist for those chunks that are not of the
 *   fixed chunk size of the small chunk arena. For example, each time a new
 *   bin is allocated it is kept as a whole. Subsequently small chunks of the
 *   fixed chunk size are split from this. These small fixed size chunks are
 *   never coalasced again and don't have size information themselves.
 *   The variable sized initial chunk in a small chunk arena is called a
 *   wilderness chunk.
 *
 *   The arena_ptr is a pointer back to the arena data structure which contains
 *   all the characteristic information about the arena. This pointer allows
 *   a quick determination of the free list and the de-allocation function
 *   upon a de-allocation request for a specific memory chunk (free).
 *
 *   The nextfree_ptr points to the next entry of the free list. This is only
 *   available when the chunk is currently un-allocated. Otherwise the memory
 *   cell belongs to the memory returned by the allocator. This technique helps
 *   to minimize the administrative overhead in terms of memory consumption.
 *
 *   Split chunks (of the fixed chunk size) use the size field for storage of a
 *   diagnostic pattern iff diagnostic heap management is activated. This pattern
 *   allows to detect certain forms of corruptions of the internal heap manager
 *   data structures caused by programs that write over the allocated amount of
 *   memory.
 *
 *   Large chunks:
 *
 *     |                  |
 *     +==================+
 *     | prevsize         |
 *     +------------------+
 *     | diag             |
 *     +==================+
 *     | size             |
 *     +------------------+
 *     | arena_ptr        |
 *     +==================+
 *     | nextfree_ptr     |     <-- returned pointer
 *     +------------------+
 *     |                  |
 *     +==================+
 *     |                  |
 *
 *   In contrast to the field entries of a small chunk administration block, a large
 *   chunk administration block always contains the diagnostic pattern as well as
 *   the chunk size in units.
 *
 *   Additionally the size of the preceding chunk (in units) is stored. A negative
 *   number in this field means that the preceding chunk is currently allocated.
 *   This information is needed for efficient coalascing of neighboring un-allocated
 *   memory chunks.
 *
 *
 *
 *****************************************************************************/

#include <unistd.h>
#include <string.h>

#include "heapmgr.h"
#include "sac_message.h"

/*
 * Definition and initialization of global array of arena descriptions.
 */

#define SMALL_ARENA_BASE(n)                                                              \
    {                                                                                    \
        {{DIAG_FREEPATTERN, &(SAC_HM_arenas[n])}}, {{0, NULL}},                          \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#define LARGE_ARENA_BASE(n)                                                              \
    {                                                                                    \
        {{-1, (void *)DIAG_FREEPATTERN}}, {{0, &(SAC_HM_arenas[n])}},                    \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#ifdef DIAG
#define DIAG_COUNTERS , 0, 0, 0, 0, 0, 0
#else
#define DIAG_COUNTERS
#endif

#define ARENA_OF_SMALL_CHUNKS(n, size, mincs, freefun)                                   \
    {                                                                                    \
        n, SMALL_ARENA_BASE (n), size, mincs, SAC_HM_arenas[n].freelist,                 \
          &freefun DIAG_COUNTERS                                                         \
    }

#define ARENA_OF_LARGE_CHUNKS(n, size, mincs, freefun)                                   \
    {                                                                                    \
        n, LARGE_ARENA_BASE (n), size, mincs, SAC_HM_arenas[n].freelist,                 \
          &freefun DIAG_COUNTERS                                                         \
    }

SAC_HM_arena_t SAC_HM_arenas[NUM_ARENAS]
  = {ARENA_OF_SMALL_CHUNKS (0, 131072, 128, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (1, 512, ARENA_2_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (2, 512, ARENA_3_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (3, 256, ARENA_4_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (4, 512, ARENA_5_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_LARGE_CHUNKS (5, 2048, ARENA_5_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (6, 8192, ARENA_6_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (7, 32768, ARENA_7_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (8, 0, ARENA_8_MINCS, SAC_HM_FreeLargeChunk)};

/*
 * Some global variables which can only be initialized at runtime.
 */

size_byte_t SAC_HM_pagesize;

static size_byte_t units_per_page;
static char *initial_break_val;

/******************************************************************************
 *
 * function:
 *   void OutOfMemory( size_byte_t request)
 *
 * description:
 *
 *   This function displays an error message and aborts program execution
 *   whenever a request for memory cannot be satisfied by the heap manager.
 *
 ******************************************************************************/

static void
OutOfMemory (size_byte_t request)
{
    SAC_RuntimeError ("SAC heap manager failed to obtain %lu Bytes of memory "
                      "from operating system !",
                      request);
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_Setup(size_byte_t initial_arena_of_arenas_size,
 *                     size_byte_t initial_top_arena_size)
 *
 * description:
 *
 *   This function initializes the internal data structures of the heap
 *   manager. The arena of arenas as well as the top arena are allocated
 *   and initialized immediately; their initial sizes given in MB are
 *   provided as  arguments.
 *
 *   Particular care is taken of the correct alignment of these inital
 *   heap structures.
 *
 ******************************************************************************/

void
SAC_HM_Setup (size_byte_t initial_arena_of_arenas_size,
              size_byte_t initial_top_arena_size)
{
    size_byte_t offset, initial_heap_size;
    SAC_HM_header_t *freep;
    char *mem;

    /*
     * Initialize some global variables.
     */
    SAC_HM_pagesize = getpagesize ();
    units_per_page = SAC_HM_pagesize / UNIT_SIZE;
    initial_break_val = (char *)sbrk (0);
    /* check for start of heap */

    /*
     * Prepare initial request for memory from operating system.
     */

    initial_arena_of_arenas_size *= MB;
    initial_top_arena_size *= MB;

    offset = ((size_byte_t)initial_break_val) % SAC_HM_pagesize;
    /* Determine offset from last memory page boundary. */

    if (offset != 0) {
        offset = SAC_HM_pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start. */
    }

    /* Compute initial heap size requested from operating system. */
    initial_heap_size = offset + initial_arena_of_arenas_size + initial_top_arena_size;

    mem = (char *)sbrk (initial_heap_size);
    if (mem == (char *)-1) {
        OutOfMemory (initial_heap_size);
    }

    mem += offset;

    DIAG_SET (SAC_HM_call_sbrk, 2);
    DIAG_SET (SAC_HM_heapsize, initial_heap_size);

    /*
     * Allocate arena of arenas.
     */

    if (initial_arena_of_arenas_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        SMALLCHUNK_SIZE (freep) = initial_arena_of_arenas_size / UNIT_SIZE;
        SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[ARENA_OF_ARENAS]);

        SMALLCHUNK_NEXTFREE (freep) = NULL;
        SMALLCHUNK_NEXTFREE (SAC_HM_arenas[ARENA_OF_ARENAS].freelist) = freep;

        DIAG_SET (SAC_HM_arenas[ARENA_OF_ARENAS].size, initial_arena_of_arenas_size);
        DIAG_SET (SAC_HM_arenas[ARENA_OF_ARENAS].bins, 1);

        mem += initial_arena_of_arenas_size;
    } else {
        DIAG_SET (SAC_HM_arenas[ARENA_OF_ARENAS].size, 0);
        DIAG_SET (SAC_HM_arenas[ARENA_OF_ARENAS].bins, 0);
    }

    /*
     * Allocate top arena.
     */

    if (initial_top_arena_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        LARGECHUNK_SIZE (freep) = initial_top_arena_size / UNIT_SIZE;
        LARGECHUNK_ARENA (freep) = &(SAC_HM_arenas[TOP_ARENA]);
        LARGECHUNK_PREVSIZE (freep) = -1;
        DIAG_SET_FREEPATTERN_LARGECHUNK (freep);

        SAC_HM_arenas[TOP_ARENA].wilderness = freep;

        DIAG_SET (SAC_HM_arenas[TOP_ARENA].size, initial_top_arena_size);
        DIAG_SET (SAC_HM_arenas[TOP_ARENA].bins, 1);
    } else {
        DIAG_SET (SAC_HM_arenas[TOP_ARENA].size, 0);
        DIAG_SET (SAC_HM_arenas[TOP_ARENA].bins, 0);
    }
}

/******************************************************************************
 *
 * function:
 *   SAC_HM_header_t *AllocateNewBinInArenaOfArenas(size_unit_t units)
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

static SAC_HM_header_t *
AllocateNewBinInArenaOfArenas (size_unit_t units)
{
    SAC_HM_header_t *freep, *lastp;
    SAC_HM_arena_t *arena;
    size_unit_t split_threshold;

    DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].cnt_alloc);

    arena = &(SAC_HM_arenas[ARENA_OF_ARENAS]);
    split_threshold = units + arena->min_chunk_size;

    /*
     * Search for sufficiently large chunk of memory in the free list.
     */

    lastp = arena->freelist;
    freep = SMALLCHUNK_NEXTFREE (lastp);

    while (freep != NULL) {

        if (SMALLCHUNK_SIZE (freep) >= split_threshold) {
            /*
             * The current chunk of memory is larger than required, so split
             * the amount of memory needed from the top of the chunk.
             */
            DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].cnt_split);
            SMALLCHUNK_SIZE (freep) -= units;
            return (freep + SMALLCHUNK_SIZE (freep));
        }

        if (SMALLCHUNK_SIZE (freep) >= units) {
            /*
             * The current chunk of memory more or less fits exactly, so remove it
             * from free list and return it to the calling context.
             */
            SMALLCHUNK_NEXTFREE (lastp) = SMALLCHUNK_NEXTFREE (freep);
            return (freep);
        }

        /*
         *  The current chunk of memory is too small, so continue with the next one.
         */
        lastp = freep;
        freep = SMALLCHUNK_NEXTFREE (freep);
    }

    /*
     * No sufficient space found in free list, so, we have to allocate
     * an additional bin for the arena of arenas within the top arena.
     * We will only return to this point if the additional allocation succeeds.
     */

    freep = (SAC_HM_header_t *)
      SAC_HM_MallocLargeChunk (SAC_HM_arenas[ARENA_OF_ARENAS].arena_size + 2,
                               &(SAC_HM_arenas[TOP_ARENA]));

    DIAG_ADD (SAC_HM_arenas[ARENA_OF_ARENAS].size,
              SAC_HM_arenas[ARENA_OF_ARENAS].arena_size * UNIT_SIZE);
    DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].bins);

    SMALLCHUNK_SIZE (freep) = arena->arena_size - units;

    SMALLCHUNK_NEXTFREE (freep) = SMALLCHUNK_NEXTFREE (arena->freelist);
    SMALLCHUNK_NEXTFREE (arena->freelist) = freep;

    return (freep + SMALLCHUNK_SIZE (freep));
}

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocSmallChunk( size_unit_t units, SAC_HM_arena_t *arena)
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
SAC_HM_MallocSmallChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *wilderness;

    DIAG_INC (arena->cnt_alloc);

    /*
     * Search for chunk of memory in the free list.
     */

    freep = SMALLCHUNK_NEXTFREE (arena->freelist);

    if (freep != NULL) {
        /*
         * Any entry in the free list will exactly fit our needs, so we may simply
         * take the first one provided that the free list is not empty.
         * The first entry is simply removed from the free list and returned to the
         * calling context.
         */
        DIAG_CHECK_FREEPATTERN_SMALLCHUNK (freep, arena->num);
        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

        SMALLCHUNK_NEXTFREE (arena->freelist) = SMALLCHUNK_NEXTFREE (freep);

        return ((void *)(freep + 1));
    }

    /*
     * There has been no entry in the free list,
     * so try to split memory from the arena큦 wilderness chunk.
     */

    wilderness = arena->wilderness;

    if (SMALLCHUNK_SIZE (wilderness) > units) {
        /*
         * The wilderness chunk is sufficiently large to satisfy the needs,
         * so split a small chunk from its top and return it to the calling
         * context.
         */
        DIAG_INC (arena->cnt_split);

        SMALLCHUNK_SIZE (wilderness) -= units;
        freep = wilderness + SMALLCHUNK_SIZE (wilderness);
        SMALLCHUNK_ARENA (freep) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

        return ((void *)(freep + 1));
    }

    if (SMALLCHUNK_SIZE (wilderness) == units) {
        /*
         * The wilderness chunk exactly satisfies the needs,
         * so return it to the calling context and disable wilderness
         * chunk in arena representation.
         */
        SMALLCHUNK_ARENA (wilderness) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (wilderness);

        arena->wilderness = arena->freelist;

        return ((void *)(wilderness + 1));
    }

    /*
     * There has also been not enough space in the wilderness chunk,
     * so we have to allocate a new bin for the given arena and use this as
     * the arena큦 new wilderness chunk.
     * Afterwards, the reqired chunk of memory is cut from the top of the
     * new wilderness chunk and returned to the calling context.
     */

    wilderness = AllocateNewBinInArenaOfArenas (arena->arena_size);

    DIAG_INC (arena->bins);
    DIAG_ADD (arena->size, arena->arena_size * UNIT_SIZE);
    DIAG_INC (arena->cnt_split);

    SMALLCHUNK_SIZE (wilderness) = arena->arena_size - units;
    arena->wilderness = wilderness;
    freep = wilderness + SMALLCHUNK_SIZE (wilderness);
    SMALLCHUNK_ARENA (freep) = arena;

    DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

    return ((void *)(freep + 1));
}

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

void *
SAC_HM_MallocSmallChunkPresplit (size_unit_t units, SAC_HM_arena_t *arena, int presplit)
{
    SAC_HM_header_t *freep, *wilderness, *lastfreep, *firstfreep;

    DIAG_INC (arena->cnt_alloc);

    /*
     * Search for chunk of memory in the free list.
     */

    freep = SMALLCHUNK_NEXTFREE (arena->freelist);

    if (freep != NULL) {
        /*
         * Any entry in the free list will exactly fit our needs, so we may simply
         * take the first one provided that the free list is not empty.
         * The first entry is simply removed from the free list and returned to the
         * calling context.
         */
        DIAG_CHECK_FREEPATTERN_SMALLCHUNK (freep, arena->num);
        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

        SMALLCHUNK_NEXTFREE (arena->freelist) = SMALLCHUNK_NEXTFREE (freep);

        return ((void *)(freep + 1));
    }

    /*
     * There has been no entry in the free list,
     * so try to split memory from the arena큦 wilderness chunk.
     */

    wilderness = arena->wilderness;

    if (SMALLCHUNK_SIZE (wilderness) < units) {
        /*
         * The wilderness is empty, so allocate new wilderness from arena of arenas.
         */
        wilderness = AllocateNewBinInArenaOfArenas (arena->arena_size);

        DIAG_INC (arena->bins);
        DIAG_ADD (arena->size, arena->arena_size * UNIT_SIZE);

        SMALLCHUNK_SIZE (wilderness) = arena->arena_size;
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

    DIAG_INC (arena->cnt_split);

    lastfreep = wilderness + SMALLCHUNK_SIZE (wilderness) - (2 * units);
    SMALLCHUNK_SIZE (wilderness) -= units * presplit;
    firstfreep = wilderness + SMALLCHUNK_SIZE (wilderness);

    SMALLCHUNK_NEXTFREE (arena->freelist) = firstfreep;

    for (freep = firstfreep; freep != lastfreep; freep += units) {
        SMALLCHUNK_NEXTFREE (freep) = freep + units;
        SMALLCHUNK_ARENA (freep) = arena;
        DIAG_SET_FREEPATTERN_SMALLCHUNK (freep);
    }

    SMALLCHUNK_NEXTFREE (lastfreep) = NULL;
    SMALLCHUNK_ARENA (lastfreep) = arena;
    DIAG_SET_FREEPATTERN_SMALLCHUNK (lastfreep);

    /*
     * Maybe the wilderness chunk is empty after pre-splitting; in this
     * case, it is removed from the arena큦 representation.
     */

    if (firstfreep == wilderness) {
        arena->wilderness = arena->freelist;
    }

    /*
     * Eventually, the topmost pre-splitted chunk of memory is initalized
     * and returned to the calling context.
     */

    freep = lastfreep + units;

    SMALLCHUNK_ARENA (freep) = arena;
    DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

    return ((void *)(freep + 1));
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
  freep = LARGECHUNK_NEXTFREE(lastp);
  
  while (freep != NULL) {
    DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena->num);
    
    if (LARGECHUNK_SIZE(freep) < units) {
      /*
       *  The current chunk of memory is too small, so continue with the next one.
       */
      lastp = freep;
      freep = LARGECHUNK_NEXTFREE(freep);
      continue;
    }
    
    if (LARGECHUNK_SIZE(freep) >= split_threshold) {
      /*
       * The current chunk of memory is larger than required, so remember it for
       * potential later use and continue with the next one.
       */
      bestp = freep;
      lastp = freep;
      freep = LARGECHUNK_NEXTFREE(freep);
      continue;
    }
    
    /*
     * The current chunk of memory more or less fits exactly, so remove it
     * from the free list, mark it as being allocated, and return it to the 
     * calling context.
     */
    LARGECHUNK_NEXTFREE( lastp) = LARGECHUNK_NEXTFREE( freep);

    LARGECHUNK_PREVSIZE(freep + LARGECHUNK_SIZE(freep)) = -1;
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

    LARGECHUNK_SIZE(bestp) = arena->arena_size - 2;
    LARGECHUNK_PREVSIZE(bestp) = -1;
    LARGECHUNK_ARENA(bestp) = arena;
  
    LARGECHUNK_NEXTFREE(bestp) = LARGECHUNK_NEXTFREE(arena->freelist);
    LARGECHUNK_NEXTFREE(arena->freelist) = bestp;

    /* LARGECHUNK_SIZE(bestp + LARGECHUNK_SIZE(bestp)) = -1; */
  }
  

  /*
   * Now, we do have a sufficiently large amount of memory, so we simply split
   * the required chunk size from the top of it.
   */

  DIAG_INC( arena->cnt_split);
  LARGECHUNK_SIZE(bestp) -= units;
  freep = bestp + LARGECHUNK_SIZE(bestp);
  LARGECHUNK_SIZE(freep) = units;
  LARGECHUNK_ARENA(freep) = arena;
  LARGECHUNK_PREVSIZE(freep) = LARGECHUNK_SIZE(bestp);
  LARGECHUNK_PREVSIZE(freep + units) = -1;
  DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep);

  return((void *)(freep+2));
}
#endif

/******************************************************************************
 *
 * function:
 *   SAC_HM_header_t *ExtendTopArenaWilderness( size_unit_t units,
 *                                              SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function allows to extend the top arena큦 wilderness chunk by
 *   "fresh" memory obtained from the operating system.
 *
 *
 *
 *
 *
 ******************************************************************************/

static SAC_HM_header_t *
ExtendTopArenaWilderness (size_unit_t units, SAC_HM_arena_t *arena)
{
    size_unit_t new_mem;
    char *mem;
    SAC_HM_header_t *wilderness;

    wilderness = arena->wilderness;

    /*
     * First, the amount of additionally required memory is computed, i.e.
     * the requested amount of memory decreased by the amount of memory still
     * available in the wilderness chunk. The result then is rounded up to
     * the next multiple of SBRK_CHUNK. This guarantees that memory is requested
     * from the operating system in sufficiently large chunks since calls to
     * the system call sbrk() are usually expensive.
     */
    new_mem = ((units - LARGECHUNK_SIZE (wilderness) + 3) * UNIT_SIZE);
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
        OutOfMemory (new_mem);
    }

    DIAG_ADD (SAC_HM_heapsize, new_mem);
    DIAG_ADD (arena->size, new_mem);
    DIAG_INC (arena->bins);

    if ((SAC_HM_header_t *)mem == wilderness + LARGECHUNK_SIZE (wilderness)) {
        /*
         * The new memory and the old wilderness chunk are contiguous.
         * This should always be the case for single-threaded operation or as
         * long as only one thread has control over the top arena.
         * As a consequence the old wilderness chunk may simply be extended.
         */
        LARGECHUNK_SIZE (wilderness) += new_mem / UNIT_SIZE;
        return (wilderness);
    } else {
        /*
         * New memory and old wilderness chunk are NOT contiguous.
         * The old wilderness becomes regular free chunk; the request will be
         * satisfied from freshly allocated memory alone.
         */
        LARGECHUNK_NEXTFREE (wilderness) = LARGECHUNK_NEXTFREE (arena->freelist);
        LARGECHUNK_NEXTFREE (arena->freelist) = wilderness;

        if (new_mem >= units * UNIT_SIZE) {
            /*
             * The freshly allocated memory suffices to satisfy request,
             * so make it the new wilderness chunk.
             */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            LARGECHUNK_SIZE (wilderness) = new_mem / UNIT_SIZE;
            LARGECHUNK_PREVSIZE (wilderness) = -1;

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
                OutOfMemory (more_mem);
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
            LARGECHUNK_SIZE (wilderness) = (new_mem + more_mem) / UNIT_SIZE;
            LARGECHUNK_PREVSIZE (wilderness) = -1;
            DIAG_SET_FREEPATTERN_LARGECHUNK (wilderness);

            return (wilderness);
        }
    }
}

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocLargeChunk( size_unit_t units,
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
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *bestp, *lastp, *wilderness, *new_wilderness;
    SAC_HM_header_t *prevp;

    size_unit_t split_threshold;

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;
    bestp = NULL;

    /*
     * Search for sufficiently large chunk of memory in the free list.
     */

    lastp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (lastp);

    while (freep != NULL) {
        DIAG_CHECK_FREEPATTERN_LARGECHUNK (freep, arena->num);

        if (LARGECHUNK_SIZE (freep) < units) {
            /*
             *  The current chunk of memory is too small, so continue with the next one.
             */
            lastp = freep;
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        if (LARGECHUNK_SIZE (freep) >= split_threshold) {
            /*
             * The current chunk of memory is larger than required, so remember it for
             * potential later use and continue with the next one.
             */
            bestp = freep;
            lastp = freep;
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /*
         * The current chunk of memory more or less fits exactly, so remove it
         * from the free list and return it to the calling context.
         */
    exact_fit:
        LARGECHUNK_NEXTFREE (lastp) = LARGECHUNK_NEXTFREE (freep);

        LARGECHUNK_PREVSIZE (freep + LARGECHUNK_SIZE (freep)) = -1;
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
    split:
        DIAG_INC (arena->cnt_split);

        LARGECHUNK_SIZE (bestp) -= units;
        freep = bestp + LARGECHUNK_SIZE (bestp);
        LARGECHUNK_SIZE (freep) = units;
        LARGECHUNK_ARENA (freep) = arena;
        LARGECHUNK_PREVSIZE (freep) = LARGECHUNK_SIZE (bestp);
        LARGECHUNK_PREVSIZE (freep + units) = -1;

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

    if (LARGECHUNK_SIZE (wilderness) >= units + 3) {
        /*
         * The wilderness chunk is sufficiently large, so split the requested
         * amount of memory from the bottom of the wilderness chunk.
         * This technique is slightly less efficient than splitting from the top.
         * However, in the case of the top arena큦 wilderness chunk this allows
         * to extend the wilderness subsequently without unnecessary fragmentation.
         */
    split_wilderness:
        new_wilderness = wilderness + units;
        LARGECHUNK_SIZE (new_wilderness) = LARGECHUNK_SIZE (wilderness) - units;
        LARGECHUNK_PREVSIZE (new_wilderness) = -1;
        DIAG_SET_FREEPATTERN_LARGECHUNK (new_wilderness);
        arena->wilderness = new_wilderness;

        LARGECHUNK_SIZE (wilderness) = units;
        LARGECHUNK_ARENA (wilderness) = arena;
        DIAG_INC (arena->cnt_split);
        DIAG_SET_ALLOCPATTERN_LARGECHUNK (wilderness);

        return ((void *)(wilderness + 2));
    }

    /*
     * There is no sufficient space in the wilderness chunk, so we now coalasce
     * coalascable chunks until a sufficiently large free chunk has been
     * created or until all chunks are coalasced as far as possible.
     */

    lastp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (lastp);

    while (freep != NULL) {
        if (LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * The previous adjacent chunk to the current chunk is also free,
             * so coalasce the two and remove the current chunk from the free list.
             */
            prevp = freep - LARGECHUNK_PREVSIZE (freep);
            LARGECHUNK_SIZE (prevp) += LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (freep + LARGECHUNK_SIZE (freep))
              = LARGECHUNK_SIZE (prevp);
            LARGECHUNK_NEXTFREE (lastp) = LARGECHUNK_NEXTFREE (freep);

            if (LARGECHUNK_SIZE (prevp) >= units) {
                if (LARGECHUNK_SIZE (freep) >= split_threshold) {
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
                    while (LARGECHUNK_NEXTFREE (lastp) != freep) {
                        lastp = LARGECHUNK_NEXTFREE (lastp);
                    }
                    goto exact_fit;
                }
            } else {
                freep = LARGECHUNK_NEXTFREE (freep);
                continue;
            }
        } else {
            lastp = freep;
            freep = LARGECHUNK_NEXTFREE (freep);
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

    if (LARGECHUNK_PREVSIZE (wilderness) > 0) {
        /*
         * The chunk adjacent to the wilderness chunk is currently free.
         */
        new_wilderness = wilderness - LARGECHUNK_PREVSIZE (wilderness);
        LARGECHUNK_SIZE (new_wilderness) += LARGECHUNK_SIZE (wilderness);

        /*
         * Next, the coalasced free chunk adjacent to the wilderness chunk must
         * be removed from the free list. Unfortunately, this requires a retraversal
         * the free list.
         */
        lastp = arena->freelist;
        while (LARGECHUNK_NEXTFREE (lastp) != new_wilderness) {
            lastp = LARGECHUNK_NEXTFREE (lastp);
        }
        LARGECHUNK_NEXTFREE (lastp) = LARGECHUNK_NEXTFREE (new_wilderness);

        wilderness = new_wilderness;
        arena->wilderness = wilderness;

        if (LARGECHUNK_SIZE (wilderness) >= units + 3) {
            /*
             * Now, the wilderness is large enough, so we directly split the requested
             * amount of memory from it.
             */
            goto split_wilderness;
        }
    }

    /*
     * We haven't found any chunk of sufficient size, so we either allocate a new
     * bin from the arena of arenas or we extend the wilderness chunk if the given
     * arena is the top arena.
     */

    if (arena->arena_size == 0) {
        /*
         * The given arena is the top arena, so we try to get new memory from the
         * operating system and extend the wilderness chunk.
         */

        wilderness = ExtendTopArenaWilderness (units, arena);
        goto split_wilderness;
    } else {
        /*
         * The given arena is not the top arena but a usual large chunk arena.
         * So, we allocate a new bin in the arena of arenas, initialize it as
         * one relatively large chunk of memory and insert it on top of the free
         * list.
         */

        bestp = AllocateNewBinInArenaOfArenas (arena->arena_size);

        DIAG_INC (arena->bins);
        DIAG_ADD (arena->size, arena->arena_size * UNIT_SIZE);
        DIAG_SET_FREEPATTERN_LARGECHUNK (bestp);

        LARGECHUNK_SIZE (bestp) = arena->arena_size;
        LARGECHUNK_PREVSIZE (bestp) = -1;
        LARGECHUNK_ARENA (bestp) = arena;

        LARGECHUNK_NEXTFREE (bestp) = LARGECHUNK_NEXTFREE (arena->freelist);
        LARGECHUNK_NEXTFREE (arena->freelist) = bestp;

        goto split;
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_FreeSmallChunk(SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
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

void
SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep;

    freep = addr - 1;

    DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_SMALLCHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SMALLCHUNK_NEXTFREE (freep) = SMALLCHUNK_NEXTFREE (arena->freelist);
    SMALLCHUNK_NEXTFREE (arena->freelist) = freep;
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

    LARGECHUNK_PREVSIZE (freep + LARGECHUNK_SIZE (freep)) = LARGECHUNK_SIZE (freep);

    LARGECHUNK_NEXTFREE (freep) = LARGECHUNK_NEXTFREE (arena->freelist);
    LARGECHUNK_NEXTFREE (arena->freelist) = freep;
}
