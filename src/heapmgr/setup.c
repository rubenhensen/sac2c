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
 * file:   setup.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains the setup or initialization facilities for the
 *   SAC private heap manager.
 *
 *
 *****************************************************************************/

#include "heapmgr.h"
#include "sac_message.h"

#include <unistd.h>

/*
 * Heap management configuration data.
 */

static const size_unit_t min_chunk_size[]
  = {ARENA_0_MINCS, ARENA_1_MINCS, ARENA_2_MINCS, ARENA_3_MINCS, ARENA_4_MINCS,
     ARENA_5_MINCS, ARENA_6_MINCS, ARENA_7_MINCS, ARENA_8_MINCS};

static const size_unit_t binsize[]
  = {ARENA_0_BINSIZE, ARENA_1_BINSIZE, ARENA_2_BINSIZE, ARENA_3_BINSIZE, ARENA_4_BINSIZE,
     ARENA_5_BINSIZE, ARENA_6_BINSIZE, ARENA_7_BINSIZE, ARENA_8_BINSIZE};

/*
 * Top arena lock for synchronisation of multi-threaded memory accesses.
 */

#ifdef MT
pthread_mutex_t SAC_HM_top_arena_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t SAC_HM_diag_counter_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

/******************************************************************************
 *
 * function:
 *   void SAC_HM_Setup(unsigned int num_threads,
 *                     size_byte_t initial_master_arena_of_arenas_size,
 *                     size_byte_t initial_worker_arena_of_arenas_size,
 *                     size_byte_t initial_top_arena_size)
 *
 * description:
 *
 *   This function initializes the internal data structures of the heap
 *   manager. The arenas of arenas as well as the top arena are allocated
 *   and initialized immediately; their initial sizes given in KB are
 *   provided as arguments.
 *
 *   Particular care is taken of the correct alignment of these inital
 *   heap structures.
 *
 ******************************************************************************/

#ifdef MT
void
SAC_HM_Setup_mt (unsigned int num_threads,
                 size_byte_t initial_master_arena_of_arenas_size,
                 size_byte_t initial_worker_arena_of_arenas_size,
                 size_byte_t initial_top_arena_size)
#else  /* MT */
void
SAC_HM_Setup (size_byte_t initial_master_arena_of_arenas_size,
              size_byte_t initial_top_arena_size)
#endif /* MT */
{
    size_byte_t offset, initial_heap_size, pagesize;
    SAC_HM_header_t *freep;
    char *mem;
    int i, t;
#ifndef MT
    const unsigned int num_threads = 1;
    size_byte_t initial_worker_arena_of_arenas_size = 0;
#endif /* MT */

    /*
     * Prepare initial request for memory from operating system.
     */

    initial_master_arena_of_arenas_size *= KB;
    initial_worker_arena_of_arenas_size *= KB;
    initial_top_arena_size *= KB;

    pagesize = getpagesize ();
    mem = (char *)sbrk (0);
    /* Determine heap origin. */

    offset = ((size_byte_t)mem) % pagesize;
    /* Determine offset of heap origin from last memory page boundary. */

    if (offset != 0) {
        offset = pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start. */
    }

    /* Compute initial heap size to be requested from operating system. */
    initial_heap_size = offset + initial_master_arena_of_arenas_size
                        + (initial_worker_arena_of_arenas_size * (num_threads - 1))
                        + initial_top_arena_size;

    mem = (char *)sbrk (initial_heap_size);
    if (mem == (char *)-1) {
        SAC_HM_OutOfMemory (initial_heap_size);
    }

    mem += offset;

    DIAG_SET (SAC_HM_call_sbrk, 2);
    DIAG_SET (SAC_HM_heapsize, initial_heap_size);

    /*
     * Initialize global array of arena descriptions.
     */

    for (t = 0; t < num_threads; t++) {
        for (i = 0; i < NUM_SMALLCHUNK_ARENAS; i++) {
            SAC_HM_arenas[t][i].num = i;
            SAC_HM_arenas[t][i].freelist[0].data1.size = 0;
            SAC_HM_arenas[t][i].freelist[0].data1.arena = &(SAC_HM_arenas[t][i]);
            SAC_HM_arenas[t][i].freelist[1].data2.nextfree = NULL;

            SAC_HM_arenas[t][i].wilderness = SAC_HM_arenas[t][i].freelist;
            SAC_HM_arenas[t][i].binsize = binsize[i];
            SAC_HM_arenas[t][i].min_chunk_size = min_chunk_size[i];
#ifdef DIAG
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[t][i].freelist);
            SAC_HM_arenas[t][i].cnt_alloc = 0;
            SAC_HM_arenas[t][i].cnt_free = 0;
            SAC_HM_arenas[t][i].cnt_split = 0;
            SAC_HM_arenas[t][i].cnt_coalasce = 0;
            SAC_HM_arenas[t][i].size = 0;
            SAC_HM_arenas[t][i].bins = 0;
#endif
        }
        for (i = NUM_SMALLCHUNK_ARENAS; i < NUM_ARENAS - 1; i++) {
            SAC_HM_arenas[t][i].num = i;
            SAC_HM_arenas[t][i].freelist[0].data3.prevsize = -1;
            SAC_HM_arenas[t][i].freelist[1].data1.arena = &(SAC_HM_arenas[t][i]);
            SAC_HM_arenas[t][i].freelist[1].data1.size = 0;
            SAC_HM_arenas[t][i].freelist[2].data2.nextfree = NULL;
            SAC_HM_arenas[t][i].wilderness = SAC_HM_arenas[t][i].freelist;
            SAC_HM_arenas[t][i].binsize = binsize[i];
            SAC_HM_arenas[t][i].min_chunk_size = min_chunk_size[i];
#ifdef DIAG
            DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_HM_arenas[t][i].freelist);
            SAC_HM_arenas[t][i].cnt_alloc = 0;
            SAC_HM_arenas[t][i].cnt_free = 0;
            SAC_HM_arenas[t][i].cnt_split = 0;
            SAC_HM_arenas[t][i].cnt_coalasce = 0;
            SAC_HM_arenas[t][i].size = 0;
            SAC_HM_arenas[t][i].bins = 0;
#endif
        }
    }

    SAC_HM_arenas[0][TOP_ARENA].num = TOP_ARENA;
    SAC_HM_arenas[0][TOP_ARENA].freelist[0].data3.prevsize = -1;
    SAC_HM_arenas[0][TOP_ARENA].freelist[1].data1.arena = &(SAC_HM_arenas[0][TOP_ARENA]);
    SAC_HM_arenas[0][TOP_ARENA].freelist[1].data1.size = 0;
    SAC_HM_arenas[0][TOP_ARENA].freelist[2].data2.nextfree = NULL;
    SAC_HM_arenas[0][TOP_ARENA].wilderness = SAC_HM_arenas[0][TOP_ARENA].freelist;
    SAC_HM_arenas[0][TOP_ARENA].binsize = binsize[TOP_ARENA];
    SAC_HM_arenas[0][TOP_ARENA].min_chunk_size = min_chunk_size[TOP_ARENA];
