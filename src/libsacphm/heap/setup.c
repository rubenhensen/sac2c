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

#include "config.h"
#include <unistd.h>
#include <stdint.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "heapmgr.h"

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

void SAC_HM_SetupWorkers (unsigned int num_threads);

/*
 * Configuration variables for heap setup.
 *
 *   These variables are defined and initialized in the compiled
 *   SAC code.
 */

extern const SAC_HM_size_byte_t SAC_HM_initial_master_arena_of_arenas_size;
extern const SAC_HM_size_byte_t SAC_HM_initial_worker_arena_of_arenas_size;
extern const SAC_HM_size_byte_t SAC_HM_initial_top_arena_size;
extern const unsigned int SAC_HM_max_worker_threads;

#if SAC_DO_HM_AUTONOMOUS
/* sac4c XT variant of the PHM library:
 * we're on our own, nobody will tell us the parameters nor initialize us.
 */

#define SAC_SET_THREADS_MAX SAC_HM_ASSUME_THREADS_MAX
#define SAC_SET_INITIAL_MASTER_HEAPSIZE (1024 * 1024)
#define SAC_SET_INITIAL_WORKER_HEAPSIZE (1024 * 1024)
#define SAC_SET_INITIAL_UNIFIED_HEAPSIZE (1024 * 1024)

/* instantiate the global variables right here and now */
SAC_HM_DEFINE__IMPL ()

#endif /* SAC_DO_HM_AUTONOMOUS */

/*
 * Locks for synchronisation of multi-threaded accesses to global
 * data structures of the heap manager.
 */

SAC_MT_DEFINE_LOCK (SAC_HM_top_arena_lock)
SAC_MT_DEFINE_LOCK (SAC_HM_diag_counter_lock)

/******************************************************************************
 *
 * function:
 *   void SAC_HM_SetupMaster()
 *
 * description:
 *
 *   This function pre-allocates heap memory for the master thread's
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

#ifdef DIAG
    /* Register the diagnostics function. */
    SAC_MessageExtensionCallback = &SAC_HM_ShowDiagnostics;
#endif

    /*
     * Prepare initial request for memory from operating system.
     */

    SAC_HM_size_byte_t pagesize = getpagesize ();
    char *mem = (char *)SBRK (0);
    /* Determine heap origin. */

    SAC_HM_size_byte_t offset = ((SAC_HM_size_byte_t)mem) % pagesize;
    /* Determine offset of heap origin from last memory page boundary. */

    if (offset != 0) {
        offset = pagesize - offset;
        /* If heap does not start on page boundary, adjust heap start. */
    }

    /* Compute initial top arena size upon setup time. */
    SAC_HM_size_byte_t initial_top_arena_size
      = SAC_HM_initial_worker_arena_of_arenas_size * SAC_HM_max_worker_threads
        + SAC_HM_initial_top_arena_size;

    /* Compute initial heap size to be requested from operating system. */
    SAC_HM_size_byte_t initial_heap_size
      = offset + SAC_HM_initial_master_arena_of_arenas_size + initial_top_arena_size;

    mem = (char *)SBRK (initial_heap_size);
    if (mem == (char *)-1) {
        SAC_HM_OutOfMemory (initial_heap_size);
    }

    mem += offset;

    DIAG_SET (SAC_HM_call_sbrk, 2);
    DIAG_SET (SAC_HM_heapsize, initial_heap_size);

    /*
     * Allocate master thread's arena of arenas.
     */

    if (SAC_HM_initial_master_arena_of_arenas_size > 0) {
        SAC_HM_header_t *freep = (SAC_HM_header_t *)mem;

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
        SAC_HM_header_t *freep = (SAC_HM_header_t *)mem;

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
        for (int i = 0; i < SAC_HM_NUM_SMALLCHUNK_ARENAS; i++) {
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[0][i].freelist);
        }
        for (int i = SAC_HM_NUM_SMALLCHUNK_ARENAS; i <= SAC_HM_TOP_ARENA; i++) {
            DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_HM_arenas[0][i].freelist);
        }
    }
#endif

    SAC_HM_SetInitialized ();
    /*
     * Mark heap manager as initialised.
     * This function call also enforces linking with the standard heap
     * mangement API compatibility layer.
     */

#if SAC_DO_HM_AUTONOMOUS
    /* sac4c XT variant of the PHM library;
     * This removes the need to call SAC_HM_Setup() in a library */

    /* initialize for the maximal number of threads in the process */
    SAC_HM_SetupWorkers (SAC_HM_ASSUME_THREADS_MAX);

