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

/*
 * Revision 1.3  1999/11/02 14:29:25  sbs
 * output changed so that it is more "obvious" that the mem-usage in the arena
 * actually is a multiple of the binsize!
 *
 * Revision 1.2  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
 * Revision 1.1  1999/09/16 09:22:25  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   diagnostics.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains global variables (mostly counters) and functions
 *   that implement heap diagnostics. There are basically two different
 *   features:
 *
 *   1. Statistical information is gathered during program runtime and
 *      presented upon program termination.
 *
 *   2. Integrity checks are performed during runtime. These are designed
 *      to detect corruptions of the internal heap manager data structures,
 *      particularly the free lists. Furthermore, subsequent de-allocation
 *      requests for the same memory location are reported.
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "heapmgr.h"
#include "sac_message.h"

#ifdef DIAG

/*
 * Global counters for heap management diagnostics.
 */

unsigned long int SAC_HM_call_sbrk = 0;
unsigned long int SAC_HM_heapsize = 0;
unsigned long int SAC_HM_call_malloc = 0;
unsigned long int SAC_HM_call_realloc = 0;
unsigned long int SAC_HM_call_calloc = 0;
unsigned long int SAC_HM_call_valloc = 0;
unsigned long int SAC_HM_call_memalign = 0;
unsigned long int SAC_HM_acquire_top_arena_lock = 0;

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

/******************************************************************************
 *
 * function:
 *   SAC_HM_CheckAllocDiagPattern(SAC_HM_size_unit_t diag, int arena_num)
 *
 * description:
 *
 *   This function checks whether a diag pattern is correct for a currently
 *   allocated chunk of memory. If not, program execution is aborted with
 *   an appropriate error message indicating that the internal data structures
 *   of the heap manager are corrupted.
 *
 *   The arena´s number is only provided for an improved error message.
 *
 ******************************************************************************/

void
SAC_HM_CheckAllocDiagPattern (SAC_HM_size_unit_t diag, int arena_num)
{
    if (diag == DIAG_FREEPATTERN) {
        atexit (SAC_HM_ShowDiagnostics);
        SAC_RuntimeError ("Tried to subsequently de-allocate heap location in "
                          "arena %d",
                          arena_num);
    }

    if (diag != DIAG_ALLOCPATTERN) {
        atexit (SAC_HM_ShowDiagnostics);
        SAC_RuntimeError ("Corrupted / missing heap administration data encountered "
                          "upon memory de-allocation in arena %d",
                          arena_num);
    }
}

/******************************************************************************
 *
 * function:
 *   SAC_HM_CheckFreeDiagPattern(SAC_HM_size_unit_t diag, int arena_num)
 *
 * description:
 *
 *   This function checks whether a diag pattern is correct for a currently
 *   un-allocated chunk of memory. If not, program execution is aborted with
 *   an appropriate error message indicating that the internal data structures
 *   of the heap manager are corrupted.
 *
 *   The arena´s number is only provided for an improved error message.
 *
 ******************************************************************************/

