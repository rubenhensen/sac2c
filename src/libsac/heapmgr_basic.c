/*
 *
 * $Log$
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
 *
 *
 *
 *
 *
 *****************************************************************************/

#include <unistd.h>
#include <string.h>
#ifdef DIAG
#include <stdio.h>
#endif

#include "heapmgr.h"
#include "sac_message.h"

#define KB 1024
#define MB (KB * KB)
#define SBRK_CHUNK (MB)

#define ARENA_BASE(n)                                                                    \
    {                                                                                    \
        {{0, NULL}}, {{0, &(SAC_HM_arenas[n])}},                                         \
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
        n, ARENA_BASE (n), size, mincs, (SAC_HM_arenas[n].base) + 1,                     \
          (SAC_HM_arenas[n].base) + 1, &freefun DIAG_COUNTERS                            \
    }

#define ARENA_OF_LARGE_CHUNKS(n, size, mincs, freefun)                                   \
    {                                                                                    \
        n, ARENA_BASE (n), size, mincs, SAC_HM_arenas[n].base, SAC_HM_arenas[n].base,    \
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
     ARENA_OF_LARGE_CHUNKS (8, 0, ARENA_8_MINCS, SAC_HM_FreeTopArena)};

#define ARENA_OF_ARENAS 0
#define TOP_ARENA (NUM_ARENAS - 1)

size_byte_t SAC_HM_pagesize;

static size_byte_t units_per_page;
static char *initial_break_val;

#ifdef DIAG
unsigned long int SAC_HM_call_sbrk = 0;
unsigned long int SAC_HM_call_malloc = 0;
unsigned long int SAC_HM_call_free = 0;
unsigned long int SAC_HM_call_realloc = 0;
unsigned long int SAC_HM_call_calloc = 0;
unsigned long int SAC_HM_call_valloc = 0;
unsigned long int SAC_HM_call_memalign = 0;
unsigned long int SAC_HM_heapsize = 0;

void
SAC_HM_CheckAllocPattern (size_unit_t diag, int arena_num)
{
    if (diag == DIAG_FREEPATTERN) {
        SAC_RuntimeError ("Tried to subsequently de-allocate heap location in arena %d",
                          arena_num);
    }

    if (diag != DIAG_ALLOCPATTERN) {
        SAC_RuntimeError ("Corrupted / missing heap administration data encountered "
                          "upon memory de-allocation in arena %d",
                          arena_num);
    }
}

void
SAC_HM_CheckFreePattern (size_unit_t diag, int arena_num)
{
    if (diag != DIAG_FREEPATTERN) {
        SAC_RuntimeError (
          "Corrupted free list encountered upon memory allocation in arena %d",
          arena_num);
    }
}

void
SAC_HM_CheckAllocPatternAnyChunk (SAC_HM_header_t *addr)
{
    if ((SMALLCHUNK_DIAG (addr - 1) != DIAG_ALLOCPATTERN)
        && (SMALLCHUNK_DIAG (addr - 1) != DIAG_FREEPATTERN)
        && (LARGECHUNK_DIAG (addr - 2) != DIAG_ALLOCPATTERN)
        && (LARGECHUNK_DIAG (addr - 2) != DIAG_FREEPATTERN)) {
        SAC_RuntimeError (
          "Corrupted free list encountered upon memory allocation in unknown arena");
    }
}

