/*
 *
 * $Log$
 * Revision 1.3  2000/01/17 19:48:01  cg
 * Removed debug code.
 *
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

#include <unistd.h>

#include "heapmgr.h"
#include "sac_message.h"

/*
 * Heap management configuration data.
 */

static const SAC_HM_size_unit_t min_chunk_size[]
  = {SAC_HM_ARENA_0_MINCS, SAC_HM_ARENA_1_MINCS, SAC_HM_ARENA_2_MINCS,
     SAC_HM_ARENA_3_MINCS, SAC_HM_ARENA_4_MINCS, SAC_HM_ARENA_5_MINCS,
     SAC_HM_ARENA_6_MINCS, SAC_HM_ARENA_7_MINCS, SAC_HM_ARENA_8_MINCS};

static const SAC_HM_size_unit_t binsize[]
  = {SAC_HM_ARENA_0_BINSIZE, SAC_HM_ARENA_1_BINSIZE, SAC_HM_ARENA_2_BINSIZE,
     SAC_HM_ARENA_3_BINSIZE, SAC_HM_ARENA_4_BINSIZE, SAC_HM_ARENA_5_BINSIZE,
     SAC_HM_ARENA_6_BINSIZE, SAC_HM_ARENA_7_BINSIZE, SAC_HM_ARENA_8_BINSIZE};

/*
 * Configuration variables for heap setup.
 *
 *   These variables are defined and initialized in the compiled
 *   SAC code.
 */

extern SAC_HM_size_byte_t SAC_HM_initial_master_arena_of_arenas_size;
extern SAC_HM_size_byte_t SAC_HM_initial_worker_arena_of_arenas_size;
extern SAC_HM_size_byte_t SAC_HM_initial_top_arena_size;
extern unsigned int SAC_HM_max_worker_threads;

/*
 * Locks for synchronisation of multi-threaded accesses to global
 * data structures of the heap manager.
 */

SAC_MT_DEFINE_LOCK (SAC_HM_top_arena_lock);
SAC_MT_DEFINE_LOCK (SAC_HM_diag_counter_lock);

/******************************************************************************
 *
 * function:
 *   void SAC_HM_SetupMaster()
 *
 * description:
 *
 *   This function pre-allocates heap memory for the master thread´s
 *   arena of arenas as well as the top arena and initializes
 *   the corresponding data structures.
 *
 *   Particular care is taken of the correct alignment of these inital
 *   heap structures.
 *
 ******************************************************************************/

void
SAC_HM_SetupMaster ()
{
    SAC_HM_size_byte_t offset, initial_heap_size;
    SAC_HM_size_byte_t initial_top_arena_size, pagesize;
    SAC_HM_header_t *freep;
    char *mem;

    /*
     * Prepare initial request for memory from operating system.
     */

    pagesize = getpagesize ();
    mem = (char *)SBRK (0);
    /* Determine heap origin. */

    offset = ((SAC_HM_size_byte_t)mem) % pagesize;
    /* Determine offset of heap origin from last memory page boundary. */

    if (offset != 0) {
        offset = pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start. */
    }

    /* Compute initial top arena size upon setup time. */
    initial_top_arena_size
      = SAC_HM_initial_worker_arena_of_arenas_size * SAC_HM_max_worker_threads
        + SAC_HM_initial_top_arena_size;

    /* Compute initial heap size to be requested from operating system. */
    initial_heap_size
      = offset + SAC_HM_initial_master_arena_of_arenas_size + initial_top_arena_size;

    mem = (char *)SBRK (initial_heap_size);
    if (mem == (char *)-1) {
        SAC_HM_OutOfMemory (initial_heap_size);
    }

    mem += offset;

    DIAG_SET (SAC_HM_call_sbrk, 2);
    DIAG_SET (SAC_HM_heapsize, initial_heap_size);

    /*
     * Allocate master thread´s arena of arenas.
     */

    if (SAC_HM_initial_master_arena_of_arenas_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        SAC_HM_SMALLCHUNK_SIZE (freep)
          = SAC_HM_initial_master_arena_of_arenas_size / SAC_HM_UNIT_SIZE;

        SAC_HM_SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS]);

        SAC_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
        SAC_HM_SMALLCHUNK_NEXTFREE (SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS].freelist)
          = freep;

        DIAG_SET (SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS].size,
                  SAC_HM_initial_master_arena_of_arenas_size);
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS].cnt_bins, 1);

        mem += SAC_HM_initial_master_arena_of_arenas_size;
    } else {
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS].size, 0);
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_ARENA_OF_ARENAS].cnt_bins, 0);
    }

    /*
     * Allocate top arena.
     */

    if (initial_top_arena_size > 0) {
        freep = (SAC_HM_header_t *)mem;

        SAC_HM_LARGECHUNK_SIZE (freep) = initial_top_arena_size / SAC_HM_UNIT_SIZE;
        SAC_HM_LARGECHUNK_ARENA (freep) = &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]);
        SAC_HM_LARGECHUNK_PREVSIZE (freep) = -1;
        DIAG_SET_FREEPATTERN_LARGECHUNK (freep);

        SAC_HM_arenas[0][SAC_HM_TOP_ARENA].wilderness = freep;

        DIAG_SET (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].size, initial_top_arena_size);
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_bins, 1);
    } else {
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].size, 0);
        DIAG_SET (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_bins, 0);
    }

    /*
     * Initialize diaganostic check patterns in initial dummy entries of
     * free lists.
     */