void
SAC_HM_CheckFreeDiagPattern (SAC_HM_size_unit_t diag, int arena_num)
{
    if (diag != DIAG_FREEPATTERN) {
        atexit (SAC_HM_ShowDiagnostics);
        SAC_RuntimeError ("Corrupted free list encountered upon memory allocation "
                          "in arena %d",
                          arena_num);
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_CheckDiagPatternAnyChunk(SAC_HM_header_t *addr)
 *
 * description:
 *
 *   This function checks whether a given chunk has a valid diag pattern,
 *   both alloc and free patterns are accepted as well large and small
 *   chunks, respectively.
 *
 ******************************************************************************/

void
SAC_HM_CheckDiagPatternAnyChunk (SAC_HM_header_t *addr)
{
    if ((SAC_HM_SMALLCHUNK_DIAG (addr - 1) != DIAG_ALLOCPATTERN)
        && (SAC_HM_SMALLCHUNK_DIAG (addr - 1) != DIAG_FREEPATTERN)
        && (SAC_HM_LARGECHUNK_DIAG (addr - 2) != DIAG_ALLOCPATTERN)
        && (SAC_HM_LARGECHUNK_DIAG (addr - 2) != DIAG_FREEPATTERN)) {
        atexit (SAC_HM_ShowDiagnostics);
        SAC_RuntimeError ("Corrupted free list encountered upon memory allocation "
                          "in unspecified arena");
    }
}

/******************************************************************************
 *
 * function:
 *   int percent(unsigned long int a, unsigned long int b)
 *
 * description:
 *
 *   This function computes the ratio of the two given numbers in percent.
 *   If the denominator is zero the result is also zero.
 *
 ******************************************************************************/

static int
percent (unsigned long int a, unsigned long int b)
{
    if (b == 0) {
        return (100);
    } else {
        return ((int)(((float)a / (float)b) * 100));
    }
}

/******************************************************************************
 *
 * function:
 *   void ShowDiagnosticsForArena(SAC_HM_arena_t *arena)
 *
 *
 * description:
 *
 *   This function pretty prints post-mortem diagnostic information concerning
 *   the given arena collected during runtime.
 *
 ******************************************************************************/

static void
ShowDiagnosticsForArena (SAC_HM_arena_t *arena)
{
    unsigned long int done_after_splitting;
    unsigned long int done_after_wilderness;
    unsigned long int done_after_coalascing;
    unsigned long int done_after_coalascing_wilderness;

    if (arena->num == -1) {
        fprintf (stderr, "Total  (without arena of arenas) :\n");
    } else {
        if (arena->num == SAC_HM_ARENA_OF_ARENAS) {
            fprintf (stderr, "Arena %d :  Arena of Arenas\n", arena->num);
        } else {
            if (arena->num < SAC_HM_NUM_SMALLCHUNK_ARENAS) {
                fprintf (stderr, "Arena %d :  memory chunk size:  %lu Bytes\n",
                         arena->num,
                         (unsigned long int)(arena->min_chunk_size) * SAC_HM_UNIT_SIZE);
            } else {
                if (arena->num < SAC_HM_TOP_ARENA) {
                    fprintf (stderr, "Arena %d :  memory chunk size:  %lu -> %lu Bytes\n",
                             arena->num,
                             (unsigned long int)(arena->min_chunk_size)
                               * SAC_HM_UNIT_SIZE,
                             (unsigned long int)(min_chunk_size[arena->num + 1])
                               * SAC_HM_UNIT_SIZE);
                } else {
                    fprintf (stderr, "Arena %d :  memory chunk size:  %lu -> ... Bytes\n",
                             arena->num,
                             (unsigned long int)(arena->min_chunk_size)
                               * SAC_HM_UNIT_SIZE);
                }
            }
        }
    }

    done_after_splitting = arena->cnt_after_freelist + arena->cnt_after_splitting;
    done_after_wilderness = done_after_splitting + arena->cnt_after_wilderness;
    done_after_coalascing = done_after_wilderness + arena->cnt_after_coalascing;
    done_after_coalascing_wilderness
      = done_after_coalascing + arena->cnt_after_coalascing_wilderness;

    fprintf (stderr,
             "  %lu bin(s) totalling %lu Bytes (%.1f MB)\n"
             "  %9lu allocations:     %9lu (%3d%%) fixed size allocations\n",

             arena->cnt_bins, arena->size, ((float)(arena->size)) / MB,

             arena->cnt_alloc, arena->cnt_alloc - arena->cnt_alloc_var_size,
             percent (arena->cnt_alloc - arena->cnt_alloc_var_size, arena->cnt_alloc));

    if (arena->cnt_after_freelist > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (%3d%%) from free list\n",
                 arena->cnt_after_freelist,
                 percent (arena->cnt_after_freelist, arena->cnt_alloc),
                 percent (arena->cnt_after_freelist, arena->cnt_alloc));
    }

    if (arena->cnt_after_splitting > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (%3d%%) after splitting\n",
                 arena->cnt_after_splitting,
                 percent (arena->cnt_after_splitting, arena->cnt_alloc),
                 percent (done_after_splitting, arena->cnt_alloc));
    }

    if (arena->cnt_after_wilderness > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (%3d%%) from wilderness\n",
                 arena->cnt_after_wilderness,
                 percent (arena->cnt_after_wilderness, arena->cnt_alloc),
                 percent (done_after_wilderness, arena->cnt_alloc));
    }

    if (arena->cnt_coalascing > 0) {
        fprintf (stderr, "            %9lu               coalascings done\n",
                 arena->cnt_coalascing);
    }

    if (arena->cnt_after_coalascing > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (%3d%%) after coalascing\n",
                 arena->cnt_after_coalascing,
                 percent (arena->cnt_after_coalascing, arena->cnt_alloc),
                 percent (done_after_coalascing, arena->cnt_alloc));
    }

    if (arena->cnt_coalascing_wilderness > 0) {
        fprintf (stderr, "            %9lu               wilderness coalascings done\n",
                 arena->cnt_coalascing_wilderness);
    }

    if (arena->cnt_after_coalascing_wilderness > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (%3d%%) after coalascing wilderness\n",
                 arena->cnt_after_coalascing_wilderness,
                 percent (arena->cnt_after_coalascing_wilderness, arena->cnt_alloc),
                 percent (done_after_coalascing_wilderness, arena->cnt_alloc));
    }

    if (arena->cnt_after_extension > 0) {
        fprintf (stderr, "            %9lu (%3d%%) (100%%) after extending arena\n",
                 arena->cnt_after_extension,
                 percent (arena->cnt_after_extension, arena->cnt_alloc));
    }

    fprintf (stderr, "  %9lu de-allocations:  %9lu (%3d%%) fixed size de-allocations\n",
             arena->cnt_free, arena->cnt_free - arena->cnt_free_var_size,
             percent (arena->cnt_free - arena->cnt_free_var_size, arena->cnt_free));

    fprintf (stderr, "==================================================================="
                     "========\n");
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_ClearDiagCounters( SAC_HM_arena_t *arena)
 *
 * description:
 *
 *   This function resets all diagnostic counters of the given arena.
 *
 ******************************************************************************/