static void
ShowDiagnosticsForArena (int num, unsigned long int size, unsigned long int bins,
                         unsigned long int alloc, unsigned long int free,
                         unsigned long int split, unsigned long int coalasce)
{
    if (num == -1) {
        fprintf (stderr, "Total   :\n");
    } else {
        if (num == ARENA_OF_ARENAS) {
            fprintf (stderr, "Arena %d :  Arena of Arenas\n", num);
        } else {
            if (num < NUM_SMALLCHUNK_ARENAS) {
                fprintf (stderr, "Arena %d :  memory chunk size:  %lu Bytes\n", num,
                         (unsigned long int)(SAC_HM_arenas[num].min_chunk_size
                                             * UNIT_SIZE));
            } else {
                if (num == TOP_ARENA) {
                    fprintf (stderr, "Arena %d :  memory chunk size:  %lu -> ... Bytes\n",
                             num,
                             (unsigned long int)(SAC_HM_arenas[num].min_chunk_size
                                                 * UNIT_SIZE));
                } else {
                    fprintf (stderr, "Arena %d :  memory chunk size:  %lu -> %lu Bytes\n",
                             num,
                             (unsigned long int)(SAC_HM_arenas[num].min_chunk_size
                                                 * UNIT_SIZE),
                             (unsigned long int)((SAC_HM_arenas[num + 1].min_chunk_size
                                                  - 1)
                                                 * UNIT_SIZE));
                }
            }
        }
    }

    fprintf (stderr,
             "            %lu Bytes (%.1f MB) in %lu bins\n"
             "            %lu allocs  %lu splittings  (%d%%)\n"
             "            %lu frees   %lu coalascings (%d%%)\n"
             "=================================================================\n",
             size, ((float)size) / MB, bins, alloc, split,
             (alloc == 0 ? 0 : (int)((((float)split) / (float)alloc) * 100)), free,
             coalasce, (free == 0 ? 0 : (int)((((float)coalasce) / (float)free) * 100)));
}

void
SAC_HM_ShowDiagnostics ()
{
    int i;
    long int cnt_alloc_total = 0, cnt_free_total = 0, cnt_split_total = 0,
             cnt_coalasce_total = 0;
    long int bins_total = 0;

    fprintf (stderr,
             "=================================================================\n"
             "Heap Management diagnostics:\n"
             "=================================================================\n");

    fprintf (stderr,
             "calls to sbrk()  :  %lu\n"
             "total heap size  :  %lu Bytes (%lu MB)\n"
             "=================================================================\n",
             SAC_HM_call_sbrk, SAC_HM_heapsize, SAC_HM_heapsize / MB);

    fprintf (stderr,
             "calls to malloc()    :  %lu\n"
             "calls to free()      :  %lu\n"
             "calls to calloc()    :  %lu\n"
             "calls to realloc()   :  %lu\n"
             "calls to valloc()    :  %lu\n"
             "calls to memalign()  :  %lu\n"
             "=================================================================\n",
             SAC_HM_call_malloc, SAC_HM_call_free, SAC_HM_call_calloc,
             SAC_HM_call_realloc, SAC_HM_call_valloc, SAC_HM_call_memalign);

    for (i = 0; i < NUM_ARENAS; i++) {
        if (SAC_HM_arenas[i].bins > 0) {
            ShowDiagnosticsForArena (i, SAC_HM_arenas[i].size, SAC_HM_arenas[i].bins,
                                     SAC_HM_arenas[i].cnt_alloc,
                                     SAC_HM_arenas[i].cnt_free,
                                     SAC_HM_arenas[i].cnt_split,
                                     SAC_HM_arenas[i].cnt_coalasce);

            bins_total += SAC_HM_arenas[i].bins;
            cnt_alloc_total += SAC_HM_arenas[i].cnt_alloc;
            cnt_free_total += SAC_HM_arenas[i].cnt_free;
            cnt_split_total += SAC_HM_arenas[i].cnt_split;
            cnt_coalasce_total += SAC_HM_arenas[i].cnt_coalasce;
        }
    }

    ShowDiagnosticsForArena (-1, SAC_HM_heapsize, bins_total, cnt_alloc_total,
                             cnt_free_total, cnt_split_total, cnt_coalasce_total);
}

#endif /* DIAG */

static void
OutOfMemory (size_byte_t request)
{
    SAC_RuntimeError ("SAC heap manager failed to obtain %lu Bytes of memory "
                      "from operating system !",
                      request);
}

/*
 * Initial heap sizes are given in MB !!
 */

