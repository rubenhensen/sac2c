/*
 *
 * $Log$
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

#include "heapmgr.h"

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
malloc (size_byte_t size)
{
    size_unit_t units;
    void *mem;
#ifdef MT
    unsigned int thread_id;
    const int multi_threaded = !SAC_MT_not_yet_parallel;
#else  /* MT */
    const unsigned int thread_id = 0;
#endif /* MT */

    DIAG_INC_LOCK (SAC_HM_call_malloc);

#ifdef MT
    if (multi_threaded && (size <= ARENA_7_MAXCS_BYTES)) {
        thread_id = *((unsigned int *)pthread_getspecific (SAC_MT_threadid_key));
    } else {
        thread_id = 0;
    }
#endif /* MT */

    if (size <= ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= ARENA_1_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[thread_id][1])));
            } else {
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[thread_id][2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= ARENA_3_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[thread_id][3])));
            } else {
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[thread_id][4])));
            }
        }
    } else {
        units = ((size - 1) / UNIT_SIZE) + 3;

        if (units < ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < ARENA_6_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][5])));
            } else {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < ARENA_8_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][7])));
            } else {
#ifdef MT
                if (multi_threaded) {
                    pthread_mutex_lock (&SAC_HM_top_arena_lock);
                    mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
                    pthread_mutex_unlock (&SAC_HM_top_arena_lock);
                } else {
                    mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
                }
#else  /* MT */
                mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
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

    DIAG_INC_LOCK (SAC_HM_call_free);

    if (addr != NULL) {
        DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_HM_header_t *)addr);
        arena = SAC_HM_ADDR_ARENA (addr);

        if (arena->num < NUM_SMALLCHUNK_ARENAS) {
            SAC_HM_FreeSmallChunk (addr, arena);
            return;
        }

#ifdef MT
        if (arena->num < TOP_ARENA) {
            SAC_HM_FreeLargeChunk (addr, arena);
        } else {
            SAC_HM_FreeTopArena_at (addr);
        }
#else  /* MT */
        SAC_HM_FreeLargeChunk (addr, arena);
#endif /* MT */
    }
}

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocCheck(size_byte_t size)
 *
 * description:
 *
 *   This function definition is needed to avoid problems at link time when
 *   calls to SAC_HM_MallocCheck() are compiled into the code due to selecting
 *   the -check m compiler flag although the private heap management is used
 *   which does these checks implicitly.
 *
 *
 ******************************************************************************/

void *
SAC_HM_MallocCheck (size_byte_t size)
{
    return (malloc (size));
}
