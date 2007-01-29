/*
 *
 * $Id$
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

#define ARRAY_PLACEMENT

void
free (void *addr)
{
    SAC_HM_arena_t *arena;

    if (addr != NULL) {

        arena = SAC_HM_ADDR_ARENA (addr);

        if (arena != NULL) {
#ifdef ARRAY_PLACEMENT
            if ((long int)arena & (long int)1) {
                addr = (SAC_HM_header_t *)((long int)arena & (~(long int)1));
                arena = SAC_HM_ADDR_ARENA (addr);
            }
#endif
            DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_HM_header_t *)addr);
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
}