void
SAC_HM_Setup (size_byte_t initial_arena_of_arenas_size,
              size_byte_t initial_top_arena_size)
{
    size_byte_t offset, initial_heap_size;
    int i;
    SAC_HM_header_t *freep;
    char *mem;

    /*
     * Initialize doubly linked free chain for dummy chunks of each arena.
     */
    for (i = 0; i < NUM_SMALLCHUNK_ARENAS; i++) {
        SMALLCHUNK_PREVFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        SMALLCHUNK_NEXTFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[i].freelist);
        /*
         * Note here, that for small chunks the size entry and the diag patern entry
         * of the chunk administration data structure are identical. So setting the
         * pattern for the initial dummy chunk of size 0 which always remains in the
         * free list only works because the particular pattern is a negative number
         * and thereby prevents the allocation schemes from using the non-existing
         * memory behind it.
         * However, this "dirty" trick allows us a uniform handling of the diag patterns
         * without having a special check for the initial dummy chunks.
         */
    }
    for (i = NUM_SMALLCHUNK_ARENAS; i < NUM_ARENAS; i++) {
        LARGECHUNK_PREVFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        LARGECHUNK_NEXTFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_HM_arenas[i].freelist);
    }

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
    /* determine offset from last memory page boundary */

    if (offset != 0) {
        offset = SAC_HM_pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start */
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

        SMALLCHUNK_NEXTFREE (freep) = SAC_HM_arenas[ARENA_OF_ARENAS].freelist;
        SMALLCHUNK_PREVFREE (freep)
          = SMALLCHUNK_PREVFREE (SAC_HM_arenas[ARENA_OF_ARENAS].freelist);
        SMALLCHUNK_NEXTFREE (SMALLCHUNK_PREVFREE (freep)) = freep;
        SMALLCHUNK_PREVFREE (SMALLCHUNK_NEXTFREE (freep)) = freep;

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

/*
 * The diagnostic check pattern mechanism cannot be used in the arena of arenas
 * because here we use small chunks, but definitely need the sizes of all chunks
 * since these are NOT uniform.
 */

static SAC_HM_header_t *
AllocateArena (size_unit_t units)
{
    SAC_HM_header_t *freep, *firstp;
    SAC_HM_arena_t *arena;
    size_unit_t split_threshold;

    DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].cnt_alloc);

    arena = &(SAC_HM_arenas[ARENA_OF_ARENAS]);
    split_threshold = units + arena->min_chunk_size;
    firstp = arena->freelist;
    freep = SMALLCHUNK_NEXTFREE (firstp);

    while (freep != firstp) {

        if (SMALLCHUNK_SIZE (freep) < units) {
            /* free space too small */
            freep = SMALLCHUNK_NEXTFREE (freep);
            continue;
        }

        /* sufficient space found */
        if (SMALLCHUNK_SIZE (freep) >= split_threshold) {
            /* Split found space */
            DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].cnt_split);
            SMALLCHUNK_SIZE (freep) -= units;
            return (freep + SMALLCHUNK_SIZE (freep));
        } else {
            /* Do NOT split found space, so remove it from free list */
            SMALLCHUNK_NEXTFREE (SMALLCHUNK_PREVFREE (freep))
              = SMALLCHUNK_NEXTFREE (freep);
            SMALLCHUNK_PREVFREE (SMALLCHUNK_NEXTFREE (freep))
              = SMALLCHUNK_PREVFREE (freep);
            return (freep);
        }
    }

    /*
     * No sufficient space found in freelist, so, we have to allocate
     * an additional arena of arenas.
     */

    freep = (SAC_HM_header_t *)
      SAC_HM_MallocTopArena (SAC_HM_arenas[ARENA_OF_ARENAS].arena_size + 2,
                             &(SAC_HM_arenas[TOP_ARENA]));

    DIAG_ADD (SAC_HM_arenas[ARENA_OF_ARENAS].size,
              SAC_HM_arenas[ARENA_OF_ARENAS].arena_size * UNIT_SIZE);
    DIAG_INC (SAC_HM_arenas[ARENA_OF_ARENAS].bins);

    SMALLCHUNK_SIZE (freep) = arena->arena_size - units;

    SMALLCHUNK_NEXTFREE (freep) = arena->freelist;
    SMALLCHUNK_PREVFREE (freep) = SMALLCHUNK_PREVFREE (arena->freelist);
    SMALLCHUNK_NEXTFREE (SMALLCHUNK_PREVFREE (freep)) = freep;
    SMALLCHUNK_PREVFREE (SMALLCHUNK_NEXTFREE (freep)) = freep;

    return (freep + SMALLCHUNK_SIZE (freep));
}

