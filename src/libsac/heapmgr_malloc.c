/*
 *
 * $Log$
 * Revision 1.1  1999/07/08 12:28:56  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:
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

#include "heapmgr.h"

void *
malloc (size_byte_t size)
{
    size_unit_t units;

    if (size <= ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= ARENA_1_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[1])));
            } else {
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= ARENA_3_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[3])));
            } else {
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[4])));
            }
        }
    } else {
        units = ((size - 1) / UNIT_SIZE) + 3;

        if (units < ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < ARENA_6_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[5])));
            } else {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < ARENA_8_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[7])));
            } else {
                return (SAC_HM_MallocTopArena (units, &(SAC_HM_arenas[8])));
            }
        }
    }
}

void
free (void *addr)
{
    SAC_HM_arena_t *arena;

    if (addr != NULL) {
        arena = SAC_HM_ADDR_2_ARENA (addr);
        arena->freefun ((SAC_HM_header_t *)addr, arena);
    }
}
