/*
 *
 * $Log$
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
 * file:   heapmgr_diag.c
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

#define DIAG

#include <stdio.h>

#include "heapmgr.h"
#include "sac_message.h"

/*
 * Global counters for heap management diagnostics.
 */

unsigned long int SAC_HM_call_sbrk = 0;
unsigned long int SAC_HM_call_malloc = 0;
unsigned long int SAC_HM_call_free = 0;
unsigned long int SAC_HM_call_realloc = 0;
unsigned long int SAC_HM_call_calloc = 0;
unsigned long int SAC_HM_call_valloc = 0;
unsigned long int SAC_HM_call_memalign = 0;
unsigned long int SAC_HM_heapsize = 0;

/******************************************************************************
 *
 * function:
 *   SAC_HM_CheckAllocDiagPattern(size_unit_t diag, int arena_num)
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
SAC_HM_CheckAllocDiagPattern (size_unit_t diag, int arena_num)
{
    if (diag == DIAG_FREEPATTERN) {
        SAC_RuntimeError ("Tried to subsequently de-allocate heap location in "
                          "arena %d",
                          arena_num);
    }

    if (diag != DIAG_ALLOCPATTERN) {
        SAC_RuntimeError ("Corrupted / missing heap administration data encountered "
                          "upon memory de-allocation in arena %d",
                          arena_num);
    }
}

/******************************************************************************
 *
 * function:
 *   SAC_HM_CheckFreeDiagPattern(size_unit_t diag, int arena_num)
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
SAC_HM_CheckFreeDiagPattern (size_unit_t diag, int arena_num)
{
    if (diag != DIAG_FREEPATTERN) {
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
    if ((SMALLCHUNK_DIAG (addr - 1) != DIAG_ALLOCPATTERN)
        && (SMALLCHUNK_DIAG (addr - 1) != DIAG_FREEPATTERN)
        && (LARGECHUNK_DIAG (addr - 2) != DIAG_ALLOCPATTERN)
        && (LARGECHUNK_DIAG (addr - 2) != DIAG_FREEPATTERN)) {
        SAC_RuntimeError ("Corrupted free list encountered upon memory allocation "
                          "in unspecified arena");
    }
}

/******************************************************************************
 *
 * function:
 *   void ShowDiagnosticsForArena(int num,
 *                                unsigned long int size,
 *                                unsigned long int bins,
 *                                unsigned long int alloc,
 *                                unsigned long int free,
 *                                unsigned long int split,
 *                                unsigned long int coalasce)
 *
 *
 * description:
 *
 *   This function pretty prints post-mortem diagnostic information concerning
 *   a given arena. These data include:
 *    - the total size of the arena,
 *    - the number of bins in this arena, i.e. the number of arena allocations,
 *    - the number of allocation and de-allocation operations,
 *    - the number of chunk splitting and coalascing operations.
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * function:
 *   void SAC_HM_ShowDiagnostics()
 *
 * description:
 *
 *   This function pretty prints post-mortem heap management diagnostics.
 *
 ******************************************************************************/

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

    /*
     *  Print diagnostics for arena of arenas.
     *  These figures shall not be summed up to form total diagnostics
     *  of all arenas because memory chunks in this arena will on purpose never
     *  be freed. So including the arena of arenas in the total figures seemingly
     *  would end up showing a space leak where actually there is none.
     */

    if (SAC_HM_arenas[0].bins > 0) {
        ShowDiagnosticsForArena (0, SAC_HM_arenas[0].size, SAC_HM_arenas[0].bins,
                                 SAC_HM_arenas[0].cnt_alloc, SAC_HM_arenas[0].cnt_free,
                                 SAC_HM_arenas[0].cnt_split,
                                 SAC_HM_arenas[0].cnt_coalasce);
    }

    /*
     *  Now, print diagnostics for all regular arenas.
     */

    for (i = 1; i < NUM_ARENAS; i++) {
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

    /*
     * Finally, print the total figures gathered in the above loop.
     */

    ShowDiagnosticsForArena (-1, SAC_HM_heapsize, bins_total, cnt_alloc_total,
                             cnt_free_total, cnt_split_total, cnt_coalasce_total);
}
