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
 * Revision 1.4  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
 * Revision 1.3  1999/07/29 07:35:41  cg
 * Two new performance related features added to SAC private heap
 * management:
 *   - pre-splitting for arenas with fixed size chunks.
 *   - deferred coalascing for arenas with variable chunk sizes.
 *
 * Revision 1.2  1999/07/16 09:41:16  cg
 * Added facilities for heap management diagnostics.
 *
 * Revision 1.1  1999/07/08 12:28:56  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   malloc.c
 *
 * prefix: none
 *
 * description:
 *
 *   This file provides general interface functions for the heap manager,
 *   i.e. basically specific implementations of malloc() and free() which
 *   themselves call the arena-specific functions of the heap manager API.
 *
 *
 *****************************************************************************/

#include <stdlib.h>

#include "heapmgr.h"

/******************************************************************************
 *
 * global variable:
 *   int SAC_HM_not_yet_initialized
 *
 * description:
 *
 *   This is a global flag that allows to keep track whether the
 *   internal data structures of the heap manager have already been
 *   initialized or not.
 *
 *
 ******************************************************************************/

int SAC_HM_not_yet_initialized = 1;

/******************************************************************************
 *
 * function:
 *   void *malloc(size_byte_t size)
 *
 * description:
 *
 *   Interface function for allocation of memory chunks.
 *
 *   Note that often the more specific heap manager API functions are called
 *   directly.
 *
 ******************************************************************************/

void *
malloc (size_t sz)
{
    const SAC_HM_size_byte_t size = (SAC_HM_size_byte_t)sz;
    SAC_HM_size_unit_t units;
    void *mem;
#ifdef MT
    unsigned int thread_id, *thread_id_ptr;
    const int multi_threaded = !SAC_MT_not_yet_parallel;
#else  /* MT */
    const unsigned int thread_id = 0;
#endif /* MT */

    DIAG_INC_LOCK (SAC_HM_call_malloc);

    if (SAC_HM_not_yet_initialized) {
        SAC_HM_SetupMaster ();
        SAC_HM_not_yet_initialized = 0;
    }

#ifdef MT
    if (multi_threaded && (size <= SAC_HM_ARENA_7_MAXCS_BYTES)) {
        thread_id_ptr = (unsigned int *)pthread_getspecific (SAC_MT_threadid_key);
        if (thread_id_ptr == NULL) {
            thread_id = 0;
        } else {
            thread_id = *thread_id_ptr;
        }
    } else {
        thread_id = 0;
    }
#endif /* MT */

    if (size <= SAC_HM_ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= SAC_HM_ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= SAC_HM_ARENA_1_MAXCS_BYTES) {
                DIAG_INC (SAC_HM_arenas[thread_id][1].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[thread_id][1])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][2].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[thread_id][2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= SAC_HM_ARENA_3_MAXCS_BYTES) {
                DIAG_INC (SAC_HM_arenas[thread_id][3].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[thread_id][3])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][4].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[thread_id][4])));
            }
        }
    } else {
        units = ((size - 1) / SAC_HM_UNIT_SIZE) + 3;

        if (units < SAC_HM_ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < SAC_HM_ARENA_6_MINCS) {
                DIAG_INC (SAC_HM_arenas[thread_id][5].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][5])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][6].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < SAC_HM_ARENA_8_MINCS) {
                DIAG_INC (SAC_HM_arenas[thread_id][7].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][7])));
            } else {
#ifdef MT
                if (multi_threaded) {
                    SAC_MT_ACQUIRE_LOCK (SAC_HM_top_arena_lock);
                    DIAG_INC (SAC_HM_acquire_top_arena_lock);
                    DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                    mem = SAC_HM_MallocLargeChunk (units,
                                                   &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
                    SAC_MT_RELEASE_LOCK (SAC_HM_top_arena_lock);
                } else {
                    DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                    mem = SAC_HM_MallocLargeChunk (units,
                                                   &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
                }
#else  /* MT */
                DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                mem = SAC_HM_MallocLargeChunk (units,
                                               &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
#endif /* MT */
                return (mem);
            }
        }
    }
}

/******************************************************************************
 *
 * function:
 *   void free(void *addr)
 *
 * description:
 *
 *   Interface function for de-allocation of memory chunks.
 *
 *   Note that often the more specific heap manager API functions are called
 *   directly.
 *
 ******************************************************************************/

void
free (void *addr)
{
    SAC_HM_arena_t *arena;

    if (addr != NULL) {
        DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_HM_header_t *)addr);
        arena = SAC_HM_ADDR_ARENA (addr);

        DIAG_INC (arena->cnt_free_var_size);

        if (arena->num < SAC_HM_NUM_SMALLCHUNK_ARENAS) {
            SAC_HM_FreeSmallChunk (addr, arena);
            return;
        }

#ifdef MT
        if (arena->num < SAC_HM_TOP_ARENA) {
            SAC_HM_FreeLargeChunk (addr, arena);
        } else {
            SAC_HM_FreeTopArena_at (addr);
        }
#else  /* MT */
        SAC_HM_FreeLargeChunk (addr, arena);
#endif /* MT */
    }
}
