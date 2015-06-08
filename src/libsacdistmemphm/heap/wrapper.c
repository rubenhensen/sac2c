/*****************************************************************************
 *
 * file:  wrapper.c
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description:
 *
 *   This file contains wrapper functions for allocation and de-allocation
 *   routines. These are required where the core routines for large and
 *   small chunk arenas cannot be called directly from the compiled SAC code.
 *
 *
 *****************************************************************************/

#include <stdlib.h>

#include "heapmgr.h"

/******************************************************************************
 *
 * function:
 *   void *SAC_DISTMEM_HM_MallocAnyChunk(SAC_DISTMEM_HM_size_byte_t size)
 *
 * description:
 *
 *   Wrapper function for allocation of a statically unknown amount of memory.
 *
 *   SAC_DISTMEM_HM_MallocAnyChunk() is also used by SAC program if these are
 *   compiled for purely sequential execution. In these cases, we also do
 *   not use malloc(), since malloc() contains an additional check whether
 *   the heap manager data structures have already been initialized or not.
 *   However, attempts to allocate memory before initialization may only
 *   occur if other library functions internally use malloc().
 *
 ******************************************************************************/

void *
SAC_DISTMEM_HM_MallocAnyChunk (SAC_DISTMEM_HM_size_byte_t size)
{
    SAC_DISTMEM_HM_size_unit_t units;

    if (size <= SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= SAC_DISTMEM_HM_ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= SAC_DISTMEM_HM_ARENA_1_MAXCS_BYTES) {
                DIAG_INC (SAC_DISTMEM_HM_arenas[1].cnt_alloc_var_size);
                return (SAC_DISTMEM_HM_MallocSmallChunk (2, &(SAC_DISTMEM_HM_arenas[1])));
            } else {
                DIAG_INC (SAC_DISTMEM_HM_arenas[2].cnt_alloc_var_size);
                return (SAC_DISTMEM_HM_MallocSmallChunk (4, &(SAC_DISTMEM_HM_arenas[2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= SAC_DISTMEM_HM_ARENA_3_MAXCS_BYTES) {
                DIAG_INC (SAC_DISTMEM_HM_arenas[3].cnt_alloc_var_size);
                return (SAC_DISTMEM_HM_MallocSmallChunk (8, &(SAC_DISTMEM_HM_arenas[3])));
            } else {
                DIAG_INC (SAC_DISTMEM_HM_arenas[4].cnt_alloc_var_size);
                return (
                  SAC_DISTMEM_HM_MallocSmallChunk (16, &(SAC_DISTMEM_HM_arenas[4])));
            }
        }
    } else {
        units = ((size - 1) / SAC_DISTMEM_HM_UNIT_SIZE) + 3;

        if (units < SAC_DISTMEM_HM_ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < SAC_DISTMEM_HM_ARENA_6_MINCS) {
                DIAG_INC (SAC_DISTMEM_HM_arenas[5].cnt_alloc_var_size);
                return (
                  SAC_DISTMEM_HM_MallocLargeChunk (units, &(SAC_DISTMEM_HM_arenas[5])));
            } else {
                DIAG_INC (SAC_DISTMEM_HM_arenas[6].cnt_alloc_var_size);
                return (
                  SAC_DISTMEM_HM_MallocLargeChunk (units, &(SAC_DISTMEM_HM_arenas[6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < SAC_DISTMEM_HM_ARENA_8_MINCS) {
                DIAG_INC (SAC_DISTMEM_HM_arenas[7].cnt_alloc_var_size);
                return (
                  SAC_DISTMEM_HM_MallocLargeChunk (units, &(SAC_DISTMEM_HM_arenas[7])));
            } else {
                DIAG_INC (SAC_DISTMEM_HM_arenas[8].cnt_alloc_var_size);
                return (
                  SAC_DISTMEM_HM_MallocLargeChunk (units,
                                                   &(SAC_DISTMEM_HM_arenas
                                                       [SAC_DISTMEM_HM_TOP_ARENA])));
            }
        }
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_DISTMEM_HM_FreeAnyChunk(void *addr)
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
SAC_DISTMEM_HM_FreeAnyChunk (void *addr)
{
    SAC_DISTMEM_HM_arena_t *arena;

    if (addr != NULL) {

        arena = SAC_DISTMEM_HM_ADDR_ARENA (addr);

        if (arena != NULL) {
            if ((long int)arena & (long int)1) {
                addr = (SAC_DISTMEM_HM_header_t *)((long int)arena & (~(long int)1));
                arena = SAC_DISTMEM_HM_ADDR_ARENA (addr);
            }

            DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_DISTMEM_HM_header_t *)addr);
            DIAG_INC (arena->cnt_free_var_size);

            if (arena->num < SAC_DISTMEM_HM_NUM_SMALLCHUNK_ARENAS) {
                SAC_DISTMEM_HM_FreeSmallChunk ((SAC_DISTMEM_HM_header_t *)addr, arena);
                return;
            }

            SAC_DISTMEM_HM_FreeLargeChunk ((SAC_DISTMEM_HM_header_t *)addr, arena);
        }
    }
}