#ifdef DIAG
    DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_HM_arenas[0][TOP_ARENA].freelist);
    SAC_HM_arenas[0][TOP_ARENA].cnt_alloc = 0;
    SAC_HM_arenas[0][TOP_ARENA].cnt_free = 0;
    SAC_HM_arenas[0][TOP_ARENA].cnt_split = 0;
    SAC_HM_arenas[0][TOP_ARENA].cnt_coalasce = 0;
    SAC_HM_arenas[0][TOP_ARENA].size = 0;
    SAC_HM_arenas[0][TOP_ARENA].bins = 0;
#endif

    /*
     * Allocate master arena of arenas.
     */

    if (initial_master_arena_of_arenas_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        SAC_HM_SMALLCHUNK_SIZE (freep) = initial_master_arena_of_arenas_size / UNIT_SIZE;
        SAC_HM_SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[0][ARENA_OF_ARENAS]);

        SAC_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
        SAC_HM_SMALLCHUNK_NEXTFREE (SAC_HM_arenas[0][ARENA_OF_ARENAS].freelist) = freep;

        DIAG_SET (SAC_HM_arenas[0][ARENA_OF_ARENAS].size,
                  initial_master_arena_of_arenas_size);
        DIAG_SET (SAC_HM_arenas[0][ARENA_OF_ARENAS].bins, 1);

        mem += initial_master_arena_of_arenas_size;
    } else {
        DIAG_SET (SAC_HM_arenas[0][ARENA_OF_ARENAS].size, 0);
        DIAG_SET (SAC_HM_arenas[0][ARENA_OF_ARENAS].bins, 0);
    }

    /*
     * Allocate worker arenas of arenas.
     */

    if (initial_worker_arena_of_arenas_size > 0) {
        for (t = 1; t < num_threads; t++) {
            freep = (SAC_HM_header_t *)mem;

            SAC_HM_SMALLCHUNK_SIZE (freep)
              = initial_worker_arena_of_arenas_size / UNIT_SIZE;
            SAC_HM_SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[t][ARENA_OF_ARENAS]);

            SAC_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
            SAC_HM_SMALLCHUNK_NEXTFREE (SAC_HM_arenas[t][ARENA_OF_ARENAS].freelist)
              = freep;

            DIAG_SET (SAC_HM_arenas[t][ARENA_OF_ARENAS].size,
                      initial_worker_arena_of_arenas_size);
            DIAG_SET (SAC_HM_arenas[t][ARENA_OF_ARENAS].bins, 1);

            mem += initial_worker_arena_of_arenas_size;
        }
    } else {
        for (t = 1; t < num_threads; t++) {
            DIAG_SET (SAC_HM_arenas[t][ARENA_OF_ARENAS].size, 0);
            DIAG_SET (SAC_HM_arenas[t][ARENA_OF_ARENAS].bins, 0);
        }
    }

    /*
     * Allocate top arena.
     */

    if (initial_top_arena_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        SAC_HM_LARGECHUNK_SIZE (freep) = initial_top_arena_size / UNIT_SIZE;
        SAC_HM_LARGECHUNK_ARENA (freep) = &(SAC_HM_arenas[0][TOP_ARENA]);
        SAC_HM_LARGECHUNK_PREVSIZE (freep) = -1;
        DIAG_SET_FREEPATTERN_LARGECHUNK (freep);

        SAC_HM_arenas[0][TOP_ARENA].wilderness = freep;

        DIAG_SET (SAC_HM_arenas[0][TOP_ARENA].size, initial_top_arena_size);
        DIAG_SET (SAC_HM_arenas[0][TOP_ARENA].bins, 1);
    } else {
        DIAG_SET (SAC_HM_arenas[0][TOP_ARENA].size, 0);
        DIAG_SET (SAC_HM_arenas[0][TOP_ARENA].bins, 0);
    }
}