#endif /* SAC_DO_HM_AUTONOMOUS */
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

void
SAC_HM_SetupWorkers (unsigned int num_threads)
{
    /*
     * Initialize worker thread entries in global array of arenas.
     */

    for (unsigned t = 1; t < num_threads; t++) {
        for (int i = 0; i < SAC_HM_NUM_SMALLCHUNK_ARENAS; i++) {
            SAC_HM_arenas[t][i].num = i;
            SAC_HM_arenas[t][i].freelist[0].data1.size = 0;
            SAC_HM_arenas[t][i].freelist[0].data1.arena = &(SAC_HM_arenas[t][i]);
            SAC_HM_arenas[t][i].freelist[1].data2.nextfree = NULL;

            SAC_HM_arenas[t][i].wilderness = SAC_HM_arenas[t][i].freelist;
            SAC_HM_arenas[t][i].binsize = binsize[i];
            SAC_HM_arenas[t][i].min_chunk_size = min_chunk_size[i];
            SAC_HM_arenas[t][i].unused_list = NULL;
#ifdef DIAG
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_HM_arenas[t][i].freelist);
            SAC_HM_ClearDiagCounters (&(SAC_HM_arenas[t][i]));
#endif
        }
        for (int i = SAC_HM_NUM_SMALLCHUNK_ARENAS; i < SAC_HM_NUM_ARENAS - 1; i++) {
            SAC_HM_arenas[t][i].num = i;
            SAC_HM_arenas[t][i].freelist[0].data3.prevsize = -1;
            SAC_HM_arenas[t][i].freelist[1].data1.arena = &(SAC_HM_arenas[t][i]);
            SAC_HM_arenas[t][i].freelist[1].data1.size = 0;
            SAC_HM_arenas[t][i].freelist[2].data2.nextfree = NULL;
            SAC_HM_arenas[t][i].wilderness = SAC_HM_arenas[t][i].freelist;
            SAC_HM_arenas[t][i].binsize = binsize[i];
            SAC_HM_arenas[t][i].min_chunk_size = min_chunk_size[i];
            SAC_HM_arenas[t][i].unused_list = NULL;
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
        SAC_HM_size_unit_t units_per_thread
          = SAC_HM_initial_worker_arena_of_arenas_size / SAC_HM_UNIT_SIZE;

        SAC_HM_size_unit_t units_total = units_per_thread * SAC_HM_max_worker_threads + 4;

        char *mem
          = (char *)SAC_HM_MallocLargeChunk (units_total,
                                             &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));

        for (unsigned t = 1; t < num_threads; t++) {
            SAC_HM_header_t *freep = (SAC_HM_header_t *)mem;

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
        for (unsigned t = 1; t < num_threads; t++) {
            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].size, 0);
            DIAG_SET (SAC_HM_arenas[t][SAC_HM_ARENA_OF_ARENAS].cnt_bins, 0);
        }
    }
}

#ifndef DIAG

/*
 * The purpose of the following 'empty' function definition is to allow heap manager
 * diagnostics to be printed when program execution is terminated by a runtime error.
 * With the following dummy function, the function SAC_RuntimeError() may always call
 * SAC_HM_ShowDiagnostics() regardless of whether the program is linked with the
 * diagnostic version of the heap manager or not.
 *
 * This function definition cannot be put into diagnostics.c because with diagnostics
 * disabled diagnostics.o will not be linked to the program, there's simply no
 * reference to SAC_HM_ShowDiagnostics(). Unfortunately, there is a reference in
 * SAC_RuntimeError() which will incurr linking with nophm.o. However, this results
 * in a linker error since with phm and nophm both linked to a program, there are
 * lots of multiply defined symbols.
 */

void
SAC_HM_ShowDiagnostics ()
{
}

#endif /* DIAG */

void
SAC_HM_Setup (unsigned int threads)
{
#if SAC_DO_HM_AUTONOMOUS
    /* sac4c xt variant */
    SAC_RuntimeWarning (
      "This PHM variant is autonomous, hence do not call SAC_HM_Setup().\n"
      "    Statically configured for %d threads.",
      SAC_HM_ASSUME_THREADS_MAX);

#else  /* SAC_DO_HM_AUTONOMOUS */
    /* normal code */
    if (SAC_HM_GetInitialized ()) {
        SAC_HM_SetupMaster ();
    }

    if (threads > 1) {
        SAC_HM_SetupWorkers (threads);
    }
#endif /* SAC_DO_HM_AUTONOMOUS */
}