void *
SAC_HM_MallocSmallChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *firstp, *wilderness;

    DIAG_INC (arena->cnt_alloc);

    firstp = arena->freelist;
    freep = SMALLCHUNK_NEXTFREE (firstp);

    DIAG_CHECK_FREEPATTERN_SMALLCHUNK (freep, arena->num);

    if (freep != firstp) {
        /* free entry found, size matches due to fixed size chunks */
        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

        SMALLCHUNK_NEXTFREE (SMALLCHUNK_PREVFREE (freep)) = SMALLCHUNK_NEXTFREE (freep);
        SMALLCHUNK_PREVFREE (SMALLCHUNK_NEXTFREE (freep)) = SMALLCHUNK_PREVFREE (freep);
        return ((void *)(freep + 1));
    }

    /*
     * Try to split memory from wilderness chunk
     */

    wilderness = arena->wilderness;

    if (SMALLCHUNK_SIZE (wilderness) > units) {
        /* sufficient space in wilderness chunk found */
        /* split chunk from wilderness */

        DIAG_INC (arena->cnt_split);

        SMALLCHUNK_SIZE (wilderness) -= units;
        freep = wilderness + SMALLCHUNK_SIZE (wilderness);
        SMALLCHUNK_ARENA (freep) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (freep);

        return ((void *)(freep + 1));
    }

    if (SMALLCHUNK_SIZE (wilderness) == units) {
        /* exactly fitting space in wilderness chunk found */

        SMALLCHUNK_ARENA (wilderness) = arena;

        DIAG_SET_ALLOCPATTERN_SMALLCHUNK (wilderness);

        arena->wilderness = arena->freelist;
        return ((void *)(wilderness + 1));
    }

    /*
     * Allocate new arena and use this as new wilderness chunk.
     */

    wilderness = AllocateArena (arena->arena_size);

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

void *
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *firstp;
    size_unit_t split_threshold;

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;
    firstp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (firstp);

    while (freep != firstp) {
        DIAG_CHECK_FREEPATTERN_LARGECHUNK (freep, arena->num);

        if (LARGECHUNK_SIZE (freep) < units) {
            /* free space too small */
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /* sufficient space found */
        if (LARGECHUNK_SIZE (freep) >= split_threshold) {
            /* Split found space */
            DIAG_INC (arena->cnt_split);
            LARGECHUNK_SIZE (freep) -= units;
            firstp = freep + LARGECHUNK_SIZE (freep);
            LARGECHUNK_SIZE (firstp) = -units;
            LARGECHUNK_ARENA (firstp) = arena;
            LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (firstp + units) = -units;
            DIAG_SET_ALLOCPATTERN_LARGECHUNK (firstp);
            return ((void *)(firstp + 2));
        } else {
            /* Do NOT split found space, so remove it from free list */
            LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep))
              = LARGECHUNK_NEXTFREE (freep);
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep))
              = LARGECHUNK_PREVFREE (freep);
            LARGECHUNK_SIZE (freep) = -LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (freep - LARGECHUNK_SIZE (freep))
              = LARGECHUNK_SIZE (freep);
            DIAG_SET_ALLOCPATTERN_LARGECHUNK (freep);
            return ((void *)(freep + 2));
        }
    }

    /*
     * No sufficient space found in freelist, so, we have to allocate
     * an additional arena.
     */

    freep = AllocateArena (arena->arena_size);

    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->bins);
    DIAG_ADD (arena->size, arena->arena_size * UNIT_SIZE);
    DIAG_INC (arena->cnt_split);

    LARGECHUNK_SIZE (freep) = arena->arena_size - (units + 2);
    LARGECHUNK_PREVSIZE (freep) = -1;
    LARGECHUNK_ARENA (freep) = arena;

    LARGECHUNK_NEXTFREE (freep) = arena->freelist;
    LARGECHUNK_PREVFREE (freep) = LARGECHUNK_PREVFREE (arena->freelist);
    LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep)) = freep;
    LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;

    firstp = freep + LARGECHUNK_SIZE (freep);

    LARGECHUNK_SIZE (firstp) = -units;
    LARGECHUNK_ARENA (firstp) = arena;
    LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);
    DIAG_SET_ALLOCPATTERN_LARGECHUNK (firstp);

    LARGECHUNK_SIZE (firstp + units) = -1;
    LARGECHUNK_PREVSIZE (firstp + units) = -units; /* should be superfluous */

    return ((void *)(firstp + 2));
}