void
SAC_HM_ClearDiagCounters (SAC_HM_arena_t *arena)
{
    arena->size = 0;
    arena->cnt_bins = 0;
    arena->cnt_alloc = 0;
    arena->cnt_alloc_var_size = 0;
    arena->cnt_after_freelist = 0;
    arena->cnt_after_splitting = 0;
    arena->cnt_after_wilderness = 0;
    arena->cnt_after_coalascing = 0;
    arena->cnt_after_coalascing_wilderness = 0;
    arena->cnt_after_extension = 0;
    arena->cnt_free = 0;
    arena->cnt_free_var_size = 0;
    arena->cnt_coalascing = 0;
    arena->cnt_coalascing_wilderness = 0;
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_AddDiagCounters( SAC_HM_arena_t *arena, SAC_HM_arena_t *add_arena)
 *
 * description:
 *
 *   This function adds the diagnostic counters of the second arena to the
 *   equivalent counters of the first arena.
 *
 ******************************************************************************/

void
SAC_HM_AddDiagCounters (SAC_HM_arena_t *arena, SAC_HM_arena_t *add_arena)
{
    arena->size += add_arena->size;
    arena->cnt_bins += add_arena->cnt_bins;
    arena->cnt_alloc += add_arena->cnt_alloc;
    arena->cnt_alloc_var_size += add_arena->cnt_alloc_var_size;
    arena->cnt_after_freelist += add_arena->cnt_after_freelist;
    arena->cnt_after_splitting += add_arena->cnt_after_splitting;
    arena->cnt_after_wilderness += add_arena->cnt_after_wilderness;
    arena->cnt_after_coalascing += add_arena->cnt_after_coalascing;
    arena->cnt_after_coalascing_wilderness += add_arena->cnt_after_coalascing_wilderness;
    arena->cnt_after_extension += add_arena->cnt_after_extension;
    arena->cnt_free += add_arena->cnt_free;
    arena->cnt_free_var_size += add_arena->cnt_free_var_size;
    arena->cnt_coalascing += add_arena->cnt_coalascing;
    arena->cnt_coalascing_wilderness += add_arena->cnt_coalascing_wilderness;
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_ShowDiagnostics(unsigned int num_threads)
 *
 * description:
 *
 *   This function pretty prints post-mortem heap management diagnostics.
 *
 ******************************************************************************/

void
SAC_HM_ShowDiagnostics ()
{
    int i, t;
    unsigned int num_threads;

#ifdef MT
    num_threads = SAC_MT_threads;
#else
    num_threads = 1;
#endif

    fprintf (stderr, "==================================================================="
                     "========\n"
                     "\n"
                     "Heap Management diagnostics:\n"
                     "\n"
                     "==================================================================="
                     "========\n");

    fprintf (stderr,
             "calls to sbrk()  :  %lu\n"
             "total heap size  :  %lu Bytes (%.1f MB)\n"
             "==========================================================================="
             "\n",
             SAC_HM_call_sbrk, SAC_HM_heapsize, ((float)SAC_HM_heapsize) / MB);

    fprintf (stderr,
             "calls to malloc()    :  %lu\n"
             "calls to calloc()    :  %lu\n"
             "calls to realloc()   :  %lu\n"
             "calls to valloc()    :  %lu\n"
             "calls to memalign()  :  %lu\n"
             "locks acquired       :  %lu\n"
             "==========================================================================="
             "\n",
             SAC_HM_call_malloc, SAC_HM_call_calloc, SAC_HM_call_realloc,
             SAC_HM_call_valloc, SAC_HM_call_memalign, SAC_HM_acquire_top_arena_lock);

    if (num_threads > 1) {
        fprintf (stderr, "\n"
                         "Master thread:\n"
                         "\n"
                         "==============================================================="
                         "============\n");
    }

    /*
     *  Print diagnostics for arena of arenas.
     *  These figures shall not be summed up to form total diagnostics
     *  of all arenas because memory chunks in this arena will on purpose never
     *  be freed. So including the arena of arenas in the total figures seemingly
     *  would end up showing a space leak where actually there is none.
     */

    if (SAC_HM_arenas[0][0].cnt_bins > 0) {
        /* Print arena info only if arena is non-empty. */
        ShowDiagnosticsForArena (&(SAC_HM_arenas[0][0]));
        SAC_HM_ClearDiagCounters (&(SAC_HM_arenas[0][0]));
    }

    /*
     *  Now, print diagnostics for all regular arenas.
     */

    for (i = 1; i < SAC_HM_NUM_ARENAS; i++) {
        if (SAC_HM_arenas[0][i].cnt_bins > 0) {
            ShowDiagnosticsForArena (&(SAC_HM_arenas[0][i]));
            SAC_HM_AddDiagCounters (&(SAC_HM_arenas[0][0]), &(SAC_HM_arenas[0][i]));
        }
    }

    /*
     * Finally, print the total figures gathered in the above loop.
     */
    SAC_HM_arenas[0][0].num = -1; /* signal total figures */
    ShowDiagnosticsForArena (&(SAC_HM_arenas[0][0]));

    if (num_threads > 1) {
        fprintf (stderr,
                 "\n"
                 "All worker threads combined (%u):\n"
                 "\n"
                 "======================================================================="
                 "====\n",
                 num_threads - 1);

        for (i = 0; i < SAC_HM_TOP_ARENA; i++) {
            for (t = 2; t < num_threads; t++) {
                SAC_HM_AddDiagCounters (&(SAC_HM_arenas[1][i]), &(SAC_HM_arenas[t][i]));
            }

            if (SAC_HM_arenas[1][i].cnt_bins > 0) {
                ShowDiagnosticsForArena (&(SAC_HM_arenas[1][i]));
            }
        }
    }
}

#endif /* DIAG */