#ifdef DIAG
    {
        int i;

        for (i = 0; i < SAC_HM_NUM_SMALLCHUNK_ARENAS; i++) {
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[0][i].freelist);
        }
        for (i = SAC_HM_NUM_SMALLCHUNK_ARENAS; i <= SAC_HM_TOP_ARENA; i++) {
            DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_HM_arenas[0][i].freelist);
        }
    }
#endif
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_SetupWorkers( unsigned int num_threads)
 *
 * description:
 *
 *   This function pre-allocates heap memory for each worker thread
 *   and initializes the corresponding arena data structures.
 *
 ******************************************************************************/

#ifdef MT

void
SAC_HM_SetupWorkers (unsigned int num_threads)
{
    SAC_HM_size_unit_t units_total, units_per_thread;
    SAC_HM_header_t *freep;
    char *mem;
    int i, t;

    /*
     * Initialize worker thread entries in global array of arenas.
     */

    for (t = 1; t < num_threads; t++) {
        for (i = 0; i < SAC_HM_NUM_SMALLCHUNK_ARENAS; i++) {
            SAC_HM_arenas[t][i].num = i;
            SAC_HM_arenas[t][i].freelist[0].data1.size = 0;
            SAC_HM_arenas[t][i].freelist[0].data1.arena = &(SAC_HM_arenas[t][i]);
            SAC_HM_arenas[t][i].freelist[1].data2.nextfree = NULL;

            SAC_HM_arenas[t][i].wilderness = SAC_HM_arenas[t][i].freelist;
            SAC_HM_arenas[t][i].binsize = binsize[i];
            SAC_HM_arenas[t][i].min_chunk_size = min_chunk_size[i];
#ifdef DIAG
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[t][i].freelist);
            SAC_HM_ClearDiagCounters (&(SAC_HM_arenas[t][i]));
#endif
        }
        for (i = SAC_HM_NUM_SMALLCHUNK_ARENAS; i < SAC_HM_NUM_ARENAS - 1; i++) {
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
            SAC_HM_ClearDiagCounters (&(SAC_HM_arenas[t][i]));
#endif
        }
    }

    /*
     * Allocate worker arenas of arenas.
     */

    if (SAC_HM_initial_worker_arena_of_arenas_size > 0) {
        units_per_thread = SAC_HM_initial_worker_arena_of_arenas_size / SAC_HM_UNIT_SIZE;

        units_total = units_per_thread * SAC_HM_max_worker_threads + 4;

        mem = (char *)SAC_HM_MallocLargeChunk (units_total,
                                               &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));

        for (t = 1; t < num_threads; t++) {
            freep = (SAC_HM_header_t *)mem;

            SAC_HM_SMALLCHUNK_SIZE (freep) = units_per_thread;

            SAC_HM_SMALLCHUNK_ARENA (freep) = &(SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS]);

            SAC_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
            SAC_HM_SMALLCHUNK_NEXTFREE (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].freelist)
              = freep;

            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].size,
                      SAC_HM_initial_worker_arena_of_arenas_size);
            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].cnt_bins, 1);

            mem += SAC_HM_initial_worker_arena_of_arenas_size;
        }
    } else {
        for (t = 1; t < num_threads; t++) {
            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].size, 0);
            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].cnt_bins, 0);
        }
    }
}

#endif /* MT */