static SAC_HM_header_t *
ExtendTopArenaWilderness (size_unit_t units, SAC_HM_arena_t *arena)
{
    size_unit_t new_mem;
    char *mem;
    SAC_HM_header_t *wilderness;

    wilderness = arena->wilderness;

    new_mem = ((units - LARGECHUNK_SIZE (wilderness) + 3) * UNIT_SIZE);
    new_mem = (new_mem + SBRK_CHUNK) & (~(SBRK_CHUNK - 1));

    DIAG_INC (SAC_HM_call_sbrk);
    mem = (char *)sbrk (new_mem);
    if (mem == (char *)-1) {
        OutOfMemory (new_mem);
    }
    DIAG_ADD (SAC_HM_heapsize, new_mem);
    DIAG_ADD (arena->size, new_mem);
    DIAG_INC (arena->bins);

    if ((SAC_HM_header_t *)mem == wilderness + LARGECHUNK_SIZE (wilderness)) {
        /* New memory and old wilderness chunk are contiguous. */
        LARGECHUNK_SIZE (wilderness) += new_mem / UNIT_SIZE;
        return (wilderness);
    } else {
        /*
         * New memory and old wilderness chunk are NOT contiguous.
         * Old wilderness becomes regular free chunk. Request is
         * satisfied from freshly allocated memory.
         */

        LARGECHUNK_SIZE (wilderness) -= 3;
        LARGECHUNK_SIZE (wilderness + LARGECHUNK_SIZE (wilderness)) = -1;

        LARGECHUNK_NEXTFREE (wilderness) = arena->freelist;
        LARGECHUNK_PREVFREE (wilderness) = LARGECHUNK_PREVFREE (arena->freelist);
        LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (wilderness)) = wilderness;
        LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (wilderness)) = wilderness;

        if (new_mem >= units * UNIT_SIZE) {
            /*
             * Freshly allocated memory suffices to satisfy request.
             */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            LARGECHUNK_SIZE (wilderness) = new_mem / UNIT_SIZE;
            LARGECHUNK_PREVSIZE (wilderness) = -1;
            DIAG_SET_FREEPATTERN_LARGECHUNK (wilderness);
            return (wilderness);
        } else {
            /*
             * Get additional memory from operating system.
             */
            size_unit_t more_mem;
            char *mem2;

            more_mem = units * UNIT_SIZE - new_mem;
            more_mem = (more_mem + SBRK_CHUNK) & (~(SBRK_CHUNK - 1));

            DIAG_INC (SAC_HM_call_sbrk);
            mem2 = (char *)sbrk (more_mem);
            if ((mem2 == (char *)-1) || (mem + new_mem != mem2)) {
                OutOfMemory (more_mem);
            }
            DIAG_ADD (SAC_HM_heapsize, more_mem);
            DIAG_ADD (arena->size, more_mem);
            DIAG_INC (arena->bins);

            /* mem and mem2 are contiguous ! */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            LARGECHUNK_SIZE (wilderness) = (new_mem + more_mem) / UNIT_SIZE;
            LARGECHUNK_PREVSIZE (wilderness) = -1;
            DIAG_SET_FREEPATTERN_LARGECHUNK (wilderness);
            return (wilderness);
        }
    }
}

