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

#include <unistd.h>
#include <string.h>

#include "heapmgr.h"
#include "sac_message.h"

#define KB 1024
#define MB (KB * KB)
#define SBRK_CHUNK (MB)

#define ARENA_BASE(n)                                                                    \
    {                                                                                    \
        {0, NULL}, {0, &(SAC_HM_arenas[n])},                                             \
        {                                                                                \
            0, NULL                                                                      \
        }                                                                                \
    }

#define ARENA_OF_SMALL_CHUNKS(n, size, mincs, freefun)                                   \
    {                                                                                    \
        ARENA_BASE (n), size, mincs, (SAC_HM_arenas[n].base) + 1,                        \
          (SAC_HM_arenas[n].base) + 1, freefun                                           \
    }

#define ARENA_OF_LARGE_CHUNKS(n, size, mincs, freefun)                                   \
    {                                                                                    \
        ARENA_BASE (n), size, mincs, SAC_HM_arenas[n].base, SAC_HM_arenas[n].base,       \
          &freefun                                                                       \
    }

SAC_HM_arena_t SAC_HM_arenas[NUM_ARENAS]
  = {ARENA_OF_SMALL_CHUNKS (0, 131072, 128, NULL),
     ARENA_OF_SMALL_CHUNKS (1, 512, ARENA_2_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (2, 512, ARENA_3_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (3, 256, ARENA_4_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_SMALL_CHUNKS (4, 512, ARENA_5_MINCS - 1, SAC_HM_FreeSmallChunk),
     ARENA_OF_LARGE_CHUNKS (5, 2048, ARENA_5_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (6, 8192, ARENA_6_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (7, 32768, ARENA_7_MINCS, SAC_HM_FreeLargeChunk),
     ARENA_OF_LARGE_CHUNKS (8, 0, ARENA_8_MINCS, SAC_HM_FreeTopArena)};

#define ARENA_OF_ARENAS 0

size_byte_t SAC_HM_pagesize;

static size_byte_t units_per_page;
static char *initial_break_val;

static void
OutOfMemory (size_byte_t request)
{
    SAC_RuntimeError ("SAC heap manager failed to obtain %lu Bytes of memory "
                      "from operating system !",
                      request);
}

void
SAC_HM_Init (size_unit_t initial_arena_of_arenas_size, size_unit_t initial_top_arena_size)
{
    size_unit_t offset, initial_heap_size;
    int i;
    SAC_HM_header_t *freep;
    char *mem;

    /*
     * Initialize doubly linked free chain for dummy chunks of each arena.
     */
    for (i = 0; i < NUM_SMALLCHUNK_ARENAS; i++) {
        SMALLCHUNK_PREVFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        SMALLCHUNK_NEXTFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
    }
    for (i = NUM_SMALLCHUNK_ARENAS; i < NUM_ARENAS; i++) {
        LARGECHUNK_PREVFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
        LARGECHUNK_NEXTFREE (SAC_HM_arenas[i].freelist) = SAC_HM_arenas[i].freelist;
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

    offset = ((size_byte_t)initial_break_val) % SAC_HM_pagesize;
    /* determine offset from last memory page boundary */

    if (offset != 0) {
        offset = SAC_HM_pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start */
    }

    /* Adjust function parameters to full page sizes. */
    initial_heap_size
      = offset + MB * (initial_arena_of_arenas_size + initial_top_arena_size);

    mem = (char *)sbrk (initial_heap_size);
    if (mem == (char *)-1) {
        OutOfMemory (initial_heap_size);
    }

    /*
     * Allocate arena of arenas.
     */

    mem += offset;
    freep = (SAC_HM_header_t *)mem;

    SMALLCHUNK_SIZE (freep) = (MB * initial_arena_of_arenas_size) / UNIT_SIZE;
    SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[ARENA_OF_ARENAS]);

    SAC_HM_arenas[ARENA_OF_ARENAS].wilderness = freep;

    /*
     * Allocate top arena.
     */

    mem += initial_arena_of_arenas_size;
    freep = (SAC_HM_header_t *)mem;

    LARGECHUNK_SIZE (freep) = (MB * initial_top_arena_size) / UNIT_SIZE;
    LARGECHUNK_ARENA (freep) = &(SAC_HM_arenas[NUM_ARENAS - 1]);
    LARGECHUNK_PREVSIZE (freep) = -1;

    SAC_HM_arenas[NUM_ARENAS - 1].wilderness = freep;
}

static SAC_HM_header_t *
AllocateArena (size_unit_t units)
{
    SAC_HM_header_t *freep, *firstp;
    SAC_HM_arena_t *arena;
    size_unit_t split_threshold;

    arena = &(SAC_HM_arenas[0]);
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
                             &(SAC_HM_arenas[NUM_ARENAS - 1]));

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

    firstp = arena->freelist;
    freep = SMALLCHUNK_NEXTFREE (firstp);

    if (freep != firstp) {
        /* free entry found, size matches due to fixed size chunks */
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

        SMALLCHUNK_SIZE (wilderness) -= units;
        freep = wilderness + SMALLCHUNK_SIZE (wilderness);
        SMALLCHUNK_ARENA (freep) = arena;
        return ((void *)(freep + 1));
    }

    if (SMALLCHUNK_SIZE (wilderness) == units) {
        /* exactly fitting space in wilderness chunk found */

        SMALLCHUNK_ARENA (wilderness) = arena;
        arena->wilderness = arena->freelist;
        return ((void *)(wilderness + 1));
    }

    /*
     * Allocate new arena and use this as new wilderness chunk.
     */

    wilderness = AllocateArena (arena->arena_size);

    SMALLCHUNK_SIZE (wilderness) = arena->arena_size - units;
    arena->wilderness = wilderness;
    freep = wilderness + SMALLCHUNK_SIZE (wilderness);
    SMALLCHUNK_ARENA (freep) = arena;
    return ((void *)(freep + 1));
}

void *
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *firstp;
    size_unit_t split_threshold;

    split_threshold = units + arena->min_chunk_size;
    firstp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (firstp);

    while (freep != firstp) {
        if (LARGECHUNK_SIZE (freep) < units) {
            /* free space too small */
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /* sufficient space found */
        if (LARGECHUNK_SIZE (freep) >= split_threshold) {
            /* Split found space */
            LARGECHUNK_SIZE (freep) -= units;
            firstp = freep + LARGECHUNK_SIZE (freep);
            LARGECHUNK_SIZE (firstp) = -units;
            LARGECHUNK_ARENA (firstp) = arena;
            LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (firstp + units) = -units;
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
            return ((void *)(freep + 2));
        }
    }

    /*
     * No sufficient space found in freelist, so, we have to allocate
     * an additional arena.
     */

    freep = AllocateArena (arena->arena_size);

    LARGECHUNK_SIZE (freep) = arena->arena_size - (units + 2);
    LARGECHUNK_PREVSIZE (freep) = -1;

    LARGECHUNK_NEXTFREE (freep) = arena->freelist;
    LARGECHUNK_PREVFREE (freep) = LARGECHUNK_PREVFREE (arena->freelist);
    LARGECHUNK_NEXTFREE (LARGECHUNK_PREVFREE (freep)) = freep;
    LARGECHUNK_PREVFREE (LARGECHUNK_NEXTFREE (freep)) = freep;

    firstp = freep + LARGECHUNK_SIZE (freep);

    LARGECHUNK_SIZE (firstp) = -units;
    LARGECHUNK_ARENA (firstp) = arena;
    LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);

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

    mem = (char *)sbrk (new_mem);
    if (mem == (char *)-1) {
        OutOfMemory (new_mem);
    }

    if ((SAC_HM_header_t *)mem == wilderness + LARGECHUNK_SIZE (wilderness)) {
        /* New memory and old wilderness chunk are contiguous. */
        LARGECHUNK_SIZE (wilderness) += new_mem / UNIT_SIZE;
        return (wilderness);
    } else {
        /*
         * New memory and old wilderness chunk are contiguous.
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
            return (wilderness);
        } else {
            /*
             * Get additional memory from operating system.
             */
            size_unit_t more_mem;
            char *mem2;

            more_mem = units * UNIT_SIZE - new_mem;
            more_mem = (more_mem + SBRK_CHUNK) & (~(SBRK_CHUNK - 1));

            mem2 = (char *)sbrk (more_mem);
            if ((mem2 == (char *)-1) || (mem + new_mem != mem2)) {
                OutOfMemory (more_mem);
            }

            /* mem and mem2 are contiguous ! */
            wilderness = (SAC_HM_header_t *)mem;

            arena->wilderness = wilderness;
            LARGECHUNK_SIZE (wilderness) = (new_mem + more_mem) / UNIT_SIZE;
            LARGECHUNK_PREVSIZE (wilderness) = -1;
            return (wilderness);
        }
    }
}

void *
SAC_HM_MallocTopArena (size_unit_t units, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *firstp, *wilderness;
    size_unit_t split_threshold;

    split_threshold = units + arena->min_chunk_size;
    firstp = arena->freelist;
    freep = LARGECHUNK_NEXTFREE (firstp);

    while (freep != firstp) {
        if (LARGECHUNK_SIZE (freep) < units) {
            /* free space too small */
            freep = LARGECHUNK_NEXTFREE (freep);
            continue;
        }

        /* sufficient space found */
        if (LARGECHUNK_SIZE (freep) >= split_threshold) {
            /* Split found space */
            LARGECHUNK_SIZE (freep) -= units;
            firstp = freep + LARGECHUNK_SIZE (freep);
            LARGECHUNK_SIZE (firstp) = -units;
            LARGECHUNK_ARENA (firstp) = arena;
            LARGECHUNK_PREVSIZE (firstp) = LARGECHUNK_SIZE (freep);
            LARGECHUNK_PREVSIZE (firstp + units) = -units;
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
            return ((void *)(freep + 2));
        }
    }

    /*
     * No sufficient space found in freelist, so, we check out the wilderness chunk.
     */

    wilderness = arena->wilderness;

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
    arena->wilderness = firstp;

    LARGECHUNK_SIZE (wilderness) = -units;
    LARGECHUNK_ARENA (wilderness) = arena;
    return ((void *)(wilderness + 2));
}

void
SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *freep, *freelist;

    freep = addr - 1;
    freelist = arena->freelist;

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
    nextp = freep - LARGECHUNK_SIZE (freep); /* chunk size is negated ! */

    if (LARGECHUNK_SIZE (nextp) > 0) {
        if (LARGECHUNK_PREVSIZE (freep) > 0) {
            /*
             * Coalasce with both neighboring chunks.
             */
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
    nextp = freep - LARGECHUNK_SIZE (freep); /* chunk size is negated ! */

    if (LARGECHUNK_SIZE (nextp) > 0) {
        if (nextp == arena->wilderness) {
            if (LARGECHUNK_PREVSIZE (freep) > 0) {
                /*
                 * Coalasce current and previous chunk with wilderness.
                 */
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
