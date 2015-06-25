/*****************************************************************************
 *
 * file:   setup.c
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description:
 *
 *   This file contains the setup or initialization facilities for the
 *   SAC distributed memory private heap manager.
 *
 *****************************************************************************/

#include "config.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include "heapmgr.h"

/*
 * Heap management configuration data.
 */

static const SAC_DISTMEM_HM_size_unit_t min_chunk_size[]
  = {SAC_DISTMEM_HM_ARENA_0_MINCS, SAC_DISTMEM_HM_ARENA_1_MINCS,
     SAC_DISTMEM_HM_ARENA_2_MINCS, SAC_DISTMEM_HM_ARENA_3_MINCS,
     SAC_DISTMEM_HM_ARENA_4_MINCS, SAC_DISTMEM_HM_ARENA_5_MINCS,
     SAC_DISTMEM_HM_ARENA_6_MINCS, SAC_DISTMEM_HM_ARENA_7_MINCS,
     SAC_DISTMEM_HM_ARENA_8_MINCS};

static const SAC_DISTMEM_HM_size_unit_t binsize[]
  = {SAC_DISTMEM_HM_ARENA_0_BINSIZE, SAC_DISTMEM_HM_ARENA_1_BINSIZE,
     SAC_DISTMEM_HM_ARENA_2_BINSIZE, SAC_DISTMEM_HM_ARENA_3_BINSIZE,
     SAC_DISTMEM_HM_ARENA_4_BINSIZE, SAC_DISTMEM_HM_ARENA_5_BINSIZE,
     SAC_DISTMEM_HM_ARENA_6_BINSIZE, SAC_DISTMEM_HM_ARENA_7_BINSIZE,
     SAC_DISTMEM_HM_ARENA_8_BINSIZE};

SAC_DISTMEM_HM_arena_t SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_NUM_ARENAS + 2]
  = SAC_DISTMEM_HM_SETUP_ARENAS ();

/******************************************************************************
 *
 * function:
 *   void SAC_DISTMEM_HM_Setup( void)
 *
 * description:
 *
 *   This function pre-allocates heap memory for the
 *   arena of arenas as well as the top arena and initializes
 *   the corresponding data structures.
 *
 *   Particular care is taken of the correct alignment of these inital
 *   heap structures.
 *
 ******************************************************************************/

void
SAC_DISTMEM_HM_Setup (void)
{
    SAC_DISTMEM_HM_size_byte_t initial_arena_of_arenas_size = SAC_DISTMEM_pagesz;

    /* Compute initial top arena size upon setup time. */
    SAC_DISTMEM_HM_size_byte_t initial_top_arena_size
      = SAC_DISTMEM_segsz - initial_arena_of_arenas_size;

    /* SAC_DISTMEM_shared_seg_ptr points to the start of the shared segment of this node.
     * It is guaranteed to start at a page boundary. */
    char *mem = (char *)SAC_DISTMEM_shared_seg_ptr;

    /*
     * Allocate arena of arenas.
     */

    if (initial_arena_of_arenas_size > 0) {
        SAC_DISTMEM_HM_header_t *freep = (SAC_DISTMEM_HM_header_t *)mem;

        SAC_DISTMEM_HM_SMALLCHUNK_SIZE (freep)
          = initial_arena_of_arenas_size / SAC_DISTMEM_HM_UNIT_SIZE;

        SAC_DISTMEM_HM_SMALLCHUNK_ARENA (freep)
          = &(SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS]);

        SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (freep) = NULL;
        SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE (
          SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS].freelist)
          = freep;

        DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS].size,
                  initial_arena_of_arenas_size);
        DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS].cnt_bins, 1);

        mem += initial_arena_of_arenas_size;
    } else {
        DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS].size, 0);
        DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_ARENA_OF_ARENAS].cnt_bins, 0);
    }

    /*
     * Allocate top arena.
     */

    SAC_DISTMEM_HM_header_t *freep = (SAC_DISTMEM_HM_header_t *)mem;

#ifdef DIAG
    fprintf (stderr, "Allocating top area at: %p\n", mem);
#endif

    SAC_DISTMEM_HM_LARGECHUNK_SIZE (freep)
      = initial_top_arena_size / SAC_DISTMEM_HM_UNIT_SIZE;
    SAC_DISTMEM_HM_LARGECHUNK_ARENA (freep)
      = &(SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA]);
    SAC_DISTMEM_HM_LARGECHUNK_PREVSIZE (freep) = -1;
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);

    SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA].wilderness = freep;

    DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA].size,
              initial_top_arena_size);
    DIAG_SET (SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA].cnt_bins, 1);

    /*
     * Initialize diaganostic check patterns in initial dummy entries of
     * free lists.
     */

#ifdef DIAG
    {
        for (int i = 0; i < SAC_DISTMEM_HM_NUM_SMALLCHUNK_ARENAS; i++) {
            DIAG_SET_FREEPATTERN_SMALLCHUNK (SAC_DISTMEM_HM_arenas[i].freelist);
        }
        for (int i = SAC_DISTMEM_HM_NUM_SMALLCHUNK_ARENAS; i <= SAC_DISTMEM_HM_TOP_ARENA;
             i++) {
            DIAG_SET_FREEPATTERN_LARGECHUNK (SAC_DISTMEM_HM_arenas[i].freelist);
        }
    }
#endif /* defined(DIAG) */
}