void *
SAC_HM_MallocTopArena (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *firstp, *wilderness;
    size_unit_t split_threshold;

    DIAG_INC (arena->cnt_alloc);

    split_threshold = units + arena->min_chunk_size;
    firstp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (firstp);

    while (freep != firstp) {
        DIAG_CHECK_FREEPATTERN_LARGECHUNK (freep, arena->num);

        if (LARGECHUNK_SIZE (freep) < units) {
            /* free space too small */
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /* sufficient space found */
        if (LARGECHUNK_SIZE (freep) >= split_threshold) {
            /* Split found space */
            DIAG_INC (arena->cnt_split);
            LARGECHUNK_SIZE (freep) -= units;
            firstp = freep + LARGECHUNK_SIZE (freep);
            LARGECHUNK_SIZE (firstp) = -units;
            LARGECHUNK_ARENA (firstp) = arena;
            LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (firstp + units) = -units;
            DIAG_SET_ALLOCPATTERN_LARGECHUNK (firstp);
            return ((void *)(firstp + 2));
        } else {
            /* Do NOT split found space, so remove it from free list */
            LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep))
              = LARGECHUNK_NEXTFREE (freep);
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep))
              = LARGECHUNK_PREVFREE (freep);
            LARGECHUNK_SIZE (freep) = -LARGECHUNK_SIZE (freep);
            LARGECHUNK_ARENA (freep) = arena;
            LARGECHUNK_PREVSIZE (freep - LARGECHUNK_SIZE (freep))
              = LARGECHUNK_SIZE (freep);
            DIAG_SET_ALLOCPATTERN_LARGECHUNK (freep);
            return ((void *)(freep + 2));
        }
    }

    /*
     * No sufficient space found in freelist, so, we check out the wilderness chunk.
     */

    wilderness = arena->wilderness;
    DIAG_CHECK_FREEPATTERN_LARGECHUNK (wilderness, arena->num);

    if (LARGECHUNK_SIZE (wilderness) < units + 3) {
        /* remaining wilderness is too small. */
        wilderness = ExtendTopArenaWilderness (units, arena);
    }

    /*
     * Now, wilderness is guaranteed to be sufficiently large.
     */

    firstp = wilderness + units; /* firstp points to new wilderness */
    LARGECHUNK_SIZE (firstp) = LARGECHUNK_SIZE (wilderness) - units;
    LARGECHUNK_PREVSIZE (firstp) = -units;
    DIAG_SET_FREEPATTERN_LARGECHUNK (firstp);
    arena->wilderness = firstp;

    LARGECHUNK_SIZE (wilderness) = -units;
    LARGECHUNK_ARENA (wilderness) = arena;
    DIAG_INC (arena->cnt_split);
    DIAG_SET_ALLOCPATTERN_LARGECHUNK (wilderness);
    return ((void *)(wilderness + 2));
}

void
SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *freelist;

    freep = addr - 1;
    freelist = arena->freelist;

    DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_SMALLCHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SMALLCHUNK_NEXTFREE (freep) = SMALLCHUNK_NEXTFREE (freelist);
    SMALLCHUNK_PREVFREE (freep) = freelist;
    SMALLCHUNK_NEXTFREE (freelist) = freep;
    SMALLCHUNK_PREVFREE (SMALLCHUNK_NEXTFREE (freep)) = freep;
}

void
SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *freelist, *prevp, *nextp;

    freep = addr - 2;

    DIAG_CHECK_ALLOCPATTERN_LARGECHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->cnt_free);

    nextp = freep - LARGECHUNK_SIZE (freep); /* chunk size is negated ! */

    if (LARGECHUNK_SIZE (nextp) > 0) {
        if (LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * Coalasce with both neighboring chunks.
             */
            DIAG_INC (arena->cnt_coalasce);
            prevp = freep - LARGECHUNK_PREVSIZE (freep);
            LARGECHUNK_SIZE (prevp) += LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
            /* chunk size is negated ! */
            LARGECHUNK_PREVSIZE (nextp + LARGECHUNK_SIZE (nextp))
              = LARGECHUNK_SIZE (prevp);
            LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (nextp))
              = LARGECHUNK_NEXTFREE (nextp);
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (nextp))
              = LARGECHUNK_PREVFREE (nextp);
            return;
        } else {
            /*
             * Coalasce with next chunk.
             */
            DIAG_INC (arena->cnt_coalasce);
            LARGECHUNK_SIZE (freep) = LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
            /* chunk size is negated ! */
            LARGECHUNK_PREVSIZE (nextp + LARGECHUNK_SIZE (nextp))
              = LARGECHUNK_SIZE (freep);
            LARGECHUNK_NEXTFREE (freep) = LARGECHUNK_NEXTFREE (nextp);
            LARGECHUNK_PREVFREE (freep) = LARGECHUNK_PREVFREE (nextp);
            LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep)) = freep;
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;
            return;
        }
    } else {
        if (LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * Coalasce with previous chunk.
             */
            DIAG_INC (arena->cnt_coalasce);
            prevp = freep - LARGECHUNK_PREVSIZE (freep);
            LARGECHUNK_SIZE (prevp)
              -= LARGECHUNK_SIZE (freep); /* chunk size is negated ! */
            LARGECHUNK_PREVSIZE (nextp) = LARGECHUNK_SIZE (prevp);
            return;
        } else {
            /*
             * No coalascing at all.
             */
            LARGECHUNK_SIZE (freep) = -LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (nextp) = LARGECHUNK_SIZE (freep);
            freelist = arena->freelist;
            LARGECHUNK_NEXTFREE (freep) = LARGECHUNK_NEXTFREE (freelist);
            LARGECHUNK_PREVFREE (freep) = freelist;
            LARGECHUNK_NEXTFREE (freelist) = freep;
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;
            return;
        }
    }
}

void
SAC_HM_FreeTopArena (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *freelist, *prevp, *nextp;

    freep = addr - 2;

    DIAG_CHECK_ALLOCPATTERN_LARGECHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->cnt_free);

    nextp = freep - LARGECHUNK_SIZE (freep); /* chunk size is negated ! */

    if (LARGECHUNK_SIZE (nextp) > 0) {
        if (nextp == arena->wilderness) {
            if (LARGECHUNK_PREVSIZE (freep) > 0) {
                /*
                 * Coalasce current and previous chunk with wilderness.
                 */
                DIAG_INC (arena->cnt_coalasce);
                prevp = freep - LARGECHUNK_PREVSIZE (freep);
                LARGECHUNK_SIZE (prevp)
                  += LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
                /* chunk size is negated ! */
                LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (prevp))
                  = LARGECHUNK_NEXTFREE (prevp);
                LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (prevp))
                  = LARGECHUNK_PREVFREE (prevp);
                /* remove previous chunk from free list. */

                arena->wilderness = prevp;
                return;
            } else {
                /*
                 * Coalasce current chunk with wilderness.
                 */
                DIAG_INC (arena->cnt_coalasce);
                LARGECHUNK_SIZE (freep)
                  = LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
                /* chunk size is negated ! */
                arena->wilderness = freep;
                return;
            }
        } else {
            if (LARGECHUNK_PREVSIZE (freep) > 0) {
                /*
                 * Coalasce with both neighboring chunks.
                 */
                DIAG_INC (arena->cnt_coalasce);
                prevp = freep - LARGECHUNK_PREVSIZE (freep);
                LARGECHUNK_SIZE (prevp)
                  += LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
                /* chunk size is negated ! */
                LARGECHUNK_PREVSIZE (nextp + LARGECHUNK_SIZE (nextp))
                  = LARGECHUNK_SIZE (prevp);
                LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (nextp))
                  = LARGECHUNK_NEXTFREE (nextp);
                LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (nextp))
                  = LARGECHUNK_PREVFREE (nextp);
                /* remove next chunk from free list. */
                return;
            } else {
                /*
                 * Coalasce with next chunk.
                 */
                DIAG_INC (arena->cnt_coalasce);
                LARGECHUNK_SIZE (freep)
                  = LARGECHUNK_SIZE (nextp) - LARGECHUNK_SIZE (freep);
                /* chunk size is negated ! */
                LARGECHUNK_PREVSIZE (nextp + LARGECHUNK_SIZE (nextp))
                  = LARGECHUNK_SIZE (freep);

                LARGECHUNK_NEXTFREE (freep) = LARGECHUNK_NEXTFREE (nextp);
                LARGECHUNK_PREVFREE (freep) = LARGECHUNK_PREVFREE (nextp);
                LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep)) = freep;
                LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;
                /* Replace next chunk by current chunk in free list. */
                return;
            }
        }
    } else {
        if (LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * Coalasce with previous chunk.
             */
            DIAG_INC (arena->cnt_coalasce);
            prevp = freep - LARGECHUNK_PREVSIZE (freep); /* chunk size is negated ! */
            LARGECHUNK_SIZE (prevp) -= LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (nextp) = LARGECHUNK_SIZE (prevp);
            return;
        } else {
            /*
             * No coalascing at all.
             */
            LARGECHUNK_SIZE (freep)
              = -LARGECHUNK_SIZE (freep); /* chunk size is negated ! */
            LARGECHUNK_PREVSIZE (nextp) = LARGECHUNK_SIZE (freep);
            freelist = arena->freelist;
            LARGECHUNK_NEXTFREE (freep) = LARGECHUNK_NEXTFREE (freelist);
            LARGECHUNK_PREVFREE (freep) = freelist;
            LARGECHUNK_NEXTFREE (freelist) = freep;
            LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;
            return;
        }
    }
}

/*
 *
 */

void *
SAC_HM_MallocCheck (size_byte_t size)
{
    return (malloc (size));
}
